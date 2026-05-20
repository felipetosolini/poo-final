#include "gamemanager.h"

GameManager::GameManager(QObject *parent)
    : QObject(parent)
{
    startingBoard.initStandardPosition();
    currentBoard = startingBoard;
}

void GameManager::loadGame(const std::vector<chess::Move>& movesToLoad) {
    moves = movesToLoad;
    currentMoveIndex = -1;
    currentBoard = startingBoard;
    startingBoard = startingBoard;
    
    emit gameLoaded(moves.size());
}

void GameManager::clearGame() {
    moves.clear();
    currentMoveIndex = -1;
    currentBoard = startingBoard;
}

void GameManager::goToMove(int index) {
    if (index < -1 || index >= static_cast<int>(moves.size())) {
        return;
    }
    
    currentMoveIndex = index;
    applyMovesToBoard(index);
    
    emit moveNavigated(index);
    emit boardUpdated(currentBoard);
}

void GameManager::previousMove() {
    if (currentMoveIndex > -1) {
        goToMove(currentMoveIndex - 1);
    }
}

void GameManager::nextMove() {
    if (currentMoveIndex < static_cast<int>(moves.size()) - 1) {
        goToMove(currentMoveIndex + 1);
    }
}

void GameManager::startGame() {
    currentMoveIndex = -1;
    currentBoard = startingBoard;
    emit boardUpdated(currentBoard);
}

void GameManager::endGame() {
    if (!moves.empty()) {
        goToMove(moves.size() - 1);
    }
    emit gameEnded();
}

void GameManager::makeMove(const chess::Move& move) {
    // Aplicar movimiento al tablero actual
    if (currentBoard.movePiece(move.getFromRow(), move.getFromCol(),
                               move.getToRow(), move.getToCol())) {
        moves.push_back(move);
        currentMoveIndex = moves.size() - 1;
        
        currentBoard.switchTurn();
        
        emit moveMade(move);
        emit moveNavigated(currentMoveIndex);
        emit boardUpdated(currentBoard);
    }
}

void GameManager::applyMovesToBoard(int upToIndex) {
    currentBoard = startingBoard;
    
    for (int i = 0; i <= upToIndex && i < static_cast<int>(moves.size()); i++) {
        const auto& move = moves[i];
        currentBoard.movePiece(move.getFromRow(), move.getFromCol(),
                              move.getToRow(), move.getToCol());
        currentBoard.switchTurn();
    }
}

void GameManager::onBoardUpdated() {
    // Slot interno para procesar cambios
}
