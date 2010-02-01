TEMPLATE = app
TARGET = example_5_rpcInterface
SOURCES += main.cpp \
    rpcClient.cpp \
    remoteinterface.cpp
HEADERS += rpcClient.h \
    remoteinterface.h
INCLUDEPATH += ../../source
QT += network \
    xml
CONFIG += console \
    debug_and_release
CONFIG(debug, debug|release):LIBS += -L../../source/debug \
    -lQXmppClient_d
else:LIBS += -L../../source/release \
    -lQXmppClient
OTHER_FILES += README