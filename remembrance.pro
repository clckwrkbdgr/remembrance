TARGET = remembrance
VERSION = 0.9.0
TEMPLATE = app
CONFIG += qt release

MOC_DIR = tmp
OBJECTS_DIR = tmp
RCC_DIR = tmp

win32:RC_FILE = appicon.rc
RESOURCES = icons.qrc
TRANSLATIONS = remembrance_ru.ts
SOURCES += main.cpp \
    mainwindow.cpp \
    keywordslistdialog.cpp \
    notedialog.cpp \
    associativedatabase.cpp
HEADERS += mainwindow.h \
    keywordslistdialog.h \
    notedialog.h \
    associativedatabase.h
