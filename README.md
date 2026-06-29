# ☀️ Sunburst

> 基于 ESP-NOW 的双向交互通信装置 ｜ P2P Interactive Communication Device

**Sunburst** 是一对基于 **ESP32** 的点对点通信设备，通过 **ESP-NOW** 协议实现文字消息与表情的无线双向传输。每台设备配备 **WS2812 LED** 作为状态指示灯，并通过串口与外接 **HMI 触摸屏** 交互。适合作为情侣/朋友间的互动小礼物、创意电子手工作品或物联网通信学习项目。

---

## 功能特性

- ⚡ **ESP-NOW 点对点通信** — 低延迟、无需 WiFi 网络，两台设备直接通信
- 💬 **文字消息传输** — 通过触摸屏输入文字，实时发送到对方设备
- 😊 **表情互传** — 支持 6 种表情代码（`a` ~ `f`），在屏幕上动态显示
- 💡 **多彩 LED 状态反馈** — 4 种炫彩动画对应不同通信状态
- 📟 **HMI 触摸屏交互** — 通过自定义串口协议驱动外接屏幕
- 🧵 **FreeRTOS 多任务调度** — 双核任务管理，高效稳定

---

## 硬件清单

| 组件 | 型号/规格 | 数量 | 说明 |
|------|----------|------|------|
| 主控 | ESP32 DOIT DevKit V1 | 2 | 主机 + 副机 |
| LED | WS2812 可编程灯珠 | 2 | 状态指示灯 |
| 触摸屏 | HMI 串口屏 | 2 | 用户交互界面 |
| 电源 | 5V / Micro-USB | 2 | 供电 |

### 引脚连接

| 功能 | ESP32 引脚 |
|------|-----------|
| WS2812 数据线 (DI) | GPIO 18 |
| 串口屏 RX (Serial2) | GPIO 16 |
| 串口屏 TX (Serial2) | GPIO 17 |

---

## 项目结构

```
Sunburst/
├── README.md
├── .gitignore
├── Sunburst_main/                # 主机端固件
│   ├── platformio.ini            # PlatformIO 项目配置
│   ├── src/
│   │   └── main.cpp              # 主程序源码
│   ├── include/
│   ├── lib/
│   └── test/
├── Sunburst_vice/                # 副机端固件
│   ├── platformio.ini
│   ├── src/
│   │   └── main.cpp              # 副机程序源码
│   ├── include/
│   ├── lib/
│   └── test/
```

> **注意：** `main` 与 `vice` 代码几乎完全相同，唯一的区别是对端 **MAC 地址**。两个目录分别用 PlatformIO 独立编译烧录。

---

## 通信架构

```
┌──────────────────────┐              ┌──────────────────────┐
│     Sunburst_main    │              │    Sunburst_vice     │
│                      │              │                      │
│  ┌──────────────┐    │   ESP-NOW    │  ┌──────────────┐    │
│  │  HMI 触摸屏  │◄──►│ ◄══════════► │◄─┤  HMI 触摸屏  │    │
│  └──────┬───────┘    │   (2.4GHz)   │  └──────┬───────┘    │
│         │ Serial2    │              │         │ Serial2    │
│  ┌──────▼───────┐    │              │  ┌──────▼───────┐    │
│  │ ESP32 DOIT   │    │              │  │ ESP32 DOIT   │    │
│  │              │    │              │  │              │    │
│  │ WS2812 ◄─────┤GP18│              │  │ WS2812 ◄─────┤GP18│
│  └──────────────┘    │              │  └──────────────┘    │
└──────────────────────┘              └──────────────────────┘
```

### 交互流程

```
触摸屏 ──(%分隔符)──► Serial2 ──► ESP32 ──(ESP-NOW)──► 对端设备 ──► 屏幕显示 + LED亮起
```

---

## 快速上手

### 前置要求

- [Visual Studio Code](https://code.visualstudio.com/)
- [PlatformIO IDE 插件](https://platformio.org/install/ide?install=vscode)
- 两条 Micro-USB 数据线

### 编译烧录

```bash
# 1. 克隆仓库
git clone https://github.com/your-username/Sunburst.git
cd Sunburst

# 2. 修改对端 MAC 地址（重要！）
# 编辑 Sunburst_main/src/main.cpp 第 25 行
# 和 Sunburst_vice/src/main.cpp 第 23 行
# 将 send_MAC 改为对端设备的实际 MAC 地址

# 3a. 烧录主机 —— 用 VS Code 打开 Sunburst_main，点击 Upload
# 3b. 烧录副机 —— 用 VS Code 打开 Sunburst_vice，点击 Upload
```

> 也可以在终端中使用 PlatformIO Core CLI：
> ```bash
> cd Sunburst_main && pio run --target upload
> cd Sunburst_vice && pio run --target upload
> ```

---

## 配置说明

### MAC 地址

两台 ESP32 互相持有对方的 MAC 地址。使用前**必须修改**源码中的目标 MAC：

```cpp
// Sunburst_main/src/main.cpp  (发送给副机)
unsigned char send_MAC[] = {0x94, 0x54, 0xC5, 0xB1, 0x9F, 0x74};  // vice 的 MAC

// Sunburst_vice/src/main.cpp  (发送给主机)
unsigned char send_MAC[] = {0x94, 0x54, 0xC5, 0xAC, 0x0E, 0x20};  // main 的 MAC
```

> 💡 获取 ESP32 MAC 地址：上传一个简单的 `WiFi.macAddress()` 打印程序即可。

### 可调参数

| 参数 | 定义位置 | 默认值 | 说明 |
|------|---------|--------|------|
| `BRIGHT` | `main.cpp` L9 | `5` | LED 全局亮度 (0-255) |
| `LIGHT_PIN` | `main.cpp` L8 | `18` | WS2812 数据引脚 |
| LED 颜色 | `LED_Controller()` | 见下表 | 各状态颜色 |
| 串口波特率 | `setup()` | `115200` | Serial & Serial2 |

---

## 通信协议

### ESP-NOW 数据结构

```cpp
// 发送
typedef struct send_Message {
  String sent_Messsage = "";  // 文字内容
  String sent_emoji = "";     // 表情代码 (a-f)
} send_Message;

// 接收
typedef struct recv_Message {
  String recv_Messsage = "";  // 文字内容
  String recv_emoji = "";     // 表情代码 (a-f)
} recv_Message;
```

### 串口屏幕指令

屏幕与 ESP32 通过 **Serial2**（115200 bps）通信，协议规则：

| 指令 | 说明 | 示例 |
|------|------|------|
| `指令名=值\xff\xff\xff` | 控制屏幕控件 | `chat.t1.txt="你好"` + `\xff\xff\xff` |
| `信号%数据%` | ESP32 接收屏幕信号 | `SEND_MESSAGE_COMING%你好呀%` |
| `chat.detect.val=N\xff\xff\xff` | 触发表情动画 (1-6) | `chat.detect.val=3\xff\xff\xff` |

**屏幕 → ESP32 信号列表：**

| 信号 | 含义 |
|------|------|
| `START_ESP_NOW` | 启动 ESP-NOW 握手 |
| `SEND_MESSAGE_COMING` | 即将发送文字消息 |
| `SEND_EMOJI_COMING` | 即将发送表情 |
| `POP_CLICKED` | 气泡被点击 |
| `MATCHING` | 进入匹配状态 |

### 表情编码

| 代码 | 屏幕动画索引 |
|------|-------------|
| `a` | 1 |
| `b` | 2 |
| `c` | 3 |
| `d` | 4 |
| `e` | 5 |
| `f` | 6 |

---

## LED 动画参考

| 状态 | 颜色 | 效果 | 触发条件 |
|------|------|------|---------|
| 🔗 **匹配中 (MATCHING)** | 淡绿 `(125, 255, 122)` | 呼吸脉冲 ×2 | 屏幕发送 `MATCHING` 信号 |
| 📤 **发送中 (Sending)** | 暖橙 `(255, 210, 148)` | 渐隐渐亮 → 熄灭 | ESP-NOW 消息发送完成 |
| 📥 **接收中 (Receiving)** | 柔红 `(255, 114, 107)` | 渐隐渐亮 → 熄灭 | 收到对端 ESP-NOW 消息 |
| 💥 **点击气泡 (POP_CLICKED)** | 随机 HSV 色 | 闪亮 400ms → 熄灭 | 点击屏幕气泡 |

---

## 软件架构

```
setup()
  ├── 初始化串口 (Serial + Serial2)
  ├── 配置 WS2812 LED
  ├── 初始化 WiFi 为 STA 模式
  ├── 初始化 ESP-NOW
  └── 创建 FreeRTOS 任务 (Core 0)

getInfo() [FreeRTOS Task — Core 0]
  ├── ESP_NOW_STATUS == 1  →  esp_now_INIT()    // 握手配对
  ├── ESP_NOW_SENT == 1   →  esp_now_SEND()     // 发送消息
  ├── LED_SWITCH == 1     →  LED_Controller()   // 控制灯效
  └── EMOJI_CONTROLLER == 1 → emojiSwitcher()   // 触发表情

Serial2 中断回调 (onDataRecv)
  解析屏幕信号 → 设置对应的全局状态标志位
```

---

## 依赖

| 库 | 版本 | 用途 |
|----|------|------|
| [FastLED](https://github.com/FastLED/FastLED) | ^3.9.14 | WS2812 LED 驱动 |
| Arduino (ESP32) | — | ESP32 框架基础 |
| WiFi (ESP32 内置) | — | ESP-NOW 底层支持 |

---

## 开发环境

- **IDE**: VS Code + PlatformIO
- **开发板**: ESP32 DOIT DevKit V1
- **分区方案**: `huge_app.csv`
- **框架**: Arduino for ESP32

---

## 使用场景

1. 双方分别持有主机和副机
2. 上电后，两台设备自动通过 ESP-NOW 配对握手
3. 在 HMI 触摸屏上输入文字或选择表情发送
4. 对方的 LED 会闪烁"接收中"动画，屏幕同步显示消息
5. 点击屏幕上的气泡，对方的 LED 会闪烁随机色彩

---

## 开源协议

本项目采用 **MIT License** 开源，欢迎 Star、Fork 和 PR！

---

<p align="center">
  <sub>Made with ❤️ and ESP-NOW</sub>
</p>
