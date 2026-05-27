#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

// ---------------------------------------------------------------------------
// Enumeración de clasificación de movimientos según delta vs. mejor jugada
// ---------------------------------------------------------------------------
enum class MoveClassification {
    Best,
    Good,
    Inaccuracy,
    Mistake,
    Blunder
};

inline QString classificationToString(MoveClassification c)
{
    switch (c) {
        case MoveClassification::Best:       return "Best";
        case MoveClassification::Good:       return "Good";
        case MoveClassification::Inaccuracy: return "Inaccuracy";
        case MoveClassification::Mistake:    return "Mistake";
        case MoveClassification::Blunder:    return "Blunder";
    }
    return "Unknown";
}

inline MoveClassification classifyMove(int deltaCp)
{
    if (deltaCp <= 10)  return MoveClassification::Best;
    if (deltaCp <= 25)  return MoveClassification::Good;
    if (deltaCp <= 100) return MoveClassification::Inaccuracy;
    if (deltaCp <= 300) return MoveClassification::Mistake;
    return MoveClassification::Blunder;
}

// ---------------------------------------------------------------------------
// Resultado del análisis de un movimiento individual (producido por Área 4)
// ---------------------------------------------------------------------------
struct MoveAnalysis {
    int    moveIndex    = 0;
    int    evalCp       = 0;      // evaluación tras el movimiento (centipawns)
    bool   isMate       = false;
    int    mateIn       = 0;
    int    deltaCp      = 0;      // diferencia entre jugada jugada y mejor jugada
    QString bestMove;             // mejor jugada en notación UCI (ej: "e2e4")
    QString playedMove;           // jugada realizada (notación algebraica)
    QStringList pvLine;           // línea principal
    MoveClassification classification = MoveClassification::Good;
};

// ---------------------------------------------------------------------------
// Punto de estadística de una partida (para el gráfico de evolución)
// ---------------------------------------------------------------------------
struct StatPoint {
    int     matchId       = 0;
    QString date;
    QString result;          // "1-0", "0-1", "1/2-1/2"
    double  accuracyWhite = 0.0;
    double  accuracyBlack = 0.0;
};
