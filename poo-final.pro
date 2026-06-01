QT += core gui widgets network sql printsupport svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

INCLUDEPATH += src src/chess

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
    src/sessionmanager.cpp \
    src/httpclient.cpp \
    src/authservice.cpp \
    src/matchhistoryservice.cpp \
    src/aiexplanationservice.cpp \
    src/statisticsservice.cpp \
    src/statisticsview.cpp \
    src/pdfexporter.cpp \
    src/config.cpp \
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
    src/userdata.h \
    src/sessionmanager.h \
    src/httpclient.h \
    src/authservice.h \
    src/matchhistoryservice.h \
    src/aiexplanationservice.h \
    src/statisticsservice.h \
    src/statisticsview.h \
    src/pdfexporter.h \
    src/analysisdata.h \
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
    src/mainwindow.ui \
    src/loginwindow.ui \
    src/registerwindow.ui

RESOURCES += \
    resources/resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Copiar stockfish.exe al directorio del ejecutable en cada build (Windows)
win32 {
    CONFIG(debug, debug|release): EXEDIR = $$OUT_PWD/debug
    else: EXEDIR = $$OUT_PWD/release

    copyStockfish.target   = copy_stockfish
    copyStockfish.commands = \
        -$(MKDIR) $$shell_path($$EXEDIR/bin) $$escape_expand(\n\t) \
        -$(COPY_FILE) $$shell_path($$PWD/bin/stockfish.exe) $$shell_path($$EXEDIR/bin/stockfish.exe)

    QMAKE_EXTRA_TARGETS += copyStockfish
    PRE_TARGETDEPS      += copy_stockfish
}
