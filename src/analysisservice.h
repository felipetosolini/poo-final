#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QStringList>
#include <vector>
#include "chess/move.h"
#include "chess/board.h"

// Clasificación de cada movimiento
enum class MoveClassification {
    Best,        // delta <= 10 cp
    Excellent,   // delta <= 10 cp (alias semántico)
    Good,        // delta <= 25 cp
    Inaccuracy,  // 26 - 100 cp
    Mistake,     // 101 - 300 cp
    Blunder      // > 300 cp
};

// Resultado del análisis de un movimiento
struct MoveAnalysis {
    int moveIndex = 0;
    int evalBefore = 0;      // centipawns antes de jugar
    int evalAfter = 0;       // centipawns después de jugar
    int delta = 0;           // pérdida respecto a la mejor jugada (siempre >= 0)
    QString bestMove;        // mejor jugada según el motor (UCI largo: "e2e4")
    QStringList pv;          // línea principal
    MoveClassification classification = MoveClassification::Good;

    static QString classificationToString(MoveClassification c) {
        switch (c) {
            case MoveClassification::Best:       return "Best";
            case MoveClassification::Excellent:  return "Excellent";
            case MoveClassification::Good:       return "Good";
            case MoveClassification::Inaccuracy: return "Inaccuracy";
            case MoveClassification::Mistake:    return "Mistake";
            case MoveClassification::Blunder:    return "Blunder";
        }
        return "Good";
    }
};

// Métricas de precisión por jugador
struct AccuracyScore {
    double white = 0.0;
    double black = 0.0;
    double whiteOpening = 0.0;   // jugadas 1-10
    double whiteMidgame = 0.0;   // jugadas 11-30
    double whiteEndgame = 0.0;   // jugadas 31+
    double blackOpening = 0.0;
    double blackMidgame = 0.0;
    double blackEndgame = 0.0;
    int whiteBlunders = 0;
    int blackBlunders = 0;
    int whiteMistakes = 0;
    int blackMistakes = 0;
};

class StockfishEngine;

class AnalysisService : public QObject {
    Q_OBJECT

public:
    explicit AnalysisService(StockfishEngine* engine, QObject *parent = nullptr);

    // Analizar todas las posiciones de la partida.
    // boards[i] = posición ANTES del movimiento i
    // moves[i]  = el movimiento jugado en esa posición
    void analyzeGame(const std::vector<chess::Board>& boards,
                     const std::vector<chess::Move>& moves,
                     int depth = 18);

    void cancelAnalysis();

    const QVector<MoveAnalysis>& results() const { return m_results; }
    AccuracyScore accuracyScore() const { return m_accuracy; }

signals:
    void progressUpdated(int current, int total);
    void moveAnalyzed(int moveIndex, MoveAnalysis analysis);
    void analysisComplete(QVector<MoveAnalysis> results, AccuracyScore accuracy);
    void analysisCancelled();

private slots:
    void onEvalUpdated(int cp);
    void onBestMoveFound(QString move);
    void onPvUpdated(QStringList pv);

private:
    StockfishEngine *m_engine;

    // Estado de la iteración
    std::vector<chess::Board> m_boards;
    std::vector<chess::Move>  m_moves;
    int m_currentIndex = 0;
    int m_depth = 18;
    bool m_running = false;
    bool m_cancelled = false;

    // Evaluación de la posición actual (mejor jugada disponible)
    int m_bestEval = 0;
    QString m_bestMove;
    QStringList m_pv;

    QVector<MoveAnalysis> m_results;
    AccuracyScore m_accuracy;

    void analyzeNext();
    void onPositionDone();
    MoveClassification classify(int delta) const;
    AccuracyScore computeAccuracy(const QVector<MoveAnalysis>& results,
                                  int totalMoves) const;
};
