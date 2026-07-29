#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);

    Q_INVOKABLE void saveRecord(const QString &message);
    Q_INVOKABLE QStringList getAllRecords();
    //底层逻辑提示：QStringList就是一个能装下无数条字符串的“集装箱”。

private:
    QSqlDatabase m_db;
    void initDatabase();

signals:

//在当前这个微型战役里，我们的数据库管理员还不需要向外界呐喊，
//只要有对外接客的 public 窗口和自己干活的 private 机密室就足够了

};

#endif // DATABASEMANAGER_H

//现在，图纸上的每一个标点符号、参数和修饰词，你都已经彻底啃透了，没有任何知识死角。
//接下来我们要推开机密室的门，去 .cpp 文件里，
//把 initDatabase()这个方法具体怎么连接 SQLite 数据库的代码写出来。
