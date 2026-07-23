#include <QApplication>//相当于你对系统说：“去工具箱里把叫 QApplication 的那个扳手给我拿过来，我等下要用。”
#include <QQmlApplicationEngine>
#include <QQuickStyle>

int main(int argc, char *argv[])//(工厂的大门)
{
    QApplication app(argc, argv);
    //实例化一个叫 app 的厂长。他接管了你电脑的鼠标点击、窗口拖拽等所有事情
    //这是大门。argc 和 argv 记录了你是不是通过命令行加了后缀来启动这个软件的
    //（比如 Lark.exe -fullscreen，这个 -fullscreen 就会被装进这两个变量里）
    //在内存的栈区，创建一个名为 app 的 QApplication 对象，并把操作系统的参数传给它。它接管了你的鼠标、键盘和窗口。
    QQuickStyle::setStyle("Material");
    QQmlApplicationEngine engine;//实例化一台“幻灯片放映机”，专门用来把你写的 QML 前端代码投影到屏幕上。

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);

    engine.loadFromModule("LarkWorkstation", "Main");//告诉放映机：“去把 Main.qml 这个幻灯片塞进去，准备播放！”
    return app.exec();//这是最核心的一句。exec() 是一个死循环。如果没有这句话，
    //程序跑完上面几行代码就直接结束关闭了。有了这句话，软件就会一直卡在这里，永远等待你点鼠标，
    //直到你关掉软件，它才会 return (返回) 给操作系统。
}