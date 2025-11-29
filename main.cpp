#include <QApplication>
#include "GhostManager.h"

int main(int argc, char *argv[]) {
    // 1. 高分屏适配设置 (针对 Qt 5 的兼容性设置，Qt 6 默认已开启)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    // 2. 初始化应用程序对象
    QApplication app(argc, argv);

    // 3. 【关键设置】禁止"最后一个窗口关闭时退出程序"
    // 因为我们的程序是系统托盘程序 (System Tray App)，
    // 即使气泡窗口消失了，程序也必须在后台继续运行，等待下一次剪贴板消息。
    app.setQuitOnLastWindowClosed(false);

    // 4. 启动核心管理器
    // GhostManager 的构造函数里会执行所有初始化工作：
    // - 创建系统托盘图标
    // - 绑定 UDP 端口监听局域网
    // -
    // - 启动 TCP 服务器
    GhostManager manager;

    // 5. 进入 Qt 的主事件循环
    return app.exec();
}
