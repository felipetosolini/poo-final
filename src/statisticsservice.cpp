#include "statisticsservice.h"
#include "httpclient.h"

#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

StatisticsService::StatisticsService(HttpClient* client, QObject* parent)
    : QObject(parent)
    , m_client(client)
{}

void StatisticsService::fetchStats(int userId)
{
    const QString endpoint = QString("/stats/%1").arg(userId);
    QNetworkReply *reply   = m_client->get(endpoint);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            emit requestFailed(reply->errorString());
            return;
        }

        const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();

        UserStats stats;
        stats.avgAccuracyWhite  = obj.value("avg_accuracy_white").toDouble();
        stats.avgAccuracyBlack  = obj.value("avg_accuracy_black").toDouble();
        stats.totalMatches      = obj.value("total_matches").toInt();
        stats.totalBlunders     = obj.value("total_blunders").toInt();
        stats.totalMistakes     = obj.value("total_mistakes").toInt();
        stats.totalInaccuracies = obj.value("total_inaccuracies").toInt();

        emit statsLoaded(stats);
    });
}

void StatisticsService::fetchMatchHistory()
{
    QNetworkReply *reply = m_client->get("/matches");

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            emit requestFailed(reply->errorString());
            return;
        }

        const QJsonArray arr = QJsonDocument::fromJson(reply->readAll()).array();

        QVector<StatPoint> history;
        history.reserve(arr.size());

        for (const QJsonValue& val : arr) {
            const QJsonObject obj = val.toObject();
            StatPoint pt;
            pt.matchId       = obj.value("id").toInt();
            pt.date          = obj.value("played_at").toString();
            pt.result        = obj.value("result").toString();
            pt.accuracyWhite = obj.value("accuracy_white").toDouble();
            pt.accuracyBlack = obj.value("accuracy_black").toDouble();
            history.append(pt);
        }

        emit historyLoaded(history);
    });
}
