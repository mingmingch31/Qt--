#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext> //拿到QML前台大厅的总机电话线，拿来“通信总机”的工具箱
#include "databasemanager.h" //把我们画的图纸拿过来

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    //核心桥梁
    DatabaseManager myManager;//在主大厅把部门开起来
    //拉一根线，名字叫"myDb",把它接在myManager这个部门上
    //以后前台大厅只要对着"myDb"喊话，就等于直接给myManager下达命令！

    //setContextProperty("myDb",&myManager);,这是工业界最粗暴、管用的前后端大同方式
    engine.rootContext()->setContextProperty("myDb",&myManager);
    //setContextProperty (单数结尾)：专门用来接一根专线。你需要给它 2 个参数（一个暗号名字 "myDb"，一个对象的内存地址 &myManager）。这就是我们想要的！
    //"myDb":我们给前台定下的暗号
    //&myManager:车间的真实内存地址（指针）
    //大白话：厂长(engine.rootContext())宣布以后前台无论是谁喊“myDb",
    //电话全部给我转接到地下室的myManager车间去！

    engine.loadFromModule("LarkTraining","Main");


    return app.exec();
}




//第二步：架设跨界专线电话 (修改 main.cpp)
//后台的车间干活能力有了，但前台 QML 大厅根本不知道有这个车间的存在。我们需要在程序的入口处，拉一根专线电话。