# 👻 LAN Ghost Clipboard (局域网幽灵剪贴板)

**LAN Ghost Clipboard** 是一款基于 Qt 6 开发的跨平台局域网剪贴板同步工具。它旨在解决 Windows、macOS 和 Linux 设备间繁琐的文本与图片传输问题，专为需要高频协作且保持专注的小型团队（如软件开发小组、数学建模竞赛团队）设计。

通过非侵入式的 UI 设计与高效的局域网广播机制，本工具能够在不打断用户工作流的前提下，实现代码片段、IP 地址、关键参数及截图的极速共享。

## 主要特性

### 1. 非侵入式交互 (Non-intrusive Workflow)
* **边缘浮窗机制**：当接收到剪贴板内容时，屏幕边缘会自动浮现半透明的提示气泡。
* **即时获取**：用户仅需将气泡拖入屏幕中心，内容即自动写入本地剪贴板。
* **自动消散**：若无需处理，气泡将在数秒后自动淡出消失，避免遮挡终端或代码编辑器，最大程度减少对当前工作的干扰。

### 2. 安全与群组隔离 (Security & Isolation)
* **群组密钥 (Group Key)**：支持设置自定义密钥。仅持有相同密钥的设备可互相通信，确保在公共局域网环境下（如比赛现场、公共办公室）的数据边界安全，防止信息被未授权设备接收。
* **权限分级**：支持设置发送与接收的安全等级。高优先级用户（如组长）可发送全员可见的指令，普通用户可设置为仅接收特定级别的信息，以减少无关信息的干扰。
* **隐私模式**：支持一键暂停同步功能，确保本地敏感操作不被广播。

### 3. 高性能网络传输
* **混合传输协议**：
    * **UDP 广播**：用于设备自动发现与纯文本传输，实现毫秒级低延迟。
    * **TCP 点对点**：用于图片与大文件传输，确保数据完整性且不占用公用广播带宽。
* **零配置连接**：无需配置服务器 IP，接入同一局域网即可自动发现对等节点。

### 4. 跨平台支持
基于 Qt 6 框架构建，提供统一的用户体验，完美兼容：
* **Windows** (10/11)
* **macOS** (12+)
* **Linux** (Ubuntu, Kali, Arch 等)

## 技术栈 (Tech Stack)

* **GUI 框架**: Qt 6 (Widgets)
* **网络通信**:
    * `QUdpSocket`: 用于信令交互与文本广播。
    * `QTcpServer` / `QTcpSocket`: 用于媒体数据流传输。
* **数据序列化**: JSON (轻量级协议封装)。
* **视觉效果**: 使用 `QPropertyAnimation` 与 `QGraphicsOpacityEffect` 实现 UI 动效。

## 构建指南 (Build Instructions)

### 前置要求

确保开发环境中已安装以下组件：

* **C++ 编译器**: 支持 C++17 标准 (GCC, Clang, MSVC)。
* **Qt 6 SDK**: 需包含 `Core`, `Gui`, `Widgets`, `Network` 模块。
* **CMake**: 版本 3.16 或更高。

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
