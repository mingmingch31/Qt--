cmake_minimum_required(VERSION 3.16) #(工厂的施工建设图纸)

project(LarkWorkstation VERSION 3.0 LANGUAGES CXX)
#告诉包工头：
#“这个工程叫 LarkWorkstation，用的语言是 C++（CXX 是 C++ 的缩写）。”

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
#极其重要！ C++ 本身是个老古董，根本不懂什么是“信号”和“槽”。
#Qt 发明了 Q_OBJECT 这个魔法宏，而 AUTOMOC 就是告诉包工头：
#“编译之前，先派个机器人把所有带 Q_OBJECT 的文件扫描一遍，偷偷在背后写一堆底层的 C++ 代码，强行让它支持信号发射。”

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 确保引入了 Charts 模块 告诉包工头：“去系统的 Qt 安装目录里，
#把 Sql (数据库)、Charts (图表) 这些大件给我搬过来，我要打包进软件里。”
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Qml Quick QuickControls2 Charts Network WebSockets Sql)

qt_standard_project_setup(REQUIRES 6.5)
qt_add_executable(LarkWorkstation main.cpp)

qt_add_qml_module(LarkWorkstation
    URI "LarkWorkstation"
    VERSION 3.0
    QML_FILES Main.qml
    SOURCES DeviceSimulator.h DeviceSimulator.cpp LogModel.h LogModel.cpp
)

#把上面搬过来的大件，跟我们自己写的代码真正地“焊死”在一起，合成一个完整的 exe 文件
target_link_libraries(LarkWorkstation PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Qml Qt6::Quick Qt6::QuickControls2 Qt6::Charts Qt6::Network Qt6::WebSockets Qt6::Sql)
set_target_properties(LarkWorkstation PROPERTIES MACOSX_BUNDLE TRUE WIN32_EXECUTABLE TRUE)