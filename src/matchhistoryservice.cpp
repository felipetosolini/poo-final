#include "matchhistoryservice.h"
#include "httpclient.h"
#include "config.h"
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

MatchHistoryService::MatchHistoryService(HttpClient* client, QObject* parent)
    : QObject(parent)
    , m_client(client)
{}

void MatchHistoryService::fetchMatches()
{
    QNetworkReply* reply = m_client->get(Config::API_MATCHES);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            emit requestFailed(reply->errorString());
            return;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        const QJsonArray    arr = doc.array();

        QVector<MatchData> matches;
        matches.reserve(arr.size());

        for (const QJsonValue& val : arr) {
            const QJsonObject obj = val.toObject();
            MatchData m;
            m.id            = obj.value("id").toInt();
            m.pgn           = obj.value("pgn").toString();
            m.result        = obj.value("result").toString();
            m.playedAt      = obj.value("played_at").toString();
            m.accuracyWhite = obj.value("accuracy_white").toDouble();
            m.accuracyBlack = obj.value("accuracy_black").toDouble();
            matches.append(m);
        }

        emit matchesLoaded(matches);
    });
}

void MatchHistoryService::uploadMatch(const QString& pgn,
                                      const QString& result,
                                      double accuracyWhite,
                                      double accuracyBlack)
{
    QJsonObject body;
    body["pgn"]            = pgn;
    body["result"]         = result;
    body["accuracy_white"] = accuracyWhite;
    body["accuracy_black"] = accuracyBlack;

    QNetworkReply* reply = m_client->post(Config::API_MATCHES, body);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            emit requestFailed(reply->errorString());
            return;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        const int matchId = doc.object().value("id").toInt(0);
        emit matchUploaded(matchId);
    });
}
