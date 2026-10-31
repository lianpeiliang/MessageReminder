#include "toastwidget.h"

#include <QApplication>
#include <QFont>
#include <QGuiApplication>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWindow>

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication app(argc, argv);
    app.setFont(QFont(QStringLiteral("Microsoft YaHei UI"), 10));

    QWidget window;
    window.setWindowTitle(QString::fromUtf8("MessageReminder · 消息提示示例"));
    window.setObjectName(QStringLiteral("demoWindow"));
    window.resize(480, 280);
    window.setMinimumSize(440, 260);
    window.setStyleSheet(
        "QWidget#demoWindow { background: #f5f7fb; }"
        "QLabel { color: #64748b; background: transparent; }"
        "QLabel#heading { color: #172033; font-size: 24px; font-weight: 600; }"
        "QPushButton { color: white; background: #2563eb; border: none;"
        "border-radius: 8px; padding: 12px 30px; font-size: 15px; }"
        "QPushButton:hover { background: #1d4ed8; }"
        "QPushButton:pressed { background: #1e40af; }"
        "QPushButton:focus { border: 2px solid #93c5fd; }");

    auto *heading = new QLabel(QString::fromUtf8("消息提示示例"), &window);
    heading->setObjectName(QStringLiteral("heading"));
    auto *description = new QLabel(
        QString::fromUtf8("连续点击，观察右下角提示的编号和叠放顺序"), &window);
    auto *button = new QPushButton(QString::fromUtf8("显示提示（已点击 0 次）"), &window);
    button->setCursor(Qt::PointingHandCursor);
    auto *hint = new QLabel(QString::fromUtf8("连续点击可叠加提示 · 每条提示 3 秒后自动消失"), &window);

    auto *layout = new QVBoxLayout(&window);
    layout->setContentsMargins(32, 28, 32, 28);
    layout->setSpacing(14);
    layout->addStretch();
    layout->addWidget(heading, 0, Qt::AlignHCenter);
    layout->addWidget(description, 0, Qt::AlignHCenter);
    layout->addSpacing(8);
    layout->addWidget(button, 0, Qt::AlignHCenter);
    layout->addWidget(hint, 0, Qt::AlignHCenter);
    layout->addStretch();

    auto *toast = new ToastWidget(&window);
    quint64 clickCount = 0;
    QObject::connect(button, &QPushButton::clicked, &window, [&window, toast, button, &clickCount]() {
        ++clickCount;
        button->setText(QString::fromUtf8("显示提示（已点击 %1 次）").arg(clickCount));
        QScreen *screen = window.windowHandle()
            ? window.windowHandle()->screen() : QGuiApplication::primaryScreen();
        toast->showMessage(QString::fromUtf8("第 %1 次提示").arg(clickCount), 3000, screen);
    });

    window.show();
    return app.exec();
}
