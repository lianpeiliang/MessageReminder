#include "toastwidget.h"

#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QScreen>
#include <QTimer>

ToastWidget::ToastWidget(QWidget *parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint
                      | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus),
      m_label(new QLabel(this)),
      m_timer(new QTimer(this))
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
    m_label->setStyleSheet("color: #f8fafc; font-size: 16px;");
    m_label->setTextFormat(Qt::PlainText);
    m_label->setWordWrap(true);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(22, 18, 22, 18);
    layout->setSpacing(14);
    layout->addWidget(icon);
    layout->addWidget(m_label, 1);

    m_timer->setSingleShot(true);
    m_timer->setTimerType(Qt::PreciseTimer);
    connect(m_timer, &QTimer::timeout, this, &QWidget::hide);
}

void ToastWidget::showMessage(const QString &message, int durationMs, QScreen *screen)
{
    if (!screen)
        screen = QGuiApplication::primaryScreen();
    if (!screen)
        return;

    m_label->setText(message);

    // availableGeometry excludes the taskbar and supports non-zero screen origins.
    const QRect area = screen->availableGeometry();
    const int margin = 20;
    move(qMax(area.left(), area.right() + 1 - width() - margin),
         qMax(area.top(), area.bottom() + 1 - height() - margin));

    show();
    raise();
    // Restart the same timer, so an earlier click cannot hide a newer message.
    m_timer->start(qMax(1, durationMs));
}

void ToastWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor("#3e4c60"), 1));
    painter.setBrush(QColor("#1e293b"));
    painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 12, 12);
}
