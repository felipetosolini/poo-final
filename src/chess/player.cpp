#include "player.h"

namespace chess {

Player::Player(const QString& name, PieceColor color)
    : name(name), color(color) {}

double Player::getAverageAccuracy() const {
    if (accuracyCount == 0) return 0.0;
    return totalAccuracy / accuracyCount;
}

void Player::recordResult(const QString& result) {
    gamesPlayed++;
    if (result == "1-0") {
        if (color == PieceColor::White) gamesWon++;
        else gamesLost++;
    } else if (result == "0-1") {
        if (color == PieceColor::Black) gamesWon++;
        else gamesLost++;
    } else {
        gamesDrawn++;
    }
}

void Player::addAccuracy(double accuracy) {
    totalAccuracy += accuracy;
    accuracyCount++;
}

} // namespace chess
