#pragma once
#include <QObject>
#include <QString>
#include <QVector>

class HttpClient;

// Datos de una partida guardada en el backend
struct MatchData {
    int     id            = 0;
    QString pgn;
    QString result;
    QString playedAt;
    double  accuracyWhite = 0.0;
    double  accuracyBlack = 0.0;
};

// Obtiene el historial de partidas del usuario y sube partidas analizadas.
class MatchHistoryService : public QObject {
    Q_OBJECT

public:
    explicit MatchHistoryService(HttpClient* client, QObject* parent = nullptr);

    void fetchMatches();
    void uploadMatch(const QString& pgn,
                     const QString& result,
                     double accuracyWhite,
                     double accuracyBlack);

signals:
    void matchesLoaded(const QVector<MatchData>& matches);
    void matchUploaded(int matchId);
    void requestFailed(const QString& error);

private:
    HttpClient* m_client;
};
