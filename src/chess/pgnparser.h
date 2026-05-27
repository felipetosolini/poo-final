#pragma once

#include <QString>
#include <vector>
#include "game.h"
#include "board.h"

namespace chess {

class PGNParser {
public:
    // Lee un archivo .pgn con std::ifstream y retorna todas las partidas
    std::vector<Game> parseFile(const QString& filePath);

    // Parsea texto PGN directamente
    std::vector<Game> parseText(const QString& text);

private:
    struct SanMove {
        PieceType piece          = PieceType::Pawn;
        int fromRow              = -1;
        int fromCol              = -1;
        int toRow                = -1;
        int toCol                = -1;
        bool isCapture           = false;
        bool isKingsideCastle    = false;
        bool isQueensideCastle   = false;
        PieceType promotion      = PieceType::None;
        QString original;
    };

    Game parseSingleGame(const QString& headerBlock, const QString& moveBlock);
    GameMetadata parseHeaders(const QString& headerBlock) const;
    std::vector<Move> parseMoves(const QString& moveText, Board& board) const;

    SanMove parseSAN(const QString& token) const;
    Move resolveMove(const Board& board, const SanMove& san, PieceColor color) const;

    bool canPieceReach(const Board& board, PieceType type, PieceColor color,
                       int fromRow, int fromCol, int toRow, int toCol) const;
    bool isPathClear(const Board& board,
                     int fromRow, int fromCol,
                     int toRow, int toCol) const;

    void applyMoveToBoard(Board& board, const Move& move,
                          const SanMove& san, PieceColor color) const;

    static QString removeVariationsAndComments(const QString& text);
};

} // namespace chess
