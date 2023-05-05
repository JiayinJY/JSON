# CMakeList.txt: JSON 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
project ("JSON")
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 11)
# 将源代码添加到此项目的可执行文件。

add_executable (JSON test.cpp)

# TODO: 如有需要，请添加测试并安装目标。
