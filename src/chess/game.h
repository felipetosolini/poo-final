#pragma once

#include <QString>
#include <vector>
#include <optional>
#include "move.h"
#include "player.h"

namespace chess {

struct GameMetadata {
    QString event;
    QString site;
    QString date;
    QString whiteName;
    QString blackName;
    QString result;
    QString opening;
};

class Game {
public:
    Game();
    Game(const QString& whiteName, const QString& blackName);

    const GameMetadata& getMetadata() const { return metadata; }
    void setMetadata(const GameMetadata& meta) { metadata = meta; }

    void addMove(const Move& move);
    const std::vector<Move>& getMoves() const { return moves; }
    int getMoveCount() const { return static_cast<int>(moves.size()); }

    const std::optional<Player>& getWhitePlayer() const { return whitePlayer; }
    const std::optional<Player>& getBlackPlayer() const { return blackPlayer; }
    void setWhitePlayer(const Player& p) { whitePlayer = p; }
    void setBlackPlayer(const Player& p) { blackPlayer = p; }

    QString getResult() const { return metadata.result; }
    bool isValid() const { return !moves.empty(); }

private:
    GameMetadata metadata;
    std::vector<Move> moves;
    std::optional<Player> whitePlayer;
    std::optional<Player> blackPlayer;
};

} // namespace chess
