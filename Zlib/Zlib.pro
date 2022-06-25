QT       -= core gui

TARGET = zlib
TEMPLATE = lib
CONFIG += staticlib

HEADERS += \
    zlib.h \
    zconf.h \
    trees.h \
    inftrees.h \
    inflate.h \
    inffixed.h \
    inffast.h \
    gzguts.h \
    deflate.h \
    crc32.h \
    zutil.h

SOURCES += \
    uncompr.c \
    trees.c \
    inftrees.c \
    inflate.c \
    inffast.c \
    infback.c \
    gzwrite.c \
    gzread.c \
    gzlib.c \
    gzclose.c \
    deflate.c \
    crc32.c \
    compress.c \
    adler32.c \
    zutil.c
