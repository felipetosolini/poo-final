#include "game.h"

namespace chess {

Game::Game() = default;

Game::Game(const QString& whiteName, const QString& blackName) {
    metadata.whiteName = whiteName;
    metadata.blackName = blackName;
    whitePlayer = Player(whiteName, PieceColor::White);
    blackPlayer = Player(blackName, PieceColor::Black);
}

void Game::addMove(const Move& move) {
    moves.push_back(move);
}

} // namespace chess
