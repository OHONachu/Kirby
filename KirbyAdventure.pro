QT       += core gui widgets
TARGET    = KirbyAdventure
TEMPLATE  = app

CONFIG   += c++11

SOURCES += \
    main.cpp \
    game.cpp \
    gamescene.cpp \
    kirby.cpp \
    enemy.cpp \
    projectile.cpp

HEADERS += \
    constants.h \
    game.h \
    gamescene.h \
    kirby.h \
    enemy.h \
    projectile.h
