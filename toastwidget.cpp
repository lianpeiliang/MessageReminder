#include "toastwidget.h"

#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPointer>
#include <QPropertyAnimation>
#include <QScreen>
#include <QTimer>
#include <QWindow>

namespace {
constexpr int PopupWidth = 280;
constexpr int PopupHeight = 84;
constexpr int PopupMargin = 20;
constexpr int PopupSpacing = 12;
}

class ToastPopup : public QWidget
{
public:
    ToastPopup(const QString &message, ToastWidget::Level level, QScreen *screen, QWidget *parent)
        : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint
                          | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus),
          targetScreen(screen), animation(new QPropertyAnimation(this, "pos", this))
    {
        setWindowTitle(QString::fromUtf8("消息提示"));
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_ShowWithoutActivating);
        setFocusPolicy(Qt::NoFocus);
        setFixedSize(PopupWidth, PopupHeight);

        QString symbol = QStringLiteral("i");
        QString foreground = QStringLiteral("#93c5fd");
        QString iconBackground = QStringLiteral("#1e3a5f");
        background = QColor("#172b45");
        border = QColor("#3b82f6");
        switch (level) {
        case ToastWidget::Level::Info:
            break;
        case ToastWidget::Level::Warning:
            symbol = QStringLiteral("!");
            foreground = QStringLiteral("#fcd34d");
            iconBackground = QStringLiteral("#60451a");
            background = QColor("#3b2e18");
            border = QColor("#f59e0b");
            break;
        case ToastWidget::Level::Error:
            symbol = QString::fromUtf8("×");
            foreground = QStringLiteral("#fca5a5");
            iconBackground = QStringLiteral("#662828");
            background = QColor("#401f27");
            border = QColor("#ef4444");
            break;
        }

        auto *icon = new QLabel(symbol, this);
        icon->setObjectName(QStringLiteral("toastIcon"));
        icon->setFixedSize(30, 30);
        icon->setAlignment(Qt::AlignCenter);
        icon->setStyleSheet(QStringLiteral("color: %1; background: %2;"
                            "border-radius: 15px; font-size: 20px; font-weight: bold;")
                            .arg(foreground, iconBackground));
        auto *label = new QLabel(message, this);
        label->setObjectName(QStringLiteral("toastMessage"));
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

        // Create the native window on the intended screen before moving/showing it.
        // Otherwise Qt 5 may interpret the first position using the owner's DPI.
        // Seed the geometry too: Windows chooses the native monitor at creation.
        move(screen->availableGeometry().center() - rect().center());
        winId();
        windowHandle()->setScreen(screen);
        // Native screen creation may have rescaled the cached QWidget size.
        // Restore logical dimensions before calculating the stack positions.
        setFixedSize(PopupWidth, PopupHeight);
    }

    void moveTo(const QPoint &position)
    {
        targetPosition = position;
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
    QPoint targetPosition;

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(border, 1));
        painter.setBrush(background);
        painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 12, 12);
    }

private:
    QColor background;
    QColor border;
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
    showMessage(message, Level::Info, durationMs, screen);
}

void ToastWidget::setMaxVisibleMessages(int count)
{
    m_maxVisibleMessages = qMax(1, count);
    // When lowering the limit, let existing notifications finish normally.
    processQueue();
}

void ToastWidget::showMessage(const QString &message, Level level, int durationMs, QScreen *screen)
{
    if (!screen)
        screen = QGuiApplication::primaryScreen();
    if (!screen)
        return;

    m_pendingMessages.enqueue(PendingMessage{message, level, qMax(1, durationMs), screen});
    processQueue();
}

void ToastWidget::processQueue()
{
    while (m_messages.size() < m_maxVisibleMessages && !m_pendingMessages.isEmpty()) {
        QScreen *screen = m_pendingMessages.head().screen.data();
        if (!screen)
            screen = QGuiApplication::primaryScreen();
        if (!screen)
            return;

        // Also respect physical screen height (84px popup, 12px gap, 20px margins).
        const int screenCapacity = qMax(1, (screen->availableGeometry().height()
            - 2 * PopupMargin + PopupSpacing) / (PopupHeight + PopupSpacing));
        int screenCount = 0;
        for (ToastPopup *popup : m_messages)
            if (popup->targetScreen == screen)
                ++screenCount;
        if (screenCount >= screenCapacity)
            return;

        const PendingMessage message = m_pendingMessages.dequeue();
        displayMessage(message, screen);
    }
}

void ToastWidget::displayMessage(const PendingMessage &message, QScreen *screen)
{
    auto *popup = new ToastPopup(message.text, message.level, screen, this);
    m_messages.append(popup);
    arrangeMessages(screen);
    popup->setWindowOpacity(0.0);
    popup->show();
    // Let the native DPI/geometry events settle before revealing the new popup.
    QTimer::singleShot(0, popup, [popup]() {
        popup->move(popup->targetPosition);
        popup->windowHandle()->setGeometry(QRect(popup->targetPosition, popup->size()));
        popup->setWindowOpacity(1.0);
    });
    popup->raise();

    // Destroying the popup also cancels its timeout callback.
    QTimer::singleShot(message.durationMs, Qt::PreciseTimer, popup, [this, popup]() {
        QScreen *screen = popup->targetScreen.data();
        m_messages.removeOne(popup);
        popup->hide();
        popup->deleteLater();
        if (screen)
            arrangeMessages(screen);
        processQueue();
    });
}

void ToastWidget::arrangeMessages(QScreen *screen)
{
    // Oldest is closest to the taskbar; new messages appear above the stack.
    const QRect area = screen->availableGeometry();
    const int margin = PopupMargin;
    const int spacing = PopupSpacing;
    int bottom = area.bottom() + 1 - margin;
    for (int i = 0; i < m_messages.size(); ++i) {
        ToastPopup *popup = m_messages.at(i);
        if (popup->targetScreen != screen)
            continue;
        // Use logical design dimensions; native DPI events can temporarily resize
        // a newly created widget before its first show has settled.
        popup->moveTo(QPoint(qMax(area.left(), area.right() + 1 - PopupWidth - margin),
                             bottom - PopupHeight));
        bottom -= PopupHeight + spacing;
    }
}
