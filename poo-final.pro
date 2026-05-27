QT += core gui widgets network sql printsupport svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/boardwidget.cpp \
    src/movelistwidget.cpp \
    src/capturedpieceswidget.cpp \
    src/evaluationbarwidget.cpp \
    src/analysissidebar.cpp \
    src/loginwindow.cpp \
    src/registerwindow.cpp \
    src/gamemanager.cpp \
    src/chess/board.cpp \
    src/chess/player.cpp \
    src/chess/game.cpp \
    src/chess/pgnparser.cpp \
    src/stockfishengine.cpp \
    src/analysisservice.cpp

HEADERS += \
    src/mainwindow.h \
    src/boardwidget.h \
    src/movelistwidget.h \
    src/capturedpieceswidget.h \
    src/evaluationbarwidget.h \
    src/analysissidebar.h \
    src/loginwindow.h \
    src/registerwindow.h \
    src/gamemanager.h \
    src/chess/board.h \
    src/chess/piece.h \
    src/chess/move.h \
    src/chess/player.h \
    src/chess/game.h \
    src/chess/pgnparser.h \
    src/config.h \
    src/utils.h \
    src/stockfishengine.h \
    src/analysisservice.h

FORMS += \
    src/mainwindow.ui

RESOURCES += \
    resources/resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
