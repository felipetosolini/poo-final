#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Chess Insight AI");
    app.setOrganizationName("POO-Final");

    MainWindow window;
    window.show();

    return app.exec();
}
