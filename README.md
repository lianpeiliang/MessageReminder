# MessageReminder

使用 **Qt Widgets（QWidget）+ qmake + C++11** 的桌面提示示例。

点击“显示提示（已点击 0 次）”按钮会累加点击次数，并在主窗口所在屏幕右下角、任务栏上方显示“第 N 次提示”。按钮同步显示累计次数，方便观察提示的叠放顺序。每次调用都创建独立提示窗，最新的一条位于最下面，之前的提示平滑上移。

例如连续调用三次后，从上到下依次是第 1、2、3 条。每条提示从自身显示时开始独立计时，默认 3 秒后自动消失并释放；后续调用不会重置之前提示的计时。中间或底部提示消失时，其余提示会自动移动补齐空位。提示窗无标题栏、置顶、不抢占键盘焦点，也不会单独出现在任务栏中。

## 运行

使用 Qt Creator 打开 `MessageReminder.pro`，选择 Qt Widgets 桌面套件，构建并运行。当前代码已使用本机 Qt 5.12.12 / MinGW 7.3 64 位套件编译验证。

也可在配置好 Qt 和编译器环境的 PowerShell 中构建：

```powershell
mkdir build
cd build
qmake ../MessageReminder.pro
mingw32-make release
./release/MessageReminder.exe
```

MSVC 套件在其开发者命令行中使用 `nmake release`。运行时需要相应 Qt 运行库；分发程序可使用套件自带的 `windeployqt` 部署。

## 接入自己的按钮

把 `toastwidget.h` 和 `toastwidget.cpp` 加入 qmake 工程，并确保 `.pro` 含有 `QT += widgets`。在所属窗口中创建并复用一个实例，各个按钮共用它就能统一叠放提示：

```cpp
auto *toast = new ToastWidget(this);
connect(button, &QPushButton::clicked, this, [toast]() {
    QApplication::clipboard()->setText(QStringLiteral("#2563EB"));
    toast->showMessage(QString::fromUtf8("已复制十六进制 #2563EB"), 3000);
});
```

复制示例需要包含 `<QApplication>` 和 `<QClipboard>`。

`showMessage(message, durationMs, screen)` 每次新增一条提示。第二个参数为该条提示的显示毫秒数；第三个参数可指定 `QScreen *`，省略时使用主屏幕。主示例传入主窗口所在屏幕，不同屏幕的提示分别叠放。

使用 `QScreen::availableGeometry()` 定位、独立单次 `QTimer` 控制生命周期、`QPropertyAnimation` 平滑移动。当前示例适合少量短暂提示；大量同时调用时，堆叠高度可能超出屏幕。
