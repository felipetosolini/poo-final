#pragma once

#include "piece.h"
#include <vector>
#include <memory>

namespace chess {

class Board {
public:
    Board();
    ~Board();

    // Inicializar posición estándar
    void initStandardPosition();

    // Acceso a piezas
    Piece* getPiece(int row, int col) const;
    void setPiece(int row, int col, Piece* piece);

    // Movimiento y captura
    bool movePiece(int fromRow, int fromCol, int toRow, int toCol);
    Piece* capturePiece(int row, int col);

    // Estado
    void clear();
    PieceColor getTurn() const { return currentTurn; }
    void switchTurn() { currentTurn = (currentTurn == PieceColor::White) ? PieceColor::Black : PieceColor::White; }

    // FEN (Forsyth-Edwards Notation) — para análisis
    QString toFen() const;
    bool fromFen(const QString& fen);

    // Clonación
    Board* clone() const;

private:
    // Matriz interna: [fila 0-7][columna 0-7]
    // Fila 0 = rank 8 (blancas abajo en pantalla, negras arriba)
    Piece* board[8][8];
    PieceColor currentTurn;

    // Helper para limpiar memoria
    void clearBoard();
};

} // namespace chess
