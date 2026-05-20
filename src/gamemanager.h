#pragma once

#include <QObject>
#include <vector>
#include "chess/board.h"
#include "chess/move.h"

class GameManager : public QObject {
    Q_OBJECT

public:
    explicit GameManager(QObject *parent = nullptr);

    // Cargar partida
    void loadGame(const std::vector<chess::Move>& moves);
    void clearGame();

    // Navegación
    void goToMove(int index);
    void previousMove();
    void nextMove();
    void startGame();
    void endGame();

    // Estado
    int getCurrentMoveIndex() const { return currentMoveIndex; }
    int getTotalMoves() const { return moves.size(); }
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
    int currentMoveIndex = -1;
    
    void applyMovesToBoard(int upToIndex);
};
