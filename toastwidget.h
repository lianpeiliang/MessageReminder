#ifndef TOASTWIDGET_H
#define TOASTWIDGET_H

#include <QWidget>
#include <QList>

class QScreen;
class ToastPopup;

// Each call adds an independently timed notification to the desktop stack.
class ToastWidget : public QWidget
{
public:
    explicit ToastWidget(QWidget *parent = nullptr);
    void showMessage(const QString &message, int durationMs = 3000,
                     QScreen *screen = nullptr);

private:
    void arrangeMessages(QScreen *screen);
    QList<ToastPopup *> m_messages;
};

#endif // TOASTWIDGET_H
