# 👻 LAN Ghost Clipboard (局域网幽灵剪贴板)

**LAN Ghost Clipboard** 是一个跨平台的局域网剪贴板同步工具，旨在解决多设备（Windows, macOS, Linux）之间传输文字和图片繁琐的问题。

**特别适合场景**：数模等比赛或开发小组(3-5人)。它允许小队成员在高度专注的状态下，极速共享 IP、Flag、代码片段或战术截图，而无需切换窗口或使用臃肿的聊天软件。

## ✨ 核心玩法 (The "Ghost" Concept)

告别烦人的弹窗和自动写入，一切为了**保持专注**。

1. **无感发现**：队友 A 复制了一段 Shell 命令，队友 B 的屏幕边缘会自动浮现一个半透明的“幽灵气泡”。
2. **拖拽即用**：
   - 👉 **需要情报？** 鼠标一甩，将气泡从边缘“拖”入屏幕，内容即刻进入你的剪贴板，直接粘贴即可。
   - 👻 **无关信息？** 别理它，几秒后气泡如烟雾般消散，绝不遮挡你的终端或代码编辑器。

## 🏆 专为比赛与团队协作打造

- ⚡ **极速情报同步**：比聊天软件更快。复制即广播，无需 `Alt+Tab` 切换窗口粘贴发送。
- 🔒 **战队物理隔离**：利用 **群组密钥 (Group Key)** 功能，确保你的情报只在自家队员之间流转，防止被同一局域网下的竞争对手窃听。
- 🚫 **零干扰模式**：半透明气泡仅在边缘呼吸，不抢占焦点，不打断你的 Coding 或操作流。

## 🚀 功能特性

- **跨平台支持**：基于 Qt 6 开发，完美兼容 Windows、macOS 和 Linux（Kali/Ubuntu 等）。
- **零配置连接**：基于 UDP 广播自动发现，插上网线/连上 Wi-Fi 即可开战。
- **富媒体传输**：
  - 📝 **文本**：实时广播，毫秒级延迟。
  - 🖼️ **图片**：支持截图和战术地图传输（基于 TCP 点对点传输，不占公用带宽）。
- **隐私与安全**：
  - 🔐 **群组密钥 (Group Key)**：只有输入相同 Key 的设备才能互通。
  - 🛡️ **安全等级 (Security Levels)**：队长可设置高等级，向全员发送重要指令；队员设置低等级，避免琐碎信息干扰队长。
  - ⏸️ **一键静默**：比赛关键时刻可一键暂停同步，保护本地操作隐私。
- **历史回溯**：内置缓存队列，错过了刚才的 Flag？右键菜单一键找回。

## 🛠️ 构建指南 (Build Instructions)

### 前置要求

- C++17 编译器 (GCC, Clang, MSVC)
- Qt 6 SDK (包含 `Core`, `Gui`, `Widgets`, `Network` 模块)
- CMake 3.16+

### 编译步骤

```
# 1. 克隆仓库
git clone [https://github.com/your-username/lan-ghost-clipboard.git](https://github.com/your-username/lan-ghost-clipboard.git)
cd lan-ghost-clipboard

# 2. 创建构建目录
mkdir build
cd build

# 3. 配置与编译
# 注意：如果 Qt 安装在非标准路径，可能需要指定 -DCMAKE_PREFIX_PATH
cmake ..
cmake --build .

# 4. 运行
./GhostClipboard
```

## 📖 使用说明

程序启动后会常驻系统托盘（System Tray）。

1. **发送情报**：在任意工具中按下 `Ctrl+C` (或 `Cmd+C`)，情报即自动广播给队友。
2. **绑定战队**：右键托盘图标 -> `设置群组密钥`。**重要**：所有队员必须输入相同的 Key（如 `Team_Alpha`），否则无法通信。
3. **权限管理**：右键托盘图标 -> `设置我的等级`。
   - **指挥官/队长**：建议设置为高等级发送指令。
   - **普通队员**：只能收到 **等级 <= 自身等级** 的消息，避免信息过载。
4. **情报回溯**：右键托盘图标 -> `查看最近消息记录`，查看刚才错过的 IP 或截图。

## 🔧 技术栈

- **GUI Framework**: Qt 6 (Widgets)
- **Network**:
  - `QUdpSocket`: 用于高频、低延迟的广播与发现。
  - `QTcpServer/Socket`: 用于稳定传输大文件/图片。
- **Data Format**: JSON (轻量级协议封装)。
- **Animation**: `QPropertyAnimation` & `QGraphicsOpacityEffect` (丝滑的 UI 动效)。
