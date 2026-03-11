cmake_minimum_required(VERSION 3.16)

project(ITTracker
    VERSION 1.0.0
    DESCRIPTION "IT Business Tracker - Evidencija Zarade"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt auto tools
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 (falls back to Qt5)
find_package(Qt6 QUIET COMPONENTS Core Widgets Sql Charts)
if(NOT Qt6_FOUND)
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets Sql Charts)
    set(QT_VERSION_MAJOR 5)
    message(STATUS "Using Qt5")
else()
    set(QT_VERSION_MAJOR 6)
    message(STATUS "Using Qt6")
endif()

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/database.cpp
    src/dashboard.cpp
    src/clientspage.cpp
    src/worklogpage.cpp
    src/jobdialog.cpp
    src/clientdialog.cpp
)

set(HEADERS
    src/mainwindow.h
    src/database.h
    src/dashboard.h
    src/clientspage.h
    src/worklogpage.h
    src/jobdialog.h
    src/clientdialog.h
    src/style.h
)

# Windows: GUI application (no console window)
if(WIN32)
    add_executable(ITTracker WIN32 ${SOURCES} ${HEADERS})
else()
    add_executable(ITTracker ${SOURCES} ${HEADERS})
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(ITTracker PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Sql
        Qt6::Charts
    )
else()
    target_link_libraries(ITTracker PRIVATE
        Qt5::Core
        Qt5::Widgets
        Qt5::Sql
        Qt5::Charts
    )
endif()

# Copy Qt DLLs on Windows (Release only)
if(WIN32)
    add_custom_command(TARGET ITTracker POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Run windeployqt manually if needed"
    )
endif()

# Set output directory
set_target_properties(ITTracker PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
