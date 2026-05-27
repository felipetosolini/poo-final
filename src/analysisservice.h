#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QStringList>
#include <vector>
#include "chess/move.h"
#include "chess/board.h"
#include "analysisdata.h"

struct AccuracyScore {
    double white        = 0.0;
    double black        = 0.0;
    double whiteOpening = 0.0;
    double whiteMidgame = 0.0;
    double whiteEndgame = 0.0;
    double blackOpening = 0.0;
    double blackMidgame = 0.0;
    double blackEndgame = 0.0;
    int whiteBlunders   = 0;
    int blackBlunders   = 0;
    int whiteMistakes   = 0;
    int blackMistakes   = 0;
};

class StockfishEngine;

class AnalysisService : public QObject {
    Q_OBJECT

public:
    explicit AnalysisService(StockfishEngine* engine, QObject *parent = nullptr);

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

    std::vector<chess::Board> m_boards;
    std::vector<chess::Move>  m_moves;
    int m_currentIndex = 0;
    int m_depth        = 18;
    bool m_running     = false;
    bool m_cancelled   = false;

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
