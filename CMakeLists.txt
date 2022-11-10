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

find_package(libtgd 4.0 REQUIRED)
if(QV_USE_QT5)
    find_package(Qt5 5.11.0 COMPONENTS Widgets)
else()
    find_package(Qt6 6.2.0 COMPONENTS OpenGLWidgets)
endif()
find_package(OpenMP)

include_directories(${TGD_INCLUDE_DIRS})
link_directories(${TGD_LIBRARY_DIRS})
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
    ${RESOURCES} appicon.rc)
set_target_properties(qv PROPERTIES WIN32_EXECUTABLE TRUE)
if(QV_USE_QT5)
    target_link_libraries(qv -lstdc++fs ${TGD_LIBRARIES} Qt5::Widgets OpenMP::OpenMP_CXX)
else()
    target_link_libraries(qv -lstdc++fs ${TGD_LIBRARIES} Qt6::OpenGLWidgets OpenMP::OpenMP_CXX)
endif()
install(TARGETS qv RUNTIME DESTINATION bin)

# Add auxiliary files for Linux-ish systems
if(UNIX)
    install(FILES "aux/qv-logo-16.png"  RENAME "de.marlam.qv.png" DESTINATION share/icons/hicolor/16x16/apps)
    install(FILES "aux/qv-logo-22.png"  RENAME "de.marlam.qv.png" DESTINATION share/icons/hicolor/22x22/apps)
    install(FILES "aux/qv-logo-32.png"  RENAME "de.marlam.qv.png" DESTINATION share/icons/hicolor/32x32/apps)
    install(FILES "aux/qv-logo-48.png"  RENAME "de.marlam.qv.png" DESTINATION share/icons/hicolor/48x48/apps)
    install(FILES "aux/qv-logo-64.png"  RENAME "de.marlam.qv.png" DESTINATION share/icons/hicolor/64x64/apps)
    install(FILES "aux/qv-logo-128.png" RENAME "de.marlam.qv.png" DESTINATION share/icons/hicolor/128x128/apps)
    install(FILES "aux/qv-logo-256.png" RENAME "de.marlam.qv.png" DESTINATION share/icons/hicolor/256x256/apps)
    install(FILES "aux/qv-logo-512.png" RENAME "de.marlam.qv.png" DESTINATION share/icons/hicolor/512x512/apps)
    install(FILES "aux/qv-logo.svg"     RENAME "de.marlam.qv.svg" DESTINATION share/icons/hicolor/scalable/apps)
    install(FILES "aux/de.marlam.qv.desktop" DESTINATION share/applications)
    install(FILES "aux/de.marlam.qv.metainfo.xml" DESTINATION share/metainfo)
endif()
