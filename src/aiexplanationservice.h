#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "analysisdata.h"

// Servicio que consulta la API de OpenAI para generar explicaciones en lenguaje
// natural de movimientos clasificados por Stockfish.
//
// Uso:
//   service->requestExplanation(moveIndex, fen, played, best, evalBefore, evalAfter, classification)
//   // luego conectar explanationReady(int, QString) para recibir la respuesta
class AIExplanationService : public QObject
{
    Q_OBJECT

public:
    explicit AIExplanationService(QObject *parent = nullptr);

    // Solicita explicación para un movimiento.
    // Parámetros:
    //   moveIndex      — índice del movimiento en la partida (0-based)
    //   fen            — posición FEN antes del movimiento
    //   playedMove     — jugada realizada (notación algebraica, ej: "Nf3")
    //   bestMove       — mejor jugada según Stockfish (notación UCI, ej: "g1f3")
    //   evalBefore     — evaluación antes del movimiento (centipawns)
    //   evalAfter      — evaluación después del movimiento (centipawns)
    //   classification — clasificación del movimiento
    void requestExplanation(int moveIndex,
                            const QString& fen,
                            const QString& playedMove,
                            const QString& bestMove,
                            int evalBefore,
                            int evalAfter,
                            MoveClassification classification);

signals:
    void explanationReady(int moveIndex, const QString& explanation);
    void requestFailed(int moveIndex, const QString& error);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_manager;

    // Estado del request en curso (para retry en caso de 429)
    int                m_pendingMoveIndex   = -1;
    int                m_retryCount         = 0;
    QString            m_lastFen;
    QString            m_lastPlayedMove;
    QString            m_lastBestMove;
    int                m_lastEvalBefore     = 0;
    int                m_lastEvalAfter      = 0;
    MoveClassification m_lastClassification = MoveClassification::Good;

    void sendRequest(int moveIndex,
                     const QString& fen,
                     const QString& playedMove,
                     const QString& bestMove,
                     int evalBefore,
                     int evalAfter,
                     MoveClassification classification);

    QString buildPrompt(const QString& fen,
                        const QString& playedMove,
                        const QString& bestMove,
                        int evalBefore,
                        int evalAfter,
                        MoveClassification classification) const;
};
