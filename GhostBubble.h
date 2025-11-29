#ifndef GHOSTBUBBLE_H
#define GHOSTBUBBLE_H

#include <QWidget>
#include <QHostAddress>
#include <QPoint>
#include <QColor>

class QLabel;
class QTimer;
class QGraphicsOpacityEffect;
class QPropertyAnimation;

class GhostBubble : public QWidget {
    Q_OBJECT

public:
    explicit GhostBubble(const QString &content, int level, const QString &type,
                         const QHostAddress &senderAddress, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void contentAccepted(const QString &content, const QString &type, const QHostAddress &senderAddress);

private slots:
    void fadeOutAndClose();
    void onFadeOutFinished();

private:
    void setupUi();
    void confirmCopy();
    QColor getLevelColor() const;

    QString m_content;
    int m_level;
    QString m_type;
    QHostAddress m_senderAddress;
    QLabel *m_label;

    bool m_isDragging;
    QPoint m_dragStartPos;

    QTimer *m_dismissTimer;
    QGraphicsOpacityEffect *m_opacityEffect;
    QPropertyAnimation *m_anim;
};

#endif // GHOSTBUBBLE_H
