#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QString>

// Wrapper sobre QNetworkAccessManager.
// Adjunta automáticamente el header Authorization: Bearer <token> cuando hay sesión activa.
class HttpClient : public QObject {
    Q_OBJECT

public:
    explicit HttpClient(QObject* parent = nullptr);

    void    setToken(const QString& token);
    QString token() const { return m_token; }

    // Retornan el QNetworkReply* para que el caller conecte finished()
    QNetworkReply* get(const QString& endpoint);
    QNetworkReply* post(const QString& endpoint, const QJsonObject& body);

private:
    QNetworkAccessManager* m_manager;
    QString                m_token;

    QNetworkRequest buildRequest(const QString& endpoint) const;
};
