cmake_minimum_required(VERSION 3.20)

project(recipe-project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(
    SOURCES
    database/database.cpp
    main.cpp
)

find_package(SQLite3 REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    SQLite::SQLite3
    pthread
)