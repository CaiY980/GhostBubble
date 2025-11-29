#include "GhostManager.h"
#include "GhostBubble.h"
#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QActionGroup>
#include <QClipboard>
#include <QMimeData>
#include <QUdpSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkDatagram>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QBuffer>
#include <QImage>
#include <QPainter>
#include <QRandomGenerator>
#include <QDebug>
#include <QTimer>
#include <QScreen>
#include <QInputDialog>
#include <QMessageBox> // 用于显示历史记录
#include <QStyle>


GhostManager::GhostManager(QObject *parent)
    : QObject(parent), m_isInternalUpdate(false), m_isSendingEnabled(true), m_userLevel(1),
    m_sharedId("DEFAULT")
{
    m_deviceId = QUuid::createUuid().toString();

    setupTray();

    m_udpSocket = new QUdpSocket(this);
    m_udpSocket->bind(UDP_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    connect(m_udpSocket, &QUdpSocket::readyRead, this, &GhostManager::onUdpReadyRead);

    m_tcpServer = new QTcpServer(this);
    if (!m_tcpServer->listen(QHostAddress::Any, TCP_PORT)) {
        qDebug() << "TCP Server failed to start!";
    } else {
        connect(m_tcpServer, &QTcpServer::newConnection, this, &GhostManager::onTcpConnectionRequest);
    }

    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &GhostManager::onClipboardChanged);

    qDebug() << "Ghost Clipboard started. Device ID:" << m_deviceId;
    qDebug() << "Current Group Key:" << m_sharedId;
}

void GhostManager::setupTray() {
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));

    QMenu *menu = new QMenu();

    // === 历史记录 (新增) ===
    QAction *actHistory = new QAction("查看最近消息记录", this);
    connect(actHistory, &QAction::triggered, this, &GhostManager::showHistory);
    menu->addAction(actHistory);

    menu->addSeparator();

    // === 发送开关 ===
    QAction *actToggleSend = new QAction("允许发送剪贴板 (Sync)", this);
    actToggleSend->setCheckable(true);
    actToggleSend->setChecked(m_isSendingEnabled);
    connect(actToggleSend, &QAction::toggled, this, &GhostManager::setSendingEnabled);
    menu->addAction(actToggleSend);

    menu->addSeparator();

    // === 群组密钥设置 ===
    QAction *actSetId = new QAction("设置群组密钥 (Group Key)", this);
    connect(actSetId, &QAction::triggered, this, &GhostManager::setSharedId);
    menu->addAction(actSetId);

    // === 等级设置 ===
    QMenu *levelMenu = menu->addMenu("设置我的等级");
    QActionGroup *levelGroup = new QActionGroup(this);

    for (int i = 1; i <= 5; ++i) {
        QString text = QString("Level %1").arg(i);
        QAction *act = levelMenu->addAction(text);
        act->setCheckable(true);
        if (i == m_userLevel) act->setChecked(true);
        levelGroup->addAction(act);
        connect(act, &QAction::triggered, [this, i]() { setUserLevel(i); });
    }

    menu->addSeparator();

    // === 调试菜单 ===
    QMenu *debugMenu = menu->addMenu("调试 (Debug)");
    debugMenu->addAction("模拟收到文本", this, &GhostManager::mockReceiveText);
    debugMenu->addAction("模拟收到图片", this, &GhostManager::mockReceiveImage);

    menu->addSeparator();
    QAction *actQuit = new QAction("退出", this);
    connect(actQuit, &QAction::triggered, this, &GhostManager::quitApp);
    menu->addAction(actQuit);

    m_trayIcon->setContextMenu(menu);
    m_trayIcon->show();
    m_trayIcon->setToolTip(QString("Ghost Clipboard - LV:%1 Key:%2").arg(m_userLevel).arg(m_sharedId));
}

void GhostManager::setSharedId() {
    bool ok;
    QString text = QInputDialog::getText(nullptr, "设置群组密钥",
                                         "请输入共享的密钥ID (例如: TeamAlpha):", QLineEdit::Normal,
                                         m_sharedId, &ok);
    if (ok && !text.isEmpty()) {
        m_sharedId = text.trimmed();
        m_trayIcon->setToolTip(QString("Ghost Clipboard - LV:%1 Key:%2").arg(m_userLevel).arg(m_sharedId));
        m_trayIcon->showMessage("设置已更新", QString("群组密钥已设置为: %1").arg(m_sharedId));
        qDebug() << "Group Key updated to:" << m_sharedId;
    }
}

void GhostManager::showHistory() {
    if (m_historyList.isEmpty()) {
        QMessageBox::information(nullptr, "剪贴板历史记录", "目前没有未处理的剪贴板历史记录。");
        return;
    }

    QStringList items;
    // 从最新的记录开始显示 (从末尾往前遍历)
    for (int i = m_historyList.size() - 1; i >= 0; --i) {
        const ClipboardMessage &msg = m_historyList.at(i);

        // 格式化内容，如果是图片则显示状态
        QString status = "";
        if (msg.type == "text") {
            status = msg.content.left(30).simplified();
        } else if (msg.type == "image-offer") {
            status = msg.rawData.isEmpty() ? "图片通知(未保存数据)" : QString("图片数据(%1 KB)").arg(msg.rawData.size() / 1024);
        }

        QString item = QString("[%1/%2] %3: %4")
                           .arg(m_historyList.size() - i) // 序号 (1是最新)
                           .arg(msg.timestamp.toString("hh:mm:ss"))
                           .arg(msg.type == "text" ? "文本(T)" : "图片(I)")
                           .arg(status);
        items.append(item);
    }

    bool ok;
    QString selection = QInputDialog::getItem(nullptr, "剪贴板历史记录",
                                              "选择要重新复制的内容:", items, 0, false, &ok);

    if (ok && !selection.isEmpty()) {
        int selectedIndexInReversedList = items.indexOf(selection);
        int selectedIndex = m_historyList.size() - 1 - selectedIndexInReversedList;

        if (selectedIndex >= 0 && selectedIndex < m_historyList.size()) {
            const ClipboardMessage &msg = m_historyList.at(selectedIndex);

            if (msg.type == "text") {
                // 文本：直接复制内容
                m_isInternalUpdate = true;
                QApplication::clipboard()->setText(msg.content);
                m_lastCopiedText = msg.content;
                m_trayIcon->showMessage("历史记录", "文本已重新复制到剪贴板。");
            } else if (msg.type == "image-offer") {
                // 图片：检查是否有保存数据
                if (!msg.rawData.isEmpty()) {
                    QImage image;
                    if (image.loadFromData(msg.rawData, "PNG")) {
                        m_isInternalUpdate = true;
                        QApplication::clipboard()->setImage(image);
                        m_trayIcon->showMessage("历史记录", "图片已从历史记录中恢复到剪贴板。");
                    } else {
                        QMessageBox::critical(nullptr, "错误", "无法解析历史记录中的图片数据。");
                    }
                } else {
                    QMessageBox::warning(nullptr, "历史记录", "此记录仅为图片通知，没有保存原始图片数据。");
                }
            }
        }
    }
}

void GhostManager::addToHistory(const ClipboardMessage &message) {
    // 1. 添加到列表
    m_historyList.append(message);

    // 2. 检查并移除超出的旧记录
    while (m_historyList.size() > MAX_HISTORY_SIZE) {
        m_historyList.removeFirst();
    }
}

void GhostManager::mockReceiveText() {
    QStringList texts = {
        "Hello World from Debugger! (Key: " + m_sharedId + ")",
        "这是一条模拟的文本消息。",
        "C++ Qt 6.0\nCross-Platform Development",
        "https://www.qt.io"
    };
    QString content = texts[QRandomGenerator::global()->bounded(texts.size())];

    ClipboardMessage msg = { QDateTime::currentDateTime(), content, "text", 1, QHostAddress::LocalHost, {} };
    addToHistory(msg);
    showGhostBubble(content, 1, "text", QHostAddress::LocalHost);
}

void GhostManager::mockReceiveImage() {
    QImage image(400, 300, QImage::Format_RGB32);
    image.fill(QColor(QRandomGenerator::global()->bounded(256),
                      QRandomGenerator::global()->bounded(256),
                      QRandomGenerator::global()->bounded(256)));

    QPainter p(&image);
    p.setPen(Qt::white);
    QFont font = p.font();
    font.setPointSize(20);
    p.setFont(font);
    p.drawText(image.rect(), Qt::AlignCenter, "Mock Image\n(Debug Mode)");

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    m_cachedImageData = byteArray;

    QString desc = QString("Mock Size: %1 KB").arg(byteArray.size() / 1024);

    // 这里只保存通知，rawData为空
    ClipboardMessage msg = { QDateTime::currentDateTime(), desc, "image-offer", 1, QHostAddress::LocalHost, {} };
    addToHistory(msg);

    showGhostBubble(desc, 1, "image-offer", QHostAddress::LocalHost);
}

void GhostManager::setSendingEnabled(bool enabled) {
    m_isSendingEnabled = enabled;
    QString status = enabled ? "开启" : "暂停";
    m_trayIcon->showMessage("设置已更新", QString("剪贴板发送已%1").arg(status));
    qDebug() << "Clipboard sending enabled:" << enabled;
}

void GhostManager::setUserLevel(int level) {
    m_userLevel = level;
    m_trayIcon->setToolTip(QString("Ghost Clipboard - LV:%1 Key:%2").arg(m_userLevel).arg(m_sharedId));
    m_trayIcon->showMessage("等级已变更", QString("Level %1").arg(level));
}

void GhostManager::onClipboardChanged() {
    if (m_isInternalUpdate) {
        m_isInternalUpdate = false;
        return;
    }

    if (!m_isSendingEnabled) {
        return;
    }

    QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    if (mimeData->hasText() && !mimeData->hasImage()) {
        QString text = mimeData->text();
        if (text == m_lastCopiedText) return;
        m_lastCopiedText = text;

        qDebug() << "Broadcasting text...";
        sendClipboardBroadcast("text", text);
    }
    else if (mimeData->hasImage()) {
        QImage image = qvariant_cast<QImage>(mimeData->imageData());
        if (!image.isNull()) {
            QByteArray byteArray;
            QBuffer buffer(&byteArray);
            buffer.open(QIODevice::WriteOnly);
            image.save(&buffer, "PNG");

            m_cachedImageData = byteArray;

            QString desc = QString("Size: %1 KB").arg(byteArray.size() / 1024);
            qDebug() << "Broadcasting image offer..." << desc;
            sendClipboardBroadcast("image-offer", desc);
        }
    }
}

void GhostManager::sendClipboardBroadcast(const QString &type, const QString &dataOrDesc) {
    QJsonObject json;
    json["id"] = m_deviceId;
    json["type"] = type;
    json["data"] = dataOrDesc;
    json["level"] = m_userLevel;
    json["group_key"] = m_sharedId;

    QJsonDocument doc(json);
    m_udpSocket->writeDatagram(doc.toJson(QJsonDocument::Compact), QHostAddress::Broadcast, UDP_PORT);
}

void GhostManager::onUdpReadyRead() {
    while (m_udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_udpSocket->receiveDatagram();
        QByteArray data = datagram.data();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull() || !doc.isObject()) continue;

        QJsonObject json = doc.object();

        QString senderId = json["id"].toString();
        if (senderId == m_deviceId) continue;

        QString incomingKey = json["group_key"].toString();
        if (incomingKey != m_sharedId) {
            qDebug() << "Message dropped: Group Key mismatch. Expected:" << m_sharedId << "Received:" << incomingKey;
            continue;
        }

        int msgLevel = json["level"].toInt(1);
        if (msgLevel > m_userLevel) continue;

        QString type = json["type"].toString();
        QString content = json["data"].toString();

        // 1. 缓存消息 (只保存通知，rawData为空)
        ClipboardMessage msg = {
            QDateTime::currentDateTime(),
            content,
            type,
            msgLevel,
            datagram.senderAddress(),
            {} // rawData is empty for the initial offer
        };
        addToHistory(msg);

        // 2. 显示气泡
        showGhostBubble(content, msgLevel, type, datagram.senderAddress());
    }
}

void GhostManager::showGhostBubble(const QString &content, int level, const QString &type, const QHostAddress &senderAddress) {
    if (type == "text" && content == QApplication::clipboard()->text()) return;

    GhostBubble *bubble = new GhostBubble(content, level, type, senderAddress);
    connect(bubble, &GhostBubble::contentAccepted, this, &GhostManager::onBubbleContentAccepted);

    QRect screenGeo = QApplication::primaryScreen()->availableGeometry();
    int xPos = screenGeo.width() - 170;
    int yPos = QRandomGenerator::global()->bounded(100, screenGeo.height() - 300);

    bubble->move(xPos, yPos);
    bubble->show();
}

void GhostManager::onBubbleContentAccepted(const QString &content, const QString &type, const QHostAddress &senderAddress) {
    if (type == "text") {
        m_isInternalUpdate = true;
        QApplication::clipboard()->setText(content);
        m_lastCopiedText = content;
    } else if (type == "image-offer") {
        downloadImageFromSender(senderAddress);
    }
}

void GhostManager::onTcpConnectionRequest() {
    QTcpSocket *clientSocket = m_tcpServer->nextPendingConnection();
    if (!m_cachedImageData.isEmpty()) {
        clientSocket->write(m_cachedImageData);
        clientSocket->flush();
    }
    connect(clientSocket, &QTcpSocket::disconnected, clientSocket, &QTcpSocket::deleteLater);
    QTimer::singleShot(2000, clientSocket, &QTcpSocket::close);
}

void GhostManager::downloadImageFromSender(const QHostAddress &address) {
    QTcpSocket *socket = new QTcpSocket(this);

    // 使用 QVariant 存储 QByteArray 指针
    QByteArray *buffer = new QByteArray();
    socket->setProperty("buffer", QVariant::fromValue(buffer));

    connect(socket, &QTcpSocket::readyRead, [buffer, socket, this]() {
        buffer->append(socket->readAll());
    });

    connect(socket, &QTcpSocket::disconnected, [socket, buffer, this]() {
        if (!buffer->isEmpty()) {
            QImage image;
            if (image.loadFromData(*buffer, "PNG")) {
                m_isInternalUpdate = true;
                QApplication::clipboard()->setImage(image);

                // 🌟 关键修改：图片下载成功后，找到最近的图片通知记录并保存数据
                for (int i = m_historyList.size() - 1; i >= 0; --i) {
                    ClipboardMessage &msg = m_historyList[i];
                    if (msg.type == "image-offer" && msg.rawData.isEmpty()) {
                        msg.rawData = *buffer; // 存储下载后的图片数据
                        qDebug() << "Image data successfully saved to history at index" << i;
                        break;
                    }
                }

                m_trayIcon->showMessage("幽灵剪贴板", "图片已接收并写入剪贴板！");
            }
        }
        delete buffer;
        socket->deleteLater();
    });

    socket->connectToHost(address, TCP_PORT);
}

void GhostManager::quitApp() {
    QApplication::quit();
}
