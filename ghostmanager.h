#ifndef GHOSTMANAGER_H
#define GHOSTMANAGER_H

#include <QObject>
#include <QHostAddress>
#include <QList>
#include <QDateTime>
#include <QByteArray> // 引入 QByteArray

class QSystemTrayIcon;
class QUdpSocket;
class QTcpServer;

// 定义消息结构体，用于缓存
struct ClipboardMessage {
    QDateTime timestamp;
    QString content;        // 用于文本内容或图片描述
    QString type;
    int level;
    QHostAddress senderAddress;
    QByteArray rawData;     // 新增：用于存储图片数据的字节数组
};

class GhostManager : public QObject {
    Q_OBJECT

public:
    explicit GhostManager(QObject *parent = nullptr);

private slots:
    void onClipboardChanged();
    void onUdpReadyRead();
    void onTcpConnectionRequest();
    void onBubbleContentAccepted(const QString &content, const QString &type, const QHostAddress &senderAddress);
    void setUserLevel(int level);
    void setSendingEnabled(bool enabled);
    void quitApp();

    void setSharedId();

    void showHistory();

    // 调试功能
    void mockReceiveText();
    void mockReceiveImage();

private:
    void setupTray();
    void showGhostBubble(const QString &content, int level, const QString &type, const QHostAddress &senderAddress);
    void sendClipboardBroadcast(const QString &type, const QString &dataOrDesc);
    void downloadImageFromSender(const QHostAddress &address);
    void addToHistory(const ClipboardMessage &message);

    QSystemTrayIcon *m_trayIcon;
    QUdpSocket *m_udpSocket;
    QTcpServer *m_tcpServer;

    QString m_deviceId;
    QString m_lastCopiedText;
    QByteArray m_cachedImageData;
    bool m_isInternalUpdate;
    bool m_isSendingEnabled;

    int m_userLevel;

    QString m_sharedId;

    // 缓存队列
    QList<ClipboardMessage> m_historyList;
    const int MAX_HISTORY_SIZE = 20; // 最大缓存条目数

    const quint16 UDP_PORT = 45454;
    const quint16 TCP_PORT = 45455;
};

#endif // GHOSTMANAGER_H
