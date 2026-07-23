# Qt
 qt app
# 监控温度 App 带日志系统

## 1. 项目简介
这是一个基于 Qt 框架开发的工业级温度监控与日志系统。主要功能包括实时模拟温度数据采集、
设备状态监控以及带有时间戳的日志记录。本项目完整展示了 C++ 后台逻辑与前端界面的无缝交互能力。

## 2. 核心功能演示

<img width="1270" height="750" alt="image" src="https://github.com/user-attachments/assets/18bae92b-cbe6-4403-80bd-9d9171a4f04b" />
<img width="1281" height="746" alt="image" src="https://github.com/user-attachments/assets/36e8477f-c462-414c-8323-cdc0f9e1884c" />
<img width="1266" height="750" alt="image" src="https://github.com/user-attachments/assets/904fa509-d130-4d9f-a2a1-c93274d685b5" />

## 3. 技术栈与亮点
*   **开发环境**: Qt 5.x / Qt 6.x, CMake
*   **核心语言**: C++, QML
*   **技术亮点**:
    *   **前后端分离**: 使用 `Q_INVOKABLE` 宏实现 C++ 核心逻辑 (`LogModel`) 与前端 QML 界面的解耦与高效通信。
    *   **实时数据流**: 设计 `DeviceSimulator` 类，模拟工业设备的实时数据上报与状态监控。
    *   **精准日志**: 结合 `QDateTime` 实现精确到秒级的带时间戳操作日志追加，并与界面保持实时同步刷新。
    *   **架构规范**: 严格的内存管理与面向对象设计，符合现代 C++ 工业软件开发规范。

## 4. 编译与运行说明
1. 确保本地已安装 Qt Creator 和相应的编译环境 (如 MinGW 或 MSVC)。
2. 克隆或下载本仓库代码到本地。
3. 使用 Qt Creator 打开根目录下的 `CMakeLists.txt` 文件。
4. 选择合适的构建套件 (Kit) 后，点击左下角的**运行**按钮即可编译启动程序。
