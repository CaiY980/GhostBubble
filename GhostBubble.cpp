#include "GhostBubble.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTimer>
#include <QPainter>
#include <QRadialGradient>
#include <QApplication>
#include <QScreen>
#include <QMouseEvent>
#include <QProcessEnvironment>

GhostBubble::GhostBubble(const QString &content, int level, const QString &type,
                         const QHostAddress &senderAddress, QWidget *parent)
    : QWidget(parent), m_content(content), m_level(level), m_type(type),
    m_senderAddress(senderAddress), m_isDragging(false)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    setFixedSize(220, 130);
    setupUi();

    m_dismissTimer = new QTimer(this);
    m_dismissTimer->setSingleShot(true);
    connect(m_dismissTimer, &QTimer::timeout, this, &GhostBubble::fadeOutAndClose);

    m_opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(m_opacityEffect);

    m_anim = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    m_anim->setDuration(1000);
    m_anim->setStartValue(0.0);
    m_anim->setEndValue(0.9);
    m_anim->setEasingCurve(QEasingCurve::OutQuad);
    m_anim->start();

    m_dismissTimer->start(5000);
}

QColor GhostBubble::getLevelColor() const {
    switch (m_level) {
    case 1: return QColor(100, 200, 255, 180);
    case 2: return QColor(100, 255, 150, 180);
    case 3: return QColor(255, 200, 50, 180);
    case 4: return QColor(255, 100, 50, 180);
    case 5: return QColor(255, 50, 50, 180);
    default: return QColor(100, 200, 255, 180);
    }
}

void GhostBubble::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);

    QLabel *levelLabel = new QLabel(QString("LV.%1").arg(m_level), this);
    levelLabel->setStyleSheet("color: rgba(255,255,255,0.7); font-weight: bold; font-size: 10px;");
    levelLabel->setAlignment(Qt::AlignRight);
    layout->addWidget(levelLabel);

    m_label = new QLabel(this);

    if (m_type == "text") {
        QString displayText = m_content;
        if (displayText.length() > 20) displayText = displayText.left(20) + "...";
        displayText.replace("\n", " ");
        m_label->setText(QString("👻 来自 LAN:\n%1").arg(displayText));
    } else if (m_type == "image-offer") {
        m_label->setText(QString("🖼️ 收到一张图片\n%1").arg(m_content));
    }

    m_label->setStyleSheet("QLabel { color: white; font-weight: bold; font-family: 'Segoe UI', sans-serif; font-size: 14px; }");
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setWordWrap(true);

    layout->addWidget(m_label);
}

void GhostBubble::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRadialGradient gradient(rect().center(), width() / 1.5);
    QColor baseColor = getLevelColor();
    gradient.setColorAt(0, baseColor);
    gradient.setColorAt(1, QColor(baseColor.red(), baseColor.green(), baseColor.blue(), 0));

    painter.setBrush(QBrush(gradient));
    painter.setPen(Qt::NoPen);
    QRect drawRect = rect().adjusted(10, 10, -10, -10);
    painter.drawRoundedRect(drawRect, 20, 20);
}

void GhostBubble::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_dragStartPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
#else
        m_dragStartPos = event->globalPos() - frameGeometry().topLeft();
#endif
        setCursor(Qt::ClosedHandCursor);
        m_dismissTimer->stop();
        m_anim->stop();
        m_opacityEffect->setOpacity(1.0);
    }
}

void GhostBubble::mouseMoveEvent(QMouseEvent *event) {
    if (m_isDragging) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        move(event->globalPosition().toPoint() - m_dragStartPos);
#else
        move(event->globalPos() - m_dragStartPos);
#endif
    }
}

void GhostBubble::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    if (m_isDragging) {
        m_isDragging = false;
        setCursor(Qt::ArrowCursor);

        int currentX = this->x();
        int screenWidth = QApplication::primaryScreen()->availableGeometry().width();

        if (currentX < (screenWidth - 200)) {
            confirmCopy();
        } else {
            m_dismissTimer->start(2000);
        }
    }
}

void GhostBubble::confirmCopy() {
    emit contentAccepted(m_content, m_type, m_senderAddress);

    m_label->setText(m_type == "text" ? "✨ 已复制!" : "⏬ 下载中...");
    m_label->setStyleSheet("QLabel { color: #AAFF00; font-weight: bold; font-size: 16px; }");

    m_anim->stop();
    m_anim->setDuration(500);
    m_anim->setStartValue(1.0);
    m_anim->setEndValue(0.0);
    connect(m_anim, &QPropertyAnimation::finished, this, &GhostBubble::close);
    m_anim->start();
}

void GhostBubble::fadeOutAndClose() {
    m_label->setText("💨");
    m_anim->stop();
    m_anim->setDuration(1000);
    m_anim->setStartValue(m_opacityEffect->opacity());
    m_anim->setEndValue(0.0);
    connect(m_anim, &QPropertyAnimation::finished, this, &GhostBubble::onFadeOutFinished);
    m_anim->start();
}

void GhostBubble::onFadeOutFinished() {
    close();
}
