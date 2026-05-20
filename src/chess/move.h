#pragma once

#include <QString>

namespace chess {

class Move {
public:
    // Coordenadas 0-7 (fila y columna)
    Move(int fromRow, int fromCol, int toRow, int toCol,
         const QString& algebraic = "")
        : fromRow(fromRow), fromCol(fromCol), toRow(toRow), toCol(toCol),
          algebraicNotation(algebraic), isCapture(false), isCastling(false) {}

    // Getters
    int getFromRow() const { return fromRow; }
    int getFromCol() const { return fromCol; }
    int getToRow() const { return toRow; }
    int getToCol() const { return toCol; }
    QString getAlgebraic() const { return algebraicNotation; }

    bool getIsCapture() const { return isCapture; }
    bool getIsCastling() const { return isCastling; }

    // Setters
    void setCapture(bool b) { isCapture = b; }
    void setCastling(bool b) { isCastling = b; }
    void setAlgebraic(const QString& notation) { algebraicNotation = notation; }

    // Convertir posición (fila, col) a notación de tablero (a1, h8, etc)
    static QString coordsToSquare(int row, int col) {
        if (row < 0 || row > 7 || col < 0 || col > 7) return "";
        char file = 'a' + col;
        char rank = '8' - row;  // fila 0 = rank 8, fila 7 = rank 1
        return QString(file) + rank;
    }

    // Convertir square (a1, h8) a coordenadas
    static bool squareToCoords(const QString& square, int& row, int& col) {
        if (square.length() != 2) return false;
        col = square[0].toLatin1() - 'a';
        row = '8' - square[1].toLatin1();
        return col >= 0 && col <= 7 && row >= 0 && row <= 7;
    }

private:
    int fromRow, fromCol;
    int toRow, toCol;
    QString algebraicNotation;
    bool isCapture;
    bool isCastling;
};

} // namespace chess
