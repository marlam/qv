# Copyright (C) 2019, 2020, 2021, 2022 Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

cmake_minimum_required(VERSION 3.12)
set(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)
set(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)
set(CMAKE_AUTOMOC ON)

project(qv)

option(QV_USE_QT5 "Use Qt5 instead of Qt6" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

find_package(libtad 2.0 REQUIRED)
if(QV_USE_QT5)
    find_package(Qt5 5.11.0 COMPONENTS Widgets)
else()
    find_package(Qt6 6.2.0 COMPONENTS Widgets OpenGL OpenGLWidgets)
endif()
find_package(OpenMP)

include_directories(${TAD_INCLUDE_DIRS})
link_directories(${TAD_LIBRARY_DIRS})
if(QV_USE_QT5)
    qt5_add_resources(RESOURCES resources.qrc)
else()
    qt6_add_resources(RESOURCES resources.qrc)
endif()
add_executable(qv
    main.cpp
    version.hpp
    gl.hpp gl.cpp
    textureholder.hpp textureholder.cpp
    color.hpp
    statistic.hpp statistic.cpp
    histogram.hpp histogram.cpp
    colormap.hpp colormap.cpp
    frame.hpp frame.cpp
    file.hpp file.cpp
    set.hpp set.cpp
    parameters.hpp parameters.cpp
    overlay.hpp overlay.cpp
    overlay-fallback.hpp overlay-fallback.cpp
    overlay-info.hpp overlay-info.cpp
    overlay-value.hpp overlay-value.cpp
    overlay-statistic.hpp overlay-statistic.cpp
    overlay-histogram.hpp overlay-histogram.cpp
    overlay-colormap.hpp overlay-colormap.cpp
    qv.hpp qv.cpp
    gui.hpp gui.cpp
    ${RESOURCES})
if(QV_USE_QT5)
    target_link_libraries(qv -lstdc++fs ${TAD_LIBRARIES} Qt5::Widgets OpenMP::OpenMP_CXX)
else()
    target_link_libraries(qv -lstdc++fs ${TAD_LIBRARIES}
        Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets
        OpenMP::OpenMP_CXX)
endif()
install(TARGETS qv RUNTIME DESTINATION bin)
