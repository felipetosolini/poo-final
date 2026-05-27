#pragma once
#include <QObject>
#include <QString>
#include "userdata.h"

class HttpClient;

// Maneja autenticación con el backend: login y registro.
// Emite signals con el resultado de cada operación.
class AuthService : public QObject {
    Q_OBJECT

public:
    explicit AuthService(HttpClient* client, QObject* parent = nullptr);

    void login(const QString& username, const QString& password);
    void registerUser(const QString& username, const QString& email, const QString& password);

signals:
    void loginSuccess(const UserData& user);
    void loginFailed(const QString& error);
    void registerSuccess();
    void registerFailed(const QString& error);

private:
    HttpClient* m_client;
};
