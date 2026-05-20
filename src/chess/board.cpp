#include "board.h"

namespace chess {

Board::Board() : currentTurn(PieceColor::White) {
    // Inicializar matriz a nullptr
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            board[i][j] = nullptr;
        }
    }
}

Board::~Board() {
    clearBoard();
}

void Board::clearBoard() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            delete board[i][j];
            board[i][j] = nullptr;
        }
    }
}

void Board::clear() {
    clearBoard();
    currentTurn = PieceColor::White;
}

void Board::initStandardPosition() {
    clearBoard();
    currentTurn = PieceColor::White;

    // Filas negras (arriba en pantalla)
    board[0][0] = new Rook(PieceColor::Black);
    board[0][1] = new Knight(PieceColor::Black);
    board[0][2] = new Bishop(PieceColor::Black);
    board[0][3] = new Queen(PieceColor::Black);
    board[0][4] = new King(PieceColor::Black);
    board[0][5] = new Bishop(PieceColor::Black);
    board[0][6] = new Knight(PieceColor::Black);
    board[0][7] = new Rook(PieceColor::Black);

    for (int i = 0; i < 8; i++) {
        board[1][i] = new Pawn(PieceColor::Black);
    }

    // Espacios vacíos (filas 2-5)
    for (int i = 2; i < 6; i++) {
        for (int j = 0; j < 8; j++) {
            board[i][j] = nullptr;
        }
    }

    // Filas blancas (abajo en pantalla)
    for (int i = 0; i < 8; i++) {
        board[6][i] = new Pawn(PieceColor::White);
    }

    board[7][0] = new Rook(PieceColor::White);
    board[7][1] = new Knight(PieceColor::White);
    board[7][2] = new Bishop(PieceColor::White);
    board[7][3] = new Queen(PieceColor::White);
    board[7][4] = new King(PieceColor::White);
    board[7][5] = new Bishop(PieceColor::White);
    board[7][6] = new Knight(PieceColor::White);
    board[7][7] = new Rook(PieceColor::White);
}

Piece* Board::getPiece(int row, int col) const {
    if (row < 0 || row > 7 || col < 0 || col > 7) {
        return nullptr;
    }
    return board[row][col];
}

void Board::setPiece(int row, int col, Piece* piece) {
    if (row < 0 || row > 7 || col < 0 || col > 7) {
        return;
    }
    delete board[row][col];
    board[row][col] = piece;
}

bool Board::movePiece(int fromRow, int fromCol, int toRow, int toCol) {
    if (fromRow < 0 || fromRow > 7 || fromCol < 0 || fromCol > 7 ||
        toRow < 0 || toRow > 7 || toCol < 0 || toCol > 7) {
        return false;
    }

    Piece* piece = board[fromRow][fromCol];
    if (!piece) return false;

    // Mover pieza
    board[toRow][toCol] = piece;
    board[fromRow][fromCol] = nullptr;
    return true;
}

Piece* Board::capturePiece(int row, int col) {
    if (row < 0 || row > 7 || col < 0 || col > 7) {
        return nullptr;
    }
    Piece* captured = board[row][col];
    board[row][col] = nullptr;
    return captured;
}

QString Board::toFen() const {
    // Implementación simplificada — solo posición, sin castling/en passant/50moves
    QString fen;
    for (int row = 0; row < 8; row++) {
        int emptyCount = 0;
        for (int col = 0; col < 8; col++) {
            Piece* p = board[row][col];
            if (!p) {
                emptyCount++;
            } else {
                if (emptyCount > 0) {
                    fen += QString::number(emptyCount);
                    emptyCount = 0;
                }
                fen += p->getSymbol();
            }
        }
        if (emptyCount > 0) {
            fen += QString::number(emptyCount);
        }
        if (row < 7) {
            fen += "/";
        }
    }
    fen += " ";
    fen += (currentTurn == PieceColor::White) ? "w" : "b";
    return fen;
}

bool Board::fromFen(const QString& fen) {
    // Implementación simplificada
    clearBoard();
    
    QStringList parts = fen.split(" ");
    if (parts.size() < 2) return false;

    QString position = parts[0];
    QString turn = parts[1];

    currentTurn = (turn == "w") ? PieceColor::White : PieceColor::Black;

    int row = 0;
    int col = 0;

    for (const QChar& c : position) {
        if (c == '/') {
            row++;
            col = 0;
        } else if (c.isDigit()) {
            col += c.digitValue();
        } else {
            PieceColor color = c.isUpper() ? PieceColor::White : PieceColor::Black;
            Piece* piece = nullptr;

            switch (c.toLower().toLatin1()) {
            case 'p': piece = new Pawn(color); break;
            case 'n': piece = new Knight(color); break;
            case 'b': piece = new Bishop(color); break;
            case 'r': piece = new Rook(color); break;
            case 'q': piece = new Queen(color); break;
            case 'k': piece = new King(color); break;
            default: break;
            }

            if (piece && row < 8 && col < 8) {
                board[row][col] = piece;
                col++;
            }
        }
    }

    return true;
}

Board* Board::clone() const {
    Board* cloned = new Board();
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j]) {
                cloned->board[i][j] = board[i][j]->clone();
            }
        }
    }
    cloned->currentTurn = currentTurn;
    return cloned;
}

} // namespace chess
