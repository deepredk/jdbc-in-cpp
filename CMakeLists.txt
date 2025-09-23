cmake_minimum_required(VERSION 4.0)
project(jdbc_in_cpp)

set(CMAKE_CXX_STANDARD 20)

include_directories(/opt/homebrew/include)

find_package(ODBC REQUIRED)

add_executable(jdbc_in_cpp main.cpp
        ODBCConnection.h
        ODBCConnection.cpp
        PreparedStatement.cpp
        SQLUtil.cpp
        SQLUtil.h)
target_link_libraries(jdbc_in_cpp PUBLIC ODBC::ODBC)