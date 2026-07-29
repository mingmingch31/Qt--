#include "databasemanager.h" //#include(包含指令):大白话叫拿工具。车间要开工，得先把工具拿进屋
//双引号""：代表这是"我们自己画的图纸"。所以把刚才的databasemanager.h拿进来，照着图纸施工。
#include <QDebug>//尖括号<>:代表是"Qt官方提供的标准工具".
//debug:厂里的大喇叭。用来在屏幕下方的黑框里向你汇报工作（代替c语言的printf）.
#include <QSqlError>//QSqlError:厂里的法医。如果数据库出错了，它能告诉你具体的死因。
#include <QSqlQuery>//QSqlQuery:厂里的SQL专员。专门负责执行增删改查的指令。
#include <QStringList>

//1.构造函数：部门开业第一天自动执行,DatabaseManager::(作用域解析符)：这两个冒号极其关键！在公共
//大车间（.cpp）里，电脑不知道你在给谁干活。这两冒号就是在宣告：
//“后面这个函数，是属于我DatabaseManager这个部门的！”
DatabaseManager::DatabaseManager(QObject *parent)
    //DatabaseManager(QObject *parent):名字和部门名字一模一样，且没有返回值（连void都没有）
    //这就是构造函数（开业剪彩仪式）.

    : QObject{parent}// : QObject(parent):冒号在这里是移交的意思，大白话：老爹（parent）,
    //我拿到你的指针了，我现在把你移交给我的干爹（QObject）去登记，以后你死的时候，干爹会自动把我一起销毁
    //这就是Qt绝不漏内存的终极秘诀。

{
    initDatabase(); //剪彩完毕的第一秒，立刻关起门来，偷偷执行内部的建库函数。刚开业，立刻关起门来在内部建好保险柜

}

//2打造绝密档案室(核心逻辑)
//void:大白话叫“白干”。干完活不往外吐任何数据，只在内部默默执行。
void DatabaseManager::initDatabase()
{
    //第一步，向系统申请一把专门开SQLite这种轻量级锁的钥匙
    m_db=QSqlDatabase::addDatabase("QSQLITE");

    //第二步，给你要建的保险柜（文件）起个名字
    m_db.setDatabaseName("LarkData2.db");

    //第三步，尝试打开保险柜（类似c语言的if(fp==NULL)）
    if (!m_db.open()){
        qDebug() << "糟糕，数据库打不开！原因：" << m_db.lastError().text();
        return;//遇到致命错误，立刻急刹车，退出函数,工业级防线（急刹车）！们都没打开，
        //下面绝对不能干活了。这个词一出，函数立刻强行结束，把Bug掐死在摇篮里
    }

    qDebug() << "====== 幽灵退散 v2.0 ======";

//大白话，雇佣一个名叫query的SQL专员，并且明确告诉他：“你以后只负责在这个m_db(LarkData.db)房间里干活！"
QSqlQuery query(m_db);

//第五步，文员写了一份建表申请表，表名叫 sensor_logs,有id和message两列
QString sql = "CREATE TABLE IF NOT EXISTS sensor_logs( "
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
              "message TEXT)";

//第六步，让文员去执行exec这份申请书
if (query.exec(sql)){
    qDebug() << "太棒了！保险柜和格子准备就绪，可以存数据了!";
    } else {
    qDebug() << "建格子失败：" << query.lastError().text();
    }
}

//留给外界的办事窗口
void DatabaseManager::saveRecord(const QString &message)

{
    QSqlQuery query(m_db);//雇佣SQL专员，并告诉他在m_db这个保险柜干活

    query.prepare("INSERT INTO sensor_logs (message) VALUES (:msg)");

    query.bindValue(":msg",message);

    if(query.exec()){
        qDebug() << "入库成功！存入的内容是：" << message;
    }else {
        qDebug() << "入库失败，死因：" << query.lastError().text();
    }

}

QStringList DatabaseManager::getAllRecords()
{
    QStringList recordList;//拿来一个空的集装箱准备装货
    QSqlQuery query(m_db);//必须先雇佣SQL专员，并给他分配保险柜m_db!
    if (query.exec("SELECT message FROM sensor_logs")){
        while (query.next()){
            recordList.append(query.value(0).toString());
        }
    }else {
        qDebug() << "调取档案失败" << query.lastError().text();
    }
    return recordList;

}