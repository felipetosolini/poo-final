#pragma once

#include <QObject>
#include <QVector>
#include "analysisdata.h"

class HttpClient;

// Estadísticas agregadas del usuario (provienen del backend)
struct UserStats {
    double avgAccuracyWhite  = 0.0;
    double avgAccuracyBlack  = 0.0;
    int    totalMatches      = 0;
    int    totalBlunders     = 0;
    int    totalMistakes     = 0;
    int    totalInaccuracies = 0;
};

// Servicio de estadísticas: consulta historial de partidas y métricas del backend.
// Usa el HttpClient existente para GET /matches y GET /stats/{user_id}.
class StatisticsService : public QObject
{
    Q_OBJECT

public:
    explicit StatisticsService(HttpClient* client, QObject* parent = nullptr);

    // Obtener estadísticas agregadas del usuario
    void fetchStats(int userId);

    // Obtener historial de partidas para el gráfico de evolución
    void fetchMatchHistory();

signals:
    void statsLoaded(const UserStats& stats);
    void historyLoaded(const QVector<StatPoint>& history);
    void requestFailed(const QString& error);

private:
    HttpClient *m_client;
};
