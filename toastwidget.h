#ifndef TOASTWIDGET_H
#define TOASTWIDGET_H

#include <QWidget>

class QLabel;
class QScreen;
class QTimer;

// A reusable desktop notification. Repeated calls restart the countdown.
class ToastWidget : public QWidget
{
public:
    explicit ToastWidget(QWidget *parent = nullptr);
    void showMessage(const QString &message, int durationMs = 3000,
                     QScreen *screen = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QLabel *m_label;
    QTimer *m_timer;
};

#endif // TOASTWIDGET_H
