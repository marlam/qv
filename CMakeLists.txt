# Copyright (C) 2019, 2020, 2021, 2022 Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

cmake_minimum_required(VERSION 3.12)
set(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)
set(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)
set(CMAKE_AUTOMOC ON)

project(qv LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

find_package(libtgd 4.0 REQUIRED)
find_package(Qt6 6.2.0 COMPONENTS OpenGLWidgets)
find_package(OpenMP)

include_directories(${TGD_INCLUDE_DIRS})
link_directories(${TGD_LIBRARY_DIRS})
add_executable(qv
    src/main.cpp
    src/version.hpp
    src/gl.hpp src/gl.cpp
    src/textureholder.hpp src/textureholder.cpp
    src/color.hpp
    src/statistic.hpp src/statistic.cpp
    src/histogram.hpp src/histogram.cpp
    src/colormap.hpp src/colormap.cpp
    src/frame.hpp src/frame.cpp
    src/file.hpp src/file.cpp
    src/set.hpp src/set.cpp
    src/parameters.hpp src/parameters.cpp
    src/overlay.hpp src/overlay.cpp
    src/overlay-fallback.hpp src/overlay-fallback.cpp
    src/overlay-info.hpp src/overlay-info.cpp
    src/overlay-value.hpp src/overlay-value.cpp
    src/overlay-statistic.hpp src/overlay-statistic.cpp
    src/overlay-histogram.hpp src/overlay-histogram.cpp
    src/overlay-colormap.hpp src/overlay-colormap.cpp
    src/qv.hpp src/qv.cpp
    src/gui.hpp src/gui.cpp
    src/appicon.rc)
qt6_add_resources(qv "misc" PREFIX "/" FILES
    src/shader-quadtree-vertex.glsl
    src/shader-quadtree-fragment.glsl
    src/shader-view-vertex.glsl
    src/shader-view-fragment.glsl
    src/shader-overlay-vertex.glsl
    src/shader-overlay-fragment.glsl
    colormaps/sequential-0.png
    colormaps/sequential-1.png
    colormaps/sequential-2.png
    colormaps/sequential-3.png
    colormaps/diverging-0.png
    colormaps/diverging-1.png
    colormaps/diverging-2.png
    colormaps/diverging-3.png
    colormaps/qualitative-0.png
    colormaps/qualitative-1.png
    aux/qv-logo-512.png)
set_target_properties(qv PROPERTIES WIN32_EXECUTABLE TRUE)
target_link_libraries(qv ${TGD_LIBRARIES} Qt6::OpenGLWidgets OpenMP::OpenMP_CXX)
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
