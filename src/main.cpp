#include <QApplication>
#include <QFile>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Chess Insight AI");
    app.setOrganizationName("POO-Final");

    // Cargar estilos desde archivo QSS
    QFile styleFile(":/styles.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        app.setStyleSheet(style);
        styleFile.close();
    }

    MainWindow window;
    // La visibilidad la maneja el constructor vía showLoginWindow() / showMainWindow()

    return app.exec();
}
