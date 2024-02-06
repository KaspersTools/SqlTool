cmake_minimum_required(VERSION 3.24.4)
project(HUMMINGBIRD_SQL_TOOL)

set(CMAKE_CXX_STANDARD 23)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
list(APPEND SQLTOOL_HEADERS_PUBLIC
        include/Hummingbird_SQL/Connection.h
        include/Hummingbird_SQL/Structures.h
        include/Hummingbird_SQL/Config.h
)

list(APPEND SQLTOOL_HEADERS_PRIVATE
        private/SqlFunctions.h
)

list(APPEND SQLTOOL_SOURCES
        src/Connection.cpp
        src/Structures.cpp
        src/SqlFunctions.cpp
)

add_library(HummingBird_Sql STATIC ${SQLTOOL_HEADERS_PUBLIC} ${SQLTOOL_HEADERS_PRIVATE} ${SQLTOOL_SOURCES})
target_include_directories(HummingBird_Sql PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(HummingBird_Sql PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private)

#find_package(MySQL)
find_package(MySQLConnectorCPP REQUIRED)
target_include_directories(HummingBird_Sql PRIVATE ${MySQLConnectorCPP_INCLUDE_DIRS})
target_link_libraries(HummingBird_Sql PRIVATE ${MySQLConnectorCPP_LIBRARIES})
message(STATUS "MySQLConnectorCPP_INCLUDE_DIRS: ${MySQLConnectorCPP_INCLUDE_DIRS}")
message(STATUS "MySQLConnectorCPP_LIBRARIES: ${MySQLConnectorCPP_LIBRARIES}")

option(BUILD_TESTS "Build the tests" OFF)
if (BUILD_TESTS)
    add_executable(HummingBird_Sql_Tests tests/main.cpp)
    target_link_libraries(HummingBird_Sql_Tests PRIVATE HummingBird_Sql)
endif ()