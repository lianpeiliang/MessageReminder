#include "toastwidget.h"

#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPointer>
#include <QPropertyAnimation>
#include <QScreen>
#include <QTimer>

class ToastPopup : public QWidget
{
public:
    ToastPopup(const QString &message, QScreen *screen, QWidget *parent)
        : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint
                          | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus),
          targetScreen(screen), animation(new QPropertyAnimation(this, "pos", this))
    {
        setWindowTitle(QString::fromUtf8("消息提示"));
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_ShowWithoutActivating);
        setFocusPolicy(Qt::NoFocus);
        setFixedSize(280, 84);

        auto *icon = new QLabel(QString::fromUtf8("✓"), this);
        icon->setFixedSize(30, 30);
        icon->setAlignment(Qt::AlignCenter);
        icon->setStyleSheet("color: #86efac; background: #244638;"
                            "border-radius: 15px; font-size: 20px;");
        auto *label = new QLabel(message, this);
        label->setStyleSheet("color: #f8fafc; font-size: 16px;");
        label->setTextFormat(Qt::PlainText);
        label->setWordWrap(true);

        auto *layout = new QHBoxLayout(this);
        layout->setContentsMargins(22, 18, 22, 18);
        layout->setSpacing(14);
        layout->addWidget(icon);
        layout->addWidget(label, 1);

        animation->setDuration(180);
        animation->setEasingCurve(QEasingCurve::OutCubic);
    }

    void moveTo(const QPoint &position)
    {
        animation->stop();
        if (!isVisible()) {
            move(position);
            return;
        }
        animation->setStartValue(pos());
        animation->setEndValue(position);
        animation->start();
    }

    QPointer<QScreen> targetScreen;

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(QColor("#3e4c60"), 1));
        painter.setBrush(QColor("#1e293b"));
        painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 12, 12);
    }

private:
    QPropertyAnimation *animation;
};

ToastWidget::ToastWidget(QWidget *parent)
    : QWidget(parent)
{
    // This widget owns the notifications; only the individual popups are shown.
    hide();
}

void ToastWidget::showMessage(const QString &message, int durationMs, QScreen *screen)
{
    if (!screen)
        screen = QGuiApplication::primaryScreen();
    if (!screen)
        return;

    auto *popup = new ToastPopup(message, screen, this);
    m_messages.append(popup);
    arrangeMessages(screen);
    popup->show();
    popup->raise();

    // Destroying the popup also cancels its timeout callback.
    QTimer::singleShot(qMax(1, durationMs), Qt::PreciseTimer, popup, [this, popup]() {
        QScreen *screen = popup->targetScreen.data();
        m_messages.removeOne(popup);
        popup->hide();
        popup->deleteLater();
        if (screen)
            arrangeMessages(screen);
    });
}

void ToastWidget::arrangeMessages(QScreen *screen)
{
    // Each screen has its own stack. Newest is closest to the taskbar.
    const QRect area = screen->availableGeometry();
    const int margin = 20;
    const int spacing = 12;
    int bottom = area.bottom() + 1 - margin;
    for (int i = m_messages.size() - 1; i >= 0; --i) {
        ToastPopup *popup = m_messages.at(i);
        if (popup->targetScreen != screen)
            continue;
        popup->moveTo(QPoint(qMax(area.left(), area.right() + 1 - popup->width() - margin),
                             bottom - popup->height()));
        bottom -= popup->height() + spacing;
    }
}
