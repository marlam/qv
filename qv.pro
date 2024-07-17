# Use CMake to build qv.
# This qmake .pro file only exists to build static binaries for Windows.

HEADERS = \
        src/version.hpp \
        src/color.hpp \
        src/colormap.hpp \
        src/file.hpp \
        src/frame.hpp \
        src/gl.hpp \
        src/textureholder.hpp \
        src/histogram.hpp \
        src/overlay-fallback.hpp \
        src/overlay-info.hpp \
        src/overlay-value.hpp \
        src/overlay-statistic.hpp \
        src/overlay-histogram.hpp \
        src/overlay-colormap.hpp \
        src/overlay.hpp \
        src/parameters.hpp \
        src/qv.hpp \
        src/set.hpp \
        src/statistic.hpp \
        src/gui.hpp

SOURCES = \
        src/colormap.cpp \
        src/file.cpp \
        src/frame.cpp \
        src/gl.cpp \
        src/textureholder.cpp \
        src/histogram.cpp \
        src/overlay-fallback.cpp \
        src/overlay-info.cpp \
        src/overlay-value.cpp \
        src/overlay-statistic.cpp \
        src/overlay-histogram.cpp \
        src/overlay-colormap.cpp \
        src/overlay.cpp \
        src/parameters.cpp \
        src/qv.cpp \
        src/set.cpp \
        src/statistic.cpp \
        src/gui.cpp \
        src/main.cpp

resources.files = \
        src/shader-quadtree-vertex.glsl \
        src/shader-quadtree-fragment.glsl \
        src/shader-view-vertex.glsl \
        src/shader-view-fragment.glsl \
        src/shader-overlay-vertex.glsl \
        src/shader-overlay-fragment.glsl \
        colormaps/sequential-0.png \
        colormaps/sequential-1.png \
        colormaps/sequential-2.png \
        colormaps/sequential-3.png \
        colormaps/diverging-0.png \
        colormaps/diverging-1.png \
        colormaps/diverging-2.png \
        colormaps/diverging-3.png \
        colormaps/qualitative-0.png \
        colormaps/qualitative-1.png \
        res/qv-logo-512.png
resources.prefix = /

RESOURCES = resources

CONFIG += release

QT += openglwidgets

QMAKE_CXXFLAGS += -std=c++17 -fopenmp

LIBS += -ltgd -fopenmp

win32 {
       # For building a static qv.exe using MXE, we need to explicitly link in
       # all the libraries required by static libtgd
       LIBS += -Wl,--allow-multiple-definition \
               -lpoppler-cpp -lpoppler -llcms2 -lfreetype -lharfbuzz -lfreetype \
               -lgta -lpfs -lpng -ljpeg -lmatio -lhdf5_cpp -lhdf5 \
               -lOpenEXR-3_1 -lImath-3_1 -lIlmThread-3_1 -lIex-3_1 \
               -lavformat -lavcodec -lavutil -lswscale -lswresample -lbcrypt -lsecur32 \
               -lgdal -lxml2 -llzma -liconv -lgta -lgif -lmfhdf -ldf -lportablexdr -lhdf5_cpp -lhdf5 -lgeos_c -lgeos -ljson-c -lexpat \
               -ltiff -lwebp -lzstd -llzma -ljpeg \
               -lexiv2 -lmman -lpsapi \
               -lcfitsio \
               -lMagick++-7.Q16HDRI -lMagickWand-7.Q16HDRI -lMagickCore-7.Q16HDRI -lharfbuzz -lfreetype -lharfbuzz_too -lfreetype_too -lfontconfig -llqr-1 -lraw -ljasper -ltiff -lwebp -llzma -lglib-2.0 -luuid -lintl -pthread
}
