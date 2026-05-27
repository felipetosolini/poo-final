#include "authservice.h"
#include "httpclient.h"
#include "config.h"
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

AuthService::AuthService(HttpClient* client, QObject* parent)
    : QObject(parent)
    , m_client(client)
{}

void AuthService::login(const QString& username, const QString& password)
{
    QJsonObject body;
    body["username"] = username;
    body["password"] = password;

    QNetworkReply* reply = m_client->post(Config::API_AUTH_LOGIN, body);

    connect(reply, &QNetworkReply::finished, this, [this, reply, username]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            const QString detail = doc.object().value("detail").toString(reply->errorString());
            emit loginFailed(detail);
            return;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        const QJsonObject   obj = doc.object();

        if (!obj.contains("access_token")) {
            emit loginFailed(obj.value("detail").toString("Login failed"));
            return;
        }

        UserData user;
        user.token    = obj.value("access_token").toString();
        user.username = username;
        user.userId   = obj.value("user_id").toInt(0);

        emit loginSuccess(user);
    });
}

void AuthService::registerUser(const QString& username,
                               const QString& email,
                               const QString& password)
{
    QJsonObject body;
    body["username"] = username;
    body["email"]    = email;
    body["password"] = password;

    QNetworkReply* reply = m_client->post(Config::API_AUTH_REGISTER, body);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            const QString detail = doc.object().value("detail").toString(reply->errorString());
            emit registerFailed(detail);
            return;
        }

        emit registerSuccess();
    });
}
