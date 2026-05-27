#pragma once
#include <QString>

// Datos del usuario autenticado — compartido entre AuthService, SessionManager y MainWindow
struct UserData {
    int     userId   = 0;
    QString username;
    QString token;
};
