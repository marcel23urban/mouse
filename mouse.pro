QT += opengl datavisualization core widgets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
LIBS += /usr/include/libusb-1.0/libusb.h -lusb-1.0
LIBS += -L/usr/include/volk -lvolk
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp
HEADERS += \
    baseprocessor.hpp \
    carrierdetection.hpp \
    dsp.hpp \
    fft.hpp \
    filesink.hpp \
    mainwindow.h \
    mousegui.hpp \
    peakdetection.hpp \
    processor_base.hpp \
    sonarview.hpp \
    libmouse.hpp \
    udpsink.hpp \
    tools.hpp

#QMAKE_CXXFLAGS += -O3 -finline-small-functions -static

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
