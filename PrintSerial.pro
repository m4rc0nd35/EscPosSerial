QT += core serialport

CONFIG += c++11 console

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += main.cpp \
    escposusb.cpp

HEADERS += \
    escposusb.h

#Setup path libs
INCLUDEPATH += $$PWD/LIB
DEPENDPATH += $$PWD/LIB
unix:!macx: LIBS += -L$$PWD/LIB/ -lusb-1.0

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
