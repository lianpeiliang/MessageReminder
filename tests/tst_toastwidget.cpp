#include "../toastwidget.h"
#include <QApplication>
#include <QLabel>
#include <QScreen>
#include <QWindow>
#include <QtTest>

class ToastWidgetTests : public QObject
{
    Q_OBJECT
private slots:
    void popupGeometryAcrossScreens()
    {
        QWidget owner;
        owner.setAttribute(Qt::WA_ShowWithoutActivating);
        owner.resize(400, 260);
        ToastWidget toast(&owner);
        auto screens = QGuiApplication::screens();
        QVERIFY(!screens.isEmpty());
        // Return to the first monitor too, exercising both DPI transition directions.
        if (screens.size() > 1)
            screens.append(screens.first());
        for (QScreen *screen : screens) {
            owner.move(screen->availableGeometry().center() - QPoint(200, 130));
            owner.show();
            QTest::qWait(100);
            for (int i = 0; i < 3; ++i)
                toast.showMessage(QString::number(i), ToastWidget::Level::Info, 700, screen);
            QTest::qWait(250);
            int count = 0;
            for (QWidget *popup : toast.findChildren<QWidget *>()) {
                if (!popup->isWindow() || !popup->isVisible())
                    continue;
                auto *label = popup->findChild<QLabel *>("toastMessage");
                QVERIFY(label);
                const int index = label->text().toInt();
                const QRect area = screen->availableGeometry();
                QCOMPARE(popup->windowHandle()->screen(), screen);
                QCOMPARE(popup->size(), QSize(280, 84));
                QCOMPARE(popup->pos(), QPoint(area.right() + 1 - 280 - 20,
                                             area.bottom() + 1 - 20 - 84 - index * 96));
                QVERIFY(area.contains(popup->frameGeometry()));
                QCOMPARE(popup->windowOpacity(), 1.0);
                ++count;
            }
            QCOMPARE(count, 3);
            QTest::qWait(550);
        }
    }
};

int main(int argc, char **argv)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    ToastWidgetTests tests;
    return QTest::qExec(&tests, argc, argv);
}
#include "tst_toastwidget.moc"
