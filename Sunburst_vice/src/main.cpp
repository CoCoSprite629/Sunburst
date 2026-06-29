#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <FastLED.h>
#define LED_TYPE WS2812
#define LIGHT_PIN 18  // DI
#define BRIGHT 5
#define NUM_LEDS 1 // LED数量
#define DATA_PIN 6 // 数据线连接的Arduino引脚

CRGB led[1]; // 定义一个数组存储led颜色
String recvData;
String dialog = "";
String emoji = "";
String LED_Status;

int ESP_NOW_STATUS = 1;
int ESP_NOW_SENT = 0;
int LED_SWITCH = 0;
int EMOJI_CONTROLLER = 0;
int message_Order = 1;

unsigned char send_MAC[] = {0x94, 0x54, 0xC5, 0xB1, 0x9F, 0x74}; // 94:54:C5:B1:9F:74  十六进制 ： 123456789abcbdef

typedef struct send_Message
{
  String sent_Messsage = "";
  String sent_emoji = "";
} send_Message;
// 定义数据发送结构体

typedef struct recv_Message
{
  String recv_Messsage = "";
  String recv_emoji = "";
} recv_Message;
// 定义数据发送结构体

send_Message myDataSent;
// 定义结构体变量
recv_Message myDataRecv;
// 定义结构体变量

TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t Task3;
// 配置多进程

void onDataRecv() // 串口中断回调函数
{
  if (Serial2.available() > 0)
  {
    recvData = Serial2.readStringUntil('%');
    Serial.printf("%s", recvData);
    if (recvData == "START_ESP_NOW")
    {
      ESP_NOW_STATUS = 1;
    }

    else if (recvData == "SEND_MESSAGE_COMING")
    {
      Serial.printf("\nReceived Messages Data: ");
      dialog = Serial2.readStringUntil('%');
      // String temp2 = temp.substring(1); // 去除所得数据前面的\r字符
      // dialog = temp.c_str();           // 传中文用write，不用printf
      // Serial.write(dialog);
      emoji = "";
      ESP_NOW_SENT = 1;
    }

    else if (recvData == "SEND_EMOJI_COMING")
    {
      Serial.printf("\nReceived Emoji Data: ");
      emoji = Serial2.readStringUntil('%');
      // String temp2 = temp.substring(1); // 去除所得数据前面的\r字符
      // emoji = temp.c_str();           // 传中文用write，不用printf  //e.g. Serial2.write(emoji);  "chat."
      // Serial.write(emoji);
      dialog = "";
      ESP_NOW_SENT = 1;
    }

    else if (recvData == "POP_CLICKED")
    {
      LED_Status = "POP_CLICKED";
      // Serial2.flush();
      LED_SWITCH = 1;
    }

    else if (recvData == "MATCHING")
    {
      LED_Status = "MATCHING";
      LED_SWITCH = 1;
    }
  }
}

void OnDataSent(const unsigned char *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast packet sent status:\t");
  Serial.print(status == ESP_NOW_SEND_SUCCESS ? "Sent success" : "sent fail");
  LED_SWITCH = 1;
  LED_Status = "Sending";
  // 设置回调函数
}

void OnDataRecv(const unsigned char *mac_addr_sent,
                const unsigned char *inComingData, int len)
{
  memcpy(&myDataRecv, inComingData, sizeof(recv_Message));
  // Serial.println("Bytes received: ");
  // Serial.println(len);
  LED_SWITCH = 1;
  LED_Status = "Receiving";
  Serial.printf("\nReceived Message: %s", myDataRecv.recv_Messsage);
  Serial.printf("\n\rReceived emoji: %s", myDataRecv.recv_emoji);
  String message1 = "chat.t1.txt=\"" + myDataRecv.recv_Messsage + "\"\xff\xff\xff"; // 第一栏
  String message2 = "chat.t2.txt=\"" + myDataRecv.recv_Messsage + "\"\xff\xff\xff"; // 第二栏
  String message3 = "chat.t3.txt=\"" + myDataRecv.recv_Messsage + "\"\xff\xff\xff"; // 第三栏
  if (myDataRecv.recv_Messsage != "")
  {
    if (message_Order == 1)
    {
      Serial2.write(message1.c_str());
      message_Order = 2;
    }

    else if (message_Order == 2)
    {
      Serial2.write(message2.c_str());
      message_Order = 3;
    }

    else if (message_Order == 3)
    {
      Serial2.write(message3.c_str());
      message_Order = 1;
    }
  }
  EMOJI_CONTROLLER = 1;
  // messageSender();
}

void esp_now_INIT() // A 是主机端，提出通信并握手
{
  esp_now_register_send_cb(OnDataSent);
  // 注册发送回调函数
  esp_now_peer_info_t peerInfo = {};
  // 注册通信频道
  memcpy(peerInfo.peer_addr, send_MAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  while (1)
  {
    if (esp_now_add_peer(&peerInfo) == ESP_OK)
    {
      Serial.println("succeed to add peer");
      ESP_NOW_STATUS = 0;
      break;
    }
    else
    {
      Serial.println("fail to add peer");
      delay(100);
    }
  }
  esp_now_register_recv_cb(OnDataRecv);
}

void esp_now_SEND()
{
  myDataSent.sent_Messsage = dialog;
  myDataSent.sent_emoji = emoji;
  esp_err_t result = esp_now_send(send_MAC, (uint8_t *)&myDataSent, sizeof(send_Message));
  if (result == ESP_OK)
  {
    Serial.print("\nsent success\n");
    Serial.printf("Sent Message: %s", myDataSent.sent_Messsage);
  }
  else
  {
    Serial.print("\nsent failure");
  }
  ESP_NOW_SENT = 0;
}

void LED_Setup()
{
  LEDS.addLeds<LED_TYPE, LIGHT_PIN, GRB>(led, 1); // WS2812的色彩排列为GRB
  LEDS.setBrightness(BRIGHT);
}

void LED_Controller()
{
  if (LED_Status == "POP_CLICKED")
  {
    int randomNumber = random();
    fill_solid(led, 1, CHSV(randomNumber, 144, 255));
    FastLED.show();
    delay(400);
    FastLED.clear();
    FastLED.show();
    LED_SWITCH = 0;
  }

  else if (LED_Status == "MATCHING")
  {
    Serial.println("LED ON");
    // Serial.printf("\nI am Running");
    for (int i = 0; i <= 1; i++)
    {
      fill_solid(led, 1, CRGB(125, 255, 122));
      for (int i = 128; i >= 0; i--)
      {
        LEDS.setBrightness(i);
        FastLED.show();
        delay(5);
      }
      delay(200);
      for (int i = 0; i <= 128; i++)
      {
        LEDS.setBrightness(i);
        FastLED.show();
        delay(5);
      }
      FastLED.clear();
      FastLED.show();
      // Serial.println(i);
    }
    LED_SWITCH = 0;
  }

  else if (LED_Status == "Sending")
  {
    // Serial.printf("\nI am Running");
    fill_solid(led, 1, CRGB(255, 210, 148));
    for (int i = 128; i >= 0; i--)
    {
      LEDS.setBrightness(i);
      FastLED.show();
      delay(10);
    }
    delay(200);
    for (int i = 0; i <= 128; i++)
    {
      LEDS.setBrightness(i);
      FastLED.show();
      delay(10);
    }
    FastLED.clear();
    FastLED.show();
    LED_SWITCH = 0;
  }

  else if (LED_Status == "Receiving")
  {
    // Serial.printf("\nI am Running");
    fill_solid(led, 1, CRGB(255, 114, 107));
    for (int i = 128; i >= 0; i--)
    {
      LEDS.setBrightness(i);
      FastLED.show();
      delay(10);
    }
    delay(200);
    for (int i = 0; i <= 128; i++)
    {
      LEDS.setBrightness(i);
      FastLED.show();
      delay(10);
    }
    FastLED.clear();
    FastLED.show();
    LED_SWITCH = 0;
  }
}

void emojiSwitcher()
{
  // Serial.println("Im in");
  String emoji_Status = myDataRecv.recv_emoji.c_str();
  Serial.printf("\nemoji code: %s", emoji_Status);
  if (emoji_Status == "a")
  {
    // Serial.printf("Im in");
    Serial2.printf("chat.detect.val=1\xff\xff\xff");
  }
  else if (emoji_Status == "b")
  {
    // Serial.printf("Im in");
    Serial2.printf("chat.detect.val=2\xff\xff\xff");
  }
  else if (emoji_Status == "c")
  {
    // Serial.printf("Im in");
    Serial2.printf("chat.detect.val=3\xff\xff\xff");
  }
  else if (emoji_Status == "d")
  {
    // Serial.printf("Im in");
    Serial2.printf("chat.detect.val=4\xff\xff\xff");
  }
  else if (emoji_Status == "e")
  {
    // Serial.printf("Im in");
    Serial2.printf("chat.detect.val=5\xff\xff\xff");
  }
  else if (emoji_Status == "f")
  {
    // Serial.printf("Im in");
    Serial2.printf("chat.detect.val=6\xff\xff\xff");
  }
  EMOJI_CONTROLLER = 0;
}

void messageSender()
{
  Serial.printf("main.t1.txt\"%s\"\xff\xff\xff", myDataRecv.recv_Messsage);
}

void getInfo(void *pvParameters)
{
  while (1)
  {
    if (ESP_NOW_STATUS == 1)
    {
      esp_now_INIT();
    }

    else if (ESP_NOW_SENT == 1)
    {
      esp_now_SEND();
    }

    else if (LED_SWITCH == 1)
    {
      LED_Controller();
    }

    else if (EMOJI_CONTROLLER == 1)
    {
      emojiSwitcher();
    }

    delay(50);
  }
}

void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200);
  Serial2.onReceive(onDataRecv);

  LED_Setup();

  WiFi.mode(WIFI_STA);
  while (1)
  {
    if (esp_now_init() == ESP_OK)
    {
      Serial.println("ESP_NOW_is_OK");
      break;
    }
    else
    {
      Serial.println("ESP_NOW_is_FAILED");
      delay(30);
    }
  }

  // 多任务调度
  xTaskCreatePinnedToCore(
      getInfo, // 配置进程运行主函数
      "Task1", // 进程名
      10000,   // 设置堆栈大小
      NULL,
      1,      // 设置优先级
      &Task1, // 任务句柄
      0);     // 在core 0运行
  delay(500);

  /*
  xTaskCreatePinnedToCore(
      setNTPServer,
      "Task2",
      10000,
      NULL,
      1,
      &Task2,
      1);      //在core 1运行
  delay(500);
  */

  /*
    xTaskCreatePinnedToCore(
        LEDControl,
        "Task3",
        10000,
        NULL,
        2,
        &Task3,
        1);      //在core 1运行
    delay(500);
    */

  delay(1000);
}

void loop()
{
  // 使用FreeRTOSteam任务调度
}