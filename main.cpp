#include "toastwidget.h"

#include <QApplication>
#include <QFont>
#include <QGuiApplication>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QHBoxLayout>
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
    window.resize(520, 500);
    window.setMinimumSize(480, 480);
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
        QString::fromUtf8("点击不同级别，观察右下角提示的颜色和叠放顺序"), &window);
    auto *hint = new QLabel(QString::fromUtf8("超出数量按顺序排队 · 显示后计时 3 秒"), &window);

    auto *layout = new QVBoxLayout(&window);
    layout->setContentsMargins(32, 28, 32, 28);
    layout->setSpacing(14);
    layout->addStretch();
    layout->addWidget(heading, 0, Qt::AlignHCenter);
    layout->addWidget(description, 0, Qt::AlignHCenter);
    layout->addSpacing(8);
    auto *toast = new ToastWidget(&window);
    auto *limitRow = new QHBoxLayout;
    auto *limitLabel = new QLabel(QString::fromUtf8("最多同时显示："), &window);
    auto *limitInput = new QSpinBox(&window);
    limitInput->setRange(1, 20);
    limitInput->setValue(toast->maxVisibleMessages());
    limitInput->setSuffix(QString::fromUtf8(" 条"));
    limitInput->setToolTip(QString::fromUtf8("调小后，已有提示会正常显示到期，再按新上限补入"));
    limitRow->addStretch();
    limitRow->addWidget(limitLabel);
    limitRow->addWidget(limitInput);
    limitRow->addStretch();
    layout->addLayout(limitRow);
    QObject::connect(limitInput, QOverload<int>::of(&QSpinBox::valueChanged),
                     toast, &ToastWidget::setMaxVisibleMessages);
    quint64 clickCount = 0;
    struct DemoLevel {
        ToastWidget::Level level;
        const char *name;
        const char *color;
        const char *hoverColor;
    };
    const DemoLevel levels[] = {
        {ToastWidget::Level::Info, "info", "#2563eb", "#1d4ed8"},
        {ToastWidget::Level::Warning, "warning", "#a16207", "#854d0e"},
        {ToastWidget::Level::Error, "error", "#dc2626", "#b91c1c"}
    };
    for (const DemoLevel &entry : levels) {
        const QString name = QString::fromLatin1(entry.name);
        auto *button = new QPushButton(
            QString::fromUtf8("%1（已点击 0 次）").arg(name), &window);
        button->setMinimumWidth(280);
        button->setCursor(Qt::PointingHandCursor);
        button->setStyleSheet(QStringLiteral(
            "QPushButton { background: %1; } QPushButton:hover { background: %2; }"
            "QPushButton:pressed { background: %2; }")
            .arg(QString::fromLatin1(entry.color), QString::fromLatin1(entry.hoverColor)));
        layout->addWidget(button, 0, Qt::AlignHCenter);
        const ToastWidget::Level level = entry.level;
        quint64 levelCount = 0;
        QObject::connect(button, &QPushButton::clicked, &window,
                         [&window, toast, button, &clickCount, name, level, levelCount]() mutable {
            ++clickCount;
            ++levelCount;
            button->setText(QString::fromUtf8("%1（已点击 %2 次）").arg(name).arg(levelCount));
            QScreen *screen = window.windowHandle()
                ? window.windowHandle()->screen() : QGuiApplication::primaryScreen();
            toast->showMessage(QString::fromUtf8("第 %1 次提示 · %2").arg(clickCount).arg(name),
                               level, 3000, screen);
        });
    }
    layout->addWidget(hint, 0, Qt::AlignHCenter);
    layout->addStretch();

    window.show();
    return app.exec();
}
