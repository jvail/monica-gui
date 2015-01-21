contains(QT_VERSION, ^4\\.*\\..*) {
  error("Cannot build Monica GUI with Qt version $${QT_VERSION}. Use at least Qt 5.0.")
}

TEMPLATE = app
TARGET = monica-gui
DESTDIR = ../bin
UI_DIR = temp/ui
OBJECTS_DIR = temp/obj
MOC_DIR = temp/moc
DEFINES += MONICA_GUI


QMAKE_CXXFLAGS += -std=c++0x
QMAKE_CXXFLAGS_RELEASE += -w
QMAKE_CFLAGS_RELEASE += -w

win32 {
  # add specific paths (e.g. boost) in pri file
  include(gui.win.pri)
}

INCLUDEPATH += \
  ../monica \
  ../util \
  ../loki-lib/include

QT += core gui widgets

HEADERS += \
    mainwindow.h \
    debugstream.h \
    workerconfiguration.h \
    csontreemodel.h \
    csontreeitem.h \
    tabconfiguration.h

SOURCES += \
    mainwindow.cpp \
    workerconfiguration.cpp \
    csontreemodel.cpp \
    csontreeitem.cpp \
    tabconfiguration.cpp

FORMS += \
    mainwindow.ui

# monica code
HEADERS += ../monica/src/soilcolumn.h
HEADERS += ../monica/src/soiltransport.h
HEADERS += ../monica/src/soiltemperature.h
HEADERS += ../monica/src/soilmoisture.h
HEADERS += ../monica/src/soilorganic.h
HEADERS += ../monica/src/monica.h
HEADERS += ../monica/src/monica-parameters.h
HEADERS += ../monica/src/crop.h
HEADERS += ../monica/src/debug.h
HEADERS += ../monica/src/conversion.h
HEADERS += ../monica/src/configuration.h

SOURCES += ../monica/src/main.cpp
SOURCES += ../monica/src/soilcolumn.cpp
SOURCES += ../monica/src/soiltransport.cpp
SOURCES += ../monica/src/soiltemperature.cpp
SOURCES += ../monica/src/soilmoisture.cpp
SOURCES += ../monica/src/soilorganic.cpp
SOURCES += ../monica/src/monica.cpp
SOURCES += ../monica/src/monica-parameters.cpp
SOURCES += ../monica/src/crop.cpp
SOURCES += ../monica/src/debug.cpp
SOURCES += ../monica/src/conversion.cpp
SOURCES += ../monica/src/configuration.cpp    

# db library code
HEADERS += ../util/db/db.h
HEADERS += ../util/db/abstract-db-connections.h
HEADERS += ../util/db/sqlite3.h
HEADERS += ../util/db/sqlite3.h

SOURCES += ../util/db/db.cpp
SOURCES += ../util/db/abstract-db-connections.cpp
SOURCES += ../util/db/sqlite3.c

# climate library code
HEADERS += ../util/climate/climate-common.h

SOURCES += ../util/climate/climate-common.cpp

# json library code
HEADERS += ../util/cson/cson_amalgamation_core.h

SOURCES += ../util/cson/cson_amalgamation_core.c

# tools library code
HEADERS += ../util/tools/algorithms.h
HEADERS += ../util/tools/date.h
HEADERS += ../util/tools/read-ini.h
HEADERS += ../util/tools/datastructures.h
HEADERS += ../util/tools/helper.h
HEADERS += ../util/tools/use-stl-algo-boost-lambda.h
HEADERS += ../util/tools/stl-algo-boost-lambda.h

SOURCES += ../util/tools/algorithms.cpp
SOURCES += ../util/tools/date.cpp
SOURCES += ../util/tools/read-ini.cpp

CONFIG += HERMES
DEFINES += STANDALONE
HERMES:DEFINES += NO_MYSQL

unix {
  LIBS = \
    -lm \
    -ldl \
    -lpthread \
    -lboost_filesystem
}
