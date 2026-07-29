#ifndef DEVICESIMULATOR_H
#define DEVICESIMULATOR_H //这叫“防卫宏”。就像夜店门口的保安。假如好几个文件都
//#include 了这个头文件，保安会查身份证：“你之前进来过没（#ifndef 意思是“如果没有定义过”）？
//没进来过？好，进去吧，并且给你盖个章（#define 定义它）。” 如果下次别的代码又想复制一遍，
//保安看到有章，直接拦截。目的：防止代码被重复复制导致编译报错。

#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QVector>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QThread>

// =========================================================
// 【架构升级】DataWorkerThread：工业级硬件轮询采集线程
// 抛弃 QTimer，直接重写 run() 死循环，做到 100% 免疫事件循环卡死
// =========================================================
class DataWorkerThread : public QThread //大白话是“我的这个 DataWorkerThread，
                             //其实也就是一个 QThread（线程）”。它继承了爹（QThread）所有的超能力
{
    Q_OBJECT
public:
    //这是构造函数（对象刚出生时执行的函数）。
    //*parent：星号 * 代表“指针”。指针就像是一张藏宝图，它本身不是宝藏，而是写着“父对象在内存的某个地址”
    //= nullptr：默认这张藏宝图是空的（地址为0）
    //目的： 在 Qt 里，只要子对象认了爹（认了 parent），当爹被销毁时，会自动把所有的孩子也销毁，这就绝对不会发生内存泄漏！
    explicit DataWorkerThread(QObject *parent = nullptr) : QThread(parent) {}
    ~DataWorkerThread() {
        m_stop = true; // 析构时拉下电闸
        wait();        // 死等线程安全退出
    }
    //~DataWorkerThread()：
    //波浪号 ~ 代表“析构函数”。对象死亡、从内存中被抹除之前，自动执行的遗言。
    //wait();：大白话是“在这个线程死透之前，让主程序等一下，确保地下室的机器完全停转了再埋土，防止数据撕裂。”


    // 接收主线程的暂停指令
    void setPaused(bool isPaused) {
        m_isPaused = isPaused;
    }

signals:
    // 将采集到的数据发射回主线程
    //signals: void sensorDataReady(...)：
    //信号声明区。 这里只是在菜单上写上“本店有这道菜”，不需要写具体怎么炒
    //（不需要写 {} 函数体）。这是供 emit 发射用的
    void sensorDataReady(double temp, double volt, double curr);


    //protected：意思是这个函数只有自己和家里人能用，外人调不了。
protected:

    //override：重写标记。因为爹（QThread）本身就有一个默认的 run 函数。
    //加上这个词，大白话就是告诉电脑：“我要把祖传的那个函数扔掉，按我自己的意思重新写一个 run！”
    // 【核心】线程启动后，唯一执行的死循环函数
    void run() override;

private:
    bool m_stop = false;       // 控制线程生死的总闸
    bool m_isPaused = false;   // 控制是否暂停采集的开关
};

// =========================================================
// DeviceSimulator：业务主控制台
// =========================================================
class DeviceSimulator : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(double voltage READ voltage NOTIFY voltageChanged)
    Q_PROPERTY(double current READ current NOTIFY currentChanged)
    Q_PROPERTY(double temperature READ temperature NOTIFY temperatureChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString alarmLog READ alarmLog NOTIFY alarmLogChanged)

public:
    explicit DeviceSimulator(QObject *parent = nullptr);
    ~DeviceSimulator();

    Q_INVOKABLE void setTempThreshold(double threshold);
    Q_INVOKABLE void exportAlarmLog(const QString &filePath);
    Q_INVOKABLE void toggleSimulation();
    Q_INVOKABLE void clearAlarmLog();
    Q_INVOKABLE void loadHistoryFromDatabase();

    double voltage() const { return m_voltage; }
    double current() const { return m_current; }
    double temperature() const { return m_temperature; }
    QString status() const { return m_status; }
    QString alarmLog() const { return m_alarmLog; }

signals:
    void voltageChanged();
    void currentChanged();
    void temperatureChanged();
    void statusChanged();
    void alarmLogChanged();
    void dataUpdated();
    void newSystemLog(QString msg);
    void historyLoaded(QString historyText);

private slots:
    void onNewConnection();
    void onClientDisconnected();
    // 接收子线程抛上来的数据
    void onSensorDataReceived(double t, double v, double c);

private:
    double m_voltage = 220.0;
    double m_current = 10.0;
    double m_temperature = 25.0;
    QString m_status = "运行中";
    QString m_alarmLog;
    double m_tempThreshold = 30.0;
    bool m_isSimulationRunning = true;

    QWebSocketServer *m_webSocketServer;
    QList<QWebSocket*> m_clients;
    QSqlDatabase m_db;

    // 【修改为新的自定义线程类】
    DataWorkerThread *m_workerThread;

    void initDatabase();
};

#endif // DEVICESIMULATOR_H