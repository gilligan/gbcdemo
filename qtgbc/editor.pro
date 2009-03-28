CC=gcc
CPP=g++ -std=c++99
CFLAGS += -g -std=c++99
CPPFLAGS += -g -std=c++99
CXXFLAGS += -g -std=c++99
LDFLAGS += -g
INCLUDEPATH += ../include
LIBS += -L../lib/hostd -limgkit -lstdkit
DEBUG = 1
HEADERS = editor.h
SOURCES = editor.cpp 
MOC_DIR = work
OBJECTS_DIR = work
UI_DIR = work
UI_HEADERS_DIR = work
target.path = qtgbc
INSTALLS += target
