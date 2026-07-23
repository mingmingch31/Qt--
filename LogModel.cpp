#include "LogModel.h"   // 引入对应的头文件，就像是这篇代码的“说明书”
#include <QDateTime>    // 引入 Qt 的日期时间处理类，我们要用它来给日志打上精确的时间戳

// 构造函数：当 LogModel 这个类被实例化（创建）时，会自动执行这里的代码
LogModel::LogModel(QObject *parent)
    : QObject(parent) // 调用父类 QObject 的构造函数，建立 Qt 的对象树（可以防止内存泄漏）
{
    // 软件刚启动、这个模型刚建立时，自动往日志里追加第一条初始化信息
    appendLog("系统启动成功，核心通讯与日志模块已连接...");
}

// 这是一个只读函数 (加了 const 表示不会修改类内部的数据)
// 它的作用是向外界（特别是前端 QML）交出当前的全部日志文本
QString LogModel::logText() const
{
    return m_logText; // 直接返回私有变量 m_logText 的内容
}

// 【核心功能函数】：向日志板里追加新信息。
// 因为你在 .h 文件里给它加了 Q_INVOKABLE 宏，所以 QML 和其他 C++ 文件都能直接调用它
void LogModel::appendLog(const QString &message)
{
    // 1. 获取当前电脑的系统时间，并把它格式化为 "年-月-日 时:分:秒" 的标准文本形式
    QString timeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    // 2. 拼装新日志文本：
    // %1 会被替换成上面获取的 timeStr (时间)
    // %2 会被替换成外界传进来的 message (具体日志内容)
    // \n 是换行符，保证每条日志独占一行
    // += 运算符表示“追加”，相当于 m_logText = m_logText + 新内容
    m_logText += QString("[%1] %2\n").arg(timeStr, message);

    // 3. 【点睛之笔】发射信号！通知前端 QML：“我的文本内容更新啦，你快点刷新屏幕！”
    emit logTextChanged();
}