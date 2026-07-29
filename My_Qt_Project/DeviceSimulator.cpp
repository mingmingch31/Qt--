#include "DeviceSimulator.h"
//#include "..."：带双引号，大白话：“去我自己的工程文件夹里，把这张说明书拿过来。”
//地下室与后台车间,这个文件是整个软件的“发动机”，所有脏活累活都在这里干
//引入图纸（头文件）
#include <QRandomGenerator>
//#include <...>：带尖括号，大白话：“去系统底层的 Qt 工具箱里，
//把名叫 QRandomGenerator（随机数摇号机）、QDebug（控制台打印机）等公用工具拿过来。”

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

// -----------------------------------------------------
// DataWorkerThread 实现：纯正的底层轮询引擎
// -----------------------------------------------------
//地下室工人的死循环作业 (DataWorkerThread::run)
//void：这个函数干完活不返回任何东西（不交报告，只默默干活）。
void DataWorkerThread::run()
//DataWorkerThread::：双冒号 :: 叫“作用域解析符”。大白话：“前面这个 run 函数，
//是属于 DataWorkerThread 这个工人的，不是别人的。”
//()：里面没有参数，说明工人干活不需要别人额外给他递东西。
{    //{ }：这个大括号就是地下室的物理墙壁。里面定义的变量 internalTemp，
    //除了这个工人，外面主车间的厂长（主线程）绝对看不见也摸不着，这在多线程里叫“线程安全”。
    // 线程内部的私有寄存器（绝对安全，主线程摸不到）
    double internalTemp = 25.0;
    double internalVolt = 220.0;
    double internalCurr = 10.0;

    // while 死循环：只要软件没关，这个线程就一直在后台跑
    while (!m_stop) {
        if (!m_isPaused) {
            // 生成随机波动
            //->：指针操作符（极其重要）。大白话：“顺着 global() 给的地图，找到摇号机，
            //然后按一下它身上的 bounded（限制范围）按钮，摇出一个 -5 到 5 之间的数字。”
            internalVolt += (QRandomGenerator::global()->bounded(-5, 6)) * 0.1;
            internalCurr += (QRandomGenerator::global()->bounded(-5, 6)) * 0.1;

            double tempDrift = 0;
            if (internalTemp > 35.0) { tempDrift = -0.3; }
            else if (internalTemp < 25.0) { tempDrift = 0.3; }

            internalTemp += (QRandomGenerator::global()->bounded(-3, 6)) * 0.1 + tempDrift;

            //qBound：Qt 自带的极值保护钳。大白话：“把温度死死卡在 20 和 40 之间，
            //超过 40 就强行压成 40，防止把图表画爆。”
            internalVolt = qBound(200.0, internalVolt, 240.0);
            internalCurr = qBound(5.0, internalCurr, 15.0);
            internalTemp = qBound(20.0, internalTemp, 40.0);

            // 将算好的数据打包，发射给主线程！emit：Qt 最核心的魔法词（发射）。
            //大白话：“工人干完一票，拿个大喇叭对着天花板上的通风管大喊一声 sensorDataReady，
            //并且把算好的三个数字装在包裹里顺着管道扔上去！”
            emit sensorDataReady(internalTemp, internalVolt, internalCurr);
        }

        // 【关键】强行让线程休眠 1000 毫秒 (1秒)。代替了脆弱的 QTimer
        //QThread::msleep(1000)：让当前这个地下室的 CPU 强制睡 1000 毫秒（1秒）。
        //逻辑走向重点： 这里睡 1 秒，完全不会影响前台大厅的动画，因为他们不在一个空间（不在一个线程）！
        QThread::msleep(1000);
    }
}

// -----------------------------------------------------
// DeviceSimulator 实现：主控台
// -----------------------------------------------------
//厂长办公室的布置 (DeviceSimulator 构造函数)
DeviceSimulator::DeviceSimulator(QObject *parent) : QObject(parent)
{
    initDatabase(); //厂长（类）一出生，第一件事先调用内部函数 initDatabase() 去买个保险柜（建数据库）。

    // 1. 直接实例化我们自己写的线程类
    m_workerThread = new DataWorkerThread(this);
    //new：在电脑内存的堆区（一片巨大的空地）里，划出一块地，造一个新的工人。
    //m_workerThread：这是一个指针（藏宝图），指着刚才造出来的那个工人。
    //(this)：把厂长自己作为爹传给工人。大白话：“以后我（厂长）被销毁的时候，
    //系统会自动顺藤摸瓜把你也干掉，不会成为游魂（内存泄漏）。”

    // 2. 建立跨线程桥梁：接收数据,connect(谁发信号, 发什么信号, 谁接收, 怎么处理)：接水管！
    connect(m_workerThread, &DataWorkerThread::sensorDataReady, this, &DeviceSimulator::onSensorDataReceived);
    //& 符号：在这里叫“取地址”。把函数和对象的具体内存坐标交出来，让 Qt 底层用一根管
    //子死死焊住。地下室工人一喊 sensorDataReady，厂长就会立刻执行onSensorDataReceived 拆包裹

    // 3. 启动引擎！它会自动调用 run() 进入死循环
    //大白话：“给地下室通电！”一执行这句，工人就会立刻开始跑上面那个 run() 死循环。
    m_workerThread->start();

    // 启动网络
    m_webSocketServer = new QWebSocketServer("Lark Server", QWebSocketServer::NonSecureMode, this);
    if (m_webSocketServer->listen(QHostAddress::Any, 8080)) {
        emit newSystemLog("网络模块：WebSocket 服务端已启动(端口8080)...");
        connect(m_webSocketServer, &QWebSocketServer::newConnection, this, &DeviceSimulator::onNewConnection);
    }
}

DeviceSimulator::~DeviceSimulator()
{
    // 软件关闭时，通知线程退出死循环并销毁
    if (m_workerThread->isRunning()) {
        m_workerThread->deleteLater();
    }
    m_webSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
    if (m_db.isOpen()) { m_db.close(); }
}

void DeviceSimulator::initDatabase()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName("lark_workstation.db");
    if (!m_db.open()) {
        emit newSystemLog("严重错误：本地数据库初始化失败！");
        return;
    }
    QSqlQuery query(m_db);
    query.exec("CREATE TABLE IF NOT EXISTS alarm_records ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "time_str TEXT, "
               "temp_val REAL)");
}

//厂长接收包裹与记账 (onSensorDataReceived)
//厂长从水管里收到包裹，把里面的 t（温度）拿出来，写在自己办公室的黑板 m_temperature 上。
void DeviceSimulator::onSensorDataReceived(double t, double v, double c)
{
    m_temperature = t;
    m_voltage = v;
    m_current = c;

    if (m_temperature > m_tempThreshold) {
        m_status = "故障(超温)";
        QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

        m_alarmLog += QString("[%1] 温度超标（%2℃）\n").arg(currentTime).arg(m_temperature, 0, 'f', 1);
        emit alarmLogChanged(); // 触发 UI 更新

        QSqlQuery query(m_db);
        //query.prepare：准备写档案。
        query.prepare("INSERT INTO alarm_records (time_str, temp_val) VALUES (:time, :temp)");
        //:time 和 :temp：这叫“占位符”。就像填表时留下的空格。
        //bindValue：把真实的时间和温度盖章填进空格里。
        query.bindValue(":time", currentTime);
        query.bindValue(":temp", m_temperature);
        //逻辑走向重点： 为什么不直接拼凑字符串？因为如果是外部输入，
        //可能会有人输入 ; DROP TABLE; 这种恶意代码把数据库毁了。用占位符是防止“SQL 注入”的铁律！

        query.exec();

    } else {
        m_status = "运行中";
    }

    emit statusChanged();

    if (!m_clients.isEmpty()) {
        QJsonObject jsonObj;
        jsonObj["temperature"] = m_temperature;
        jsonObj["voltage"] = m_voltage;
        QJsonDocument doc(jsonObj);
        QString jsonString = doc.toJson(QJsonDocument::Compact);
        for (QWebSocket *client : m_clients) {
            client->sendTextMessage(jsonString);
        }
    }

    emit voltageChanged();
    emit currentChanged();
    emit temperatureChanged();

    // 通知前端：数据搞定了，去画图吧！
    //厂长把一切登记妥当后，按下面前的红色广播按钮，发射 dataUpdated 信号给前台大厅：“数据更新了，快去画图！”
    emit dataUpdated();
}

void DeviceSimulator::loadHistoryFromDatabase()
{
    QSqlQuery query(m_db);
    query.exec("SELECT time_str, temp_val FROM alarm_records ORDER BY id DESC LIMIT 20");
    QString resultStr = "--- 数据库最近20条报警记录 ---\n";
    while (query.next()) {
        QString timeStr = query.value(0).toString();
        double tempVal = query.value(1).toDouble();
        resultStr += QString("[%1] 历史超温: %2℃\n").arg(timeStr).arg(tempVal, 0, 'f', 1);
    }
    resultStr += "--------------------------------\n";
    emit historyLoaded(resultStr);
}

void DeviceSimulator::toggleSimulation() {
    m_isSimulationRunning = !m_isSimulationRunning;

    if (!m_isSimulationRunning) {
        m_status = "已暂停";
        emit newSystemLog("警告：操作员已手动暂停监控，断开传感层指令");
    } else {
        m_status = "运行中";
        emit newSystemLog("提示：系统恢复运行，重连传感层");
    }

    emit statusChanged();

    // 直接通知底层线程挂起
    if (m_workerThread) {
        m_workerThread->setPaused(!m_isSimulationRunning);
    }
}

void DeviceSimulator::clearAlarmLog() {
    m_alarmLog.clear();
    emit alarmLogChanged();
}
void DeviceSimulator::onNewConnection() {}
void DeviceSimulator::onClientDisconnected() {}
void DeviceSimulator::setTempThreshold(double threshold) {}
void DeviceSimulator::exportAlarmLog(const QString &filePath) {}