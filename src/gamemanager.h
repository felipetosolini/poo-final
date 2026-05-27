#pragma once

#include <QObject>
#include <vector>
#include "chess/board.h"
#include "chess/game.h"
#include "chess/move.h"

class GameManager : public QObject {
    Q_OBJECT

public:
    explicit GameManager(QObject *parent = nullptr);

    // Cargar partida
    void loadGame(const std::vector<chess::Move>& moves);
    void clearGame();

    // Metadatos de la partida activa (jugadores, fecha, apertura…)
    void setMetadata(const chess::GameMetadata& meta) { m_metadata = meta; }
    const chess::GameMetadata& getMetadata() const { return m_metadata; }

    // Navegación
    void goToMove(int index);
    void previousMove();
    void nextMove();
    void startGame();
    void endGame();

    // Estado
    int getCurrentMoveIndex() const { return currentMoveIndex; }
    int getTotalMoves() const { return moves.size(); }
    const std::vector<chess::Move>& getMoves() const { return moves; }
    chess::Board getCurrentBoard() const { return currentBoard; }

    // Realizar movimiento
    void makeMove(const chess::Move& move);

signals:
    void boardUpdated(const chess::Board& board);
    void moveNavigated(int index);
    void moveMade(const chess::Move& move);
    void gameLoaded(int totalMoves);
    void gameEnded();

private slots:
    void onBoardUpdated();

private:
    std::vector<chess::Move> moves;
    chess::Board currentBoard;
    chess::Board startingBoard;
    chess::GameMetadata m_metadata;
    int currentMoveIndex = -1;
    
    void applyMovesToBoard(int upToIndex);
};
