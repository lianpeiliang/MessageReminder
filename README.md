# MessageReminder

## 演示视频

### 软件功能演示

https://github.com/user-attachments/assets/3cfba22c-1252-4bbc-b70f-38ee8b5d1a5b


使用 **Qt Widgets（QWidget）+ qmake + C++11** 的桌面提示示例。

示例提供 `info`、`warning`、`error` 三个按钮，分别使用蓝色信息、黄色警告、红色错误提示，并搭配不同图标。每个按钮独立累计点击次数；右下角显示“第 N 次提示 · 级别”，其中 N 是三个按钮共用的调用序号，方便观察混合叠放顺序。最早的一条位于最下面，后续提示依次出现在上方；连续显示第 1、2、3 条时，从上到下为 3、2、1。新增提示不会把已有提示向上顶，提示消失后，上方提示平滑下移补位。

默认最多同时显示 10 条（屏幕可用高度不足时会进一步限制），超过上限的消息按调用顺序进入先进先出队列。例如屏幕能容纳 10 条时，连续调用十二次，先显示第 1～10 条，第 11、12 条等待；有空位时依次补入。每条提示从实际显示时开始独立计时，默认 4 秒后自动消失并释放，排队期间不计时；后续调用不会重置之前提示的计时。中间或底部提示消失时，其余提示会自动移动补齐空位。提示窗无标题栏、置顶、不抢占键盘焦点，也不会单独出现在任务栏中。

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
    toast->showMessage(QString::fromUtf8("已复制十六进制 #2563EB"));
});
```

复制示例需要包含 `<QApplication>` 和 `<QClipboard>`。

`showMessage(message, durationMs, screen)` 每次新增一条提示。第二个参数为该条提示的显示毫秒数；第三个参数可指定 `QScreen *`，省略时使用主屏幕。主示例传入主窗口所在屏幕，不同屏幕的提示分别叠放。

原有调用默认使用 `Info`。需要指定级别时，使用新增的重载，三种级别均独立计时、共用叠放顺序：

```cpp
toast->showMessage(QString::fromUtf8("复制成功"), ToastWidget::Level::Info);
toast->showMessage(QString::fromUtf8("请检查输入"), ToastWidget::Level::Warning);
toast->showMessage(QString::fromUtf8("操作失败"), ToastWidget::Level::Error, 5000);
```

用 `toast->setMaxVisibleMessages(m)` 设置同一实例的同时显示上限（跨屏幕合计），`maxVisibleMessages()` 读取配置值，小于 1 的值按 1 处理。演示界面也可直接调整上限。调大后立即补入等待消息；调小后已显示的消息继续显示到期，数量降到新上限以下后再补入，不提前关闭提示。每个屏幕还会根据可用高度限制数量，避免提示被顶出屏幕。所有级别共用先进先出队列，不按严重程度插队。

队列当前保留所有等待消息，持续高频调用会导致等待时间增加。所属实例销毁时，显示中的提示和等待队列一同释放。

使用 `QScreen::availableGeometry()` 定位、独立单次 `QTimer` 控制生命周期、`QPropertyAnimation` 平滑移动。

双屏缩放不同时，提示窗会先绑定目标屏幕并处理首次原生窗口定位，再显示内容，避免先在主屏出现后跨屏移动。

## 双屏回归验证

在配置好 Qt 和编译器环境的 PowerShell 中运行：

```powershell
mkdir build/tests
cd build/tests
qmake ../../tests/tests.pro
mingw32-make release
./release/toastwidget_tests.exe
```

测试会短暂显示窗口，在各块实际屏幕及返回第一块屏幕时验证提示的屏幕归属、尺寸、坐标和间距。建议使用不同缩放比例的双屏运行。
