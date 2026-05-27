#pragma once

#include <QString>
#include "piece.h"

namespace chess {

class Player {
public:
    explicit Player(const QString& name, PieceColor color = PieceColor::None);

    QString getName() const { return name; }
    PieceColor getColor() const { return color; }
    void setColor(PieceColor c) { color = c; }

    int getGamesPlayed() const { return gamesPlayed; }
    int getGamesWon()    const { return gamesWon; }
    int getGamesDrawn()  const { return gamesDrawn; }
    int getGamesLost()   const { return gamesLost; }

    double getAverageAccuracy() const;

    // result: "1-0", "0-1", "1/2-1/2", "*"
    void recordResult(const QString& result);
    void addAccuracy(double accuracy);

private:
    QString name;
    PieceColor color;

    int gamesPlayed  = 0;
    int gamesWon     = 0;
    int gamesDrawn   = 0;
    int gamesLost    = 0;
    double totalAccuracy = 0.0;
    int accuracyCount    = 0;
};

} // namespace chess
