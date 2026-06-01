#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

enum class MoveClassification {
    Best,
    Excellent,
    Good,
    Inaccuracy,
    Mistake,
    Blunder
};

inline QString classificationToString(MoveClassification c)
{
    switch (c) {
        case MoveClassification::Best:       return "Best";
        case MoveClassification::Excellent:  return "Excellent";
        case MoveClassification::Good:       return "Good";
        case MoveClassification::Inaccuracy: return "Inaccuracy";
        case MoveClassification::Mistake:    return "Mistake";
        case MoveClassification::Blunder:    return "Blunder";
    }
    return "Unknown";
}

inline MoveClassification classifyMove(int deltaCp)
{
    if (deltaCp == 0)   return MoveClassification::Best;
    if (deltaCp <= 10)  return MoveClassification::Excellent;
    if (deltaCp <= 25)  return MoveClassification::Good;
    if (deltaCp <= 100) return MoveClassification::Inaccuracy;
    if (deltaCp <= 300) return MoveClassification::Mistake;
    return MoveClassification::Blunder;
}

struct MoveAnalysis {
    int    moveIndex  = 0;
    int    evalBefore = 0;   // eval antes del movimiento (centipawns)
    int    evalAfter  = 0;   // eval después del movimiento (centipawns)
    int    delta      = 0;   // pérdida vs mejor jugada (siempre >= 0)
    bool   isMate     = false;
    int    mateIn     = 0;
    QString fen;             // FEN de la posición ANTES del movimiento
    QString bestMove;
    QString playedMove;
    QStringList pv;
    MoveClassification classification = MoveClassification::Good;

    static QString classificationToString(MoveClassification c) {
        return ::classificationToString(c);
    }
};

struct StatPoint {
    int     matchId       = 0;
    QString date;
    QString result;
    double  accuracyWhite = 0.0;
    double  accuracyBlack = 0.0;
};
