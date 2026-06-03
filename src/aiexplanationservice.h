#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "analysisdata.h"
#include "chess/game.h"

// Servicio que consulta la API de OpenAI para generar explicaciones en lenguaje
// natural de movimientos clasificados por Stockfish.
class AIExplanationService : public QObject
{
    Q_OBJECT

public:
    explicit AIExplanationService(QObject *parent = nullptr);

    // Solicita explicación para un movimiento individual.
    void requestExplanation(int moveIndex,
                            const QString& fen,
                            const QString& playedMove,
                            const QString& bestMove,
                            int evalBefore,
                            int evalAfter,
                            MoveClassification classification);

    // Solicita un resumen global de la partida.
    // Emite gameSummaryReady(QString) cuando está listo.
    void requestGameSummary(const chess::GameMetadata& metadata,
                            const QVector<MoveAnalysis>& analysis);

signals:
    void explanationReady(int moveIndex, const QString& explanation);
    void requestFailed(int moveIndex, const QString& error);
    void gameSummaryReady(const QString& summary);
    void gameSummaryFailed(const QString& error);

private:
    QNetworkAccessManager *m_manager;

    void sendRequest(int moveIndex,
                     const QString& fen,
                     const QString& playedMove,
                     const QString& bestMove,
                     int evalBefore,
                     int evalAfter,
                     MoveClassification classification,
                     int retryCount = 0);

    QString buildPrompt(const QString& fen,
                        const QString& playedMove,
                        const QString& bestMove,
                        int evalBefore,
                        int evalAfter,
                        MoveClassification classification) const;

    QString buildSummaryPrompt(const chess::GameMetadata& metadata,
                               const QVector<MoveAnalysis>& analysis) const;
};
