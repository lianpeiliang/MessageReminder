#ifndef TOASTWIDGET_H
#define TOASTWIDGET_H

#include <QWidget>
#include <QList>
#include <QPointer>
#include <QQueue>

class QScreen;
class ToastPopup;

// Calls are queued in order; each notification is timed only while visible.
class ToastWidget : public QWidget
{
public:
    enum class Level { Info, Warning, Error };

    explicit ToastWidget(QWidget *parent = nullptr);
    void setMaxVisibleMessages(int count);
    int maxVisibleMessages() const { return m_maxVisibleMessages; }
    void showMessage(const QString &message, int durationMs = 3000,
                     QScreen *screen = nullptr);
    void showMessage(const QString &message, Level level, int durationMs = 3000,
                     QScreen *screen = nullptr);

private:
    struct PendingMessage {
        QString text;
        Level level;
        int durationMs;
        QPointer<QScreen> screen;
    };
    void processQueue();
    void displayMessage(const PendingMessage &message, QScreen *screen);
    void arrangeMessages(QScreen *screen);
    int m_maxVisibleMessages = 3;
    QQueue<PendingMessage> m_pendingMessages;
    QList<ToastPopup *> m_messages;
};

#endif // TOASTWIDGET_H
