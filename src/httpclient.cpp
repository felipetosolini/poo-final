#include "httpclient.h"
#include "config.h"
#include <QJsonDocument>
#include <QUrl>

HttpClient::HttpClient(QObject* parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{}

void HttpClient::setToken(const QString& token)
{
    m_token = token;
}

QNetworkRequest HttpClient::buildRequest(const QString& endpoint) const
{
    QNetworkRequest request;
    request.setUrl(QUrl(Config::API_BASE_URL + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    if (!m_token.isEmpty()) {
        request.setRawHeader("Authorization",
                             ("Bearer " + m_token).toUtf8());
    }

    return request;
}

QNetworkReply* HttpClient::get(const QString& endpoint)
{
    return m_manager->get(buildRequest(endpoint));
}

QNetworkReply* HttpClient::post(const QString& endpoint, const QJsonObject& body)
{
    const QByteArray data = QJsonDocument(body).toJson(QJsonDocument::Compact);
    return m_manager->post(buildRequest(endpoint), data);
}
