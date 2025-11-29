cmake_minimum_required(VERSION 3.16)

project(GhostClipboard LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 确保所有必需的 Qt 模块（Core, Gui, Widgets, Network）只查找一次
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

# 启用自动 MOC (Meta-Object Compiler), UIC, RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 确保只包含 .cpp 源文件，并使用正确的初始大写命名
add_executable(GhostClipboard
    main.cpp
    GhostBubble.h
    GhostManager.h
    GhostBubble.cpp
    GhostManager.cpp
)

# 链接所有必需的 Qt 库
target_link_libraries(GhostClipboard PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

# 针对 Windows 子系统的设置 (隐藏控制台窗口)
if(WIN32)
    set_target_properties(GhostClipboard PROPERTIES WIN32_EXECUTABLE ON)
endif()
