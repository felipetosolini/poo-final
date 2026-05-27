#include "pgnparser.h"
#include "../utils.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <QRegularExpression>

namespace chess {

// ── helpers internas ────────────────────────────────────────────────────────

namespace {
    struct CandidateSquare {
        int row;
        int col;
    };
}

// Elimina comentarios { } y variantes ( ) con soporte para anidamiento
QString PGNParser::removeVariationsAndComments(const QString& text) {
    QString result;
    result.reserve(text.size());

    int depth = 0;
    bool inComment = false;

    for (int i = 0; i < text.length(); ++i) {
        QChar c = text[i];

        if (inComment) {
            if (c == '}') inComment = false;
            continue;
        }
        if (c == '{') { inComment = true;  continue; }
        if (c == ';') {
            // Comentario hasta fin de línea
            while (i < text.length() - 1 && text[i + 1] != '\n') ++i;
            continue;
        }
        if (c == '(') { ++depth; continue; }
        if (c == ')') { if (depth > 0) --depth; continue; }

        if (depth == 0) result += c;
    }
    return result;
}

// ── API pública ─────────────────────────────────────────────────────────────

std::vector<Game> PGNParser::parseFile(const QString& filePath) {
    std::ifstream file(filePath.toStdString());
    if (!file.is_open()) return {};

    std::ostringstream ss;
    ss << file.rdbuf();
    return parseText(QString::fromStdString(ss.str()));
}

std::vector<Game> PGNParser::parseText(const QString& text) {
    std::vector<Game> games;

    QStringList lines = text.split('\n');
    QString currentHeaders;
    QString currentMoves;
    bool inMoveSection = false;

    for (const QString& rawLine : lines) {
        QString line = rawLine.trimmed();

        if (line.startsWith('[')) {
            // Nuevo bloque de headers: guardar la partida anterior si había
            if (inMoveSection && !currentMoves.trimmed().isEmpty()) {
                games.push_back(parseSingleGame(currentHeaders, currentMoves));
                currentHeaders.clear();
                currentMoves.clear();
                inMoveSection = false;
            }
            currentHeaders += line + "\n";
        } else if (line.isEmpty()) {
            if (!currentHeaders.isEmpty() && !inMoveSection)
                inMoveSection = true;
        } else {
            if (inMoveSection) currentMoves += line + " ";
        }
    }

    // Última partida
    if (!currentHeaders.isEmpty() && !currentMoves.trimmed().isEmpty())
        games.push_back(parseSingleGame(currentHeaders, currentMoves));

    return games;
}

// ── parsing interno ──────────────────────────────────────────────────────────

Game PGNParser::parseSingleGame(const QString& headerBlock, const QString& moveBlock) {
    GameMetadata metadata = parseHeaders(headerBlock);

    Game game;
    game.setMetadata(metadata);

    if (!metadata.whiteName.isEmpty())
        game.setWhitePlayer(Player(metadata.whiteName, PieceColor::White));
    if (!metadata.blackName.isEmpty())
        game.setBlackPlayer(Player(metadata.blackName, PieceColor::Black));

    Board board;
    board.initStandardPosition();

    for (const Move& move : parseMoves(moveBlock, board))
        game.addMove(move);

    return game;
}

GameMetadata PGNParser::parseHeaders(const QString& headerBlock) const {
    GameMetadata meta;

    QRegularExpression rx(R"(\[(\w+)\s+"([^"]*)"\])");
    auto it = rx.globalMatch(headerBlock);

    while (it.hasNext()) {
        auto m = it.next();
        const QString key   = m.captured(1);
        const QString value = m.captured(2);

        if      (key == "Event")                    meta.event     = value;
        else if (key == "Site")                     meta.site      = value;
        else if (key == "Date")                     meta.date      = value;
        else if (key == "White")                    meta.whiteName = value;
        else if (key == "Black")                    meta.blackName = value;
        else if (key == "Result")                   meta.result    = value;
        else if (key == "Opening" || key == "ECO")  meta.opening   = value;
    }

    return meta;
}

std::vector<Move> PGNParser::parseMoves(const QString& moveText, Board& board) const {
    std::vector<Move> moves;

    QString cleaned = removeVariationsAndComments(moveText);

    static const QRegularExpression moveNumRx(R"(^\d+\.+$)");
    static const QRegularExpression resultRx(R"(^(1-0|0-1|1\/2-1\/2|\*)$)");

    QStringList tokens = cleaned.split(QRegularExpression(R"(\s+)"), Qt::SkipEmptyParts);

    PieceColor currentColor = PieceColor::White;

    for (const QString& token : tokens) {
        if (moveNumRx.match(token).hasMatch()) continue;
        if (resultRx.match(token).hasMatch())  continue;
        if (token.startsWith('$'))             continue; // NAG annotation

        SanMove san = parseSAN(token);
        if (san.toRow == -1 && !san.isKingsideCastle && !san.isQueensideCastle)
            continue; // no se pudo parsear (anotación, etc.)

        Move move = resolveMove(board, san, currentColor);
        applyMoveToBoard(board, move, san, currentColor);
        moves.push_back(move);

        currentColor = (currentColor == PieceColor::White)
                       ? PieceColor::Black : PieceColor::White;
    }

    return moves;
}

// ── conversión SAN → coordenadas ─────────────────────────────────────────────

PGNParser::SanMove PGNParser::parseSAN(const QString& token) const {
    SanMove result;
    result.original = token;

    QString s = token;

    // Quitar indicadores de jaque y anotaciones al final
    while (!s.isEmpty()) {
        QChar last = s[s.length() - 1];
        if (last == '+' || last == '#' || last == '!' || last == '?')
            s.chop(1);
        else
            break;
    }
    if (s.isEmpty()) return result;

    // Enroque
    if (s == "O-O-O" || s == "0-0-0") { result.isQueensideCastle = true; return result; }
    if (s == "O-O"   || s == "0-0")   { result.isKingsideCastle  = true; return result; }

    // Promoción con '=' (e.g. e8=Q)
    if (s.contains('=')) {
        int eq = s.indexOf('=');
        if (eq + 1 < s.length()) {
            switch (s[eq + 1].toLatin1()) {
                case 'Q': result.promotion = PieceType::Queen;  break;
                case 'R': result.promotion = PieceType::Rook;   break;
                case 'B': result.promotion = PieceType::Bishop; break;
                case 'N': result.promotion = PieceType::Knight; break;
            }
        }
        s = s.left(eq);
    }

    // Tipo de pieza (primera letra mayúscula)
    if (!s.isEmpty() && s[0].isUpper()) {
        switch (s[0].toLatin1()) {
            case 'N': result.piece = PieceType::Knight; s = s.mid(1); break;
            case 'B': result.piece = PieceType::Bishop; s = s.mid(1); break;
            case 'R': result.piece = PieceType::Rook;   s = s.mid(1); break;
            case 'Q': result.piece = PieceType::Queen;  s = s.mid(1); break;
            case 'K': result.piece = PieceType::King;   s = s.mid(1); break;
            default:  result.piece = PieceType::Pawn;   break;
        }
    }

    // Captura
    result.isCapture = s.contains('x');
    s.remove('x');

    // Promoción sin '=' (e.g. e8Q, e8N — omitimos 'B' por ambigüedad)
    if (result.promotion == PieceType::None && s.length() >= 3) {
        QChar last = s[s.length() - 1];
        QChar rank = s[s.length() - 2];
        if ((rank == '1' || rank == '8') &&
            (last == 'Q' || last == 'R' || last == 'N')) {
            switch (last.toLatin1()) {
                case 'Q': result.promotion = PieceType::Queen;  break;
                case 'R': result.promotion = PieceType::Rook;   break;
                case 'N': result.promotion = PieceType::Knight; break;
            }
            s.chop(1);
        }
    }

    // Casilla de destino = últimos 2 caracteres [a-h][1-8]
    if (s.length() >= 2) {
        QString dest = s.right(2);
        if (dest[0] >= 'a' && dest[0] <= 'h' &&
            dest[1] >= '1' && dest[1] <= '8') {
            int r, c;
            if (Move::squareToCoords(dest, r, c)) {
                result.toRow = r;
                result.toCol = c;
            }
            s.chop(2);
        }
    }

    // Desambiguación restante
    if (s.length() == 1) {
        QChar dis = s[0];
        if (dis >= 'a' && dis <= 'h')     result.fromCol = dis.toLatin1() - 'a';
        else if (dis >= '1' && dis <= '8') result.fromRow = '8' - dis.toLatin1();
    } else if (s.length() == 2) {
        int r, c;
        if (Move::squareToCoords(s, r, c)) {
            result.fromRow = r;
            result.fromCol = c;
        }
    }

    return result;
}

// Usa utils::findFirst para encontrar la pieza de origen
Move PGNParser::resolveMove(const Board& board, const SanMove& san, PieceColor color) const {
    if (san.isKingsideCastle) {
        int row = (color == PieceColor::White) ? 7 : 0;
        return Move(row, 4, row, 6, san.original);
    }
    if (san.isQueensideCastle) {
        int row = (color == PieceColor::White) ? 7 : 0;
        return Move(row, 4, row, 2, san.original);
    }

    // Recolectar candidatos del tipo y color correctos
    std::vector<CandidateSquare> candidates;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            Piece* p = board.getPiece(r, c);
            if (p && p->getColor() == color && p->getType() == san.piece)
                candidates.push_back({r, c});
        }
    }

    // Buscar con findFirst — usa el template de utils.h
    auto found = utils::findFirst(candidates, [&](const CandidateSquare& sq) {
        if (san.fromRow != -1 && sq.row != san.fromRow) return false;
        if (san.fromCol != -1 && sq.col != san.fromCol) return false;
        return canPieceReach(board, san.piece, color,
                             sq.row, sq.col, san.toRow, san.toCol);
    });

    if (found.has_value()) {
        Move move(found->row, found->col, san.toRow, san.toCol, san.original);
        move.setCapture(san.isCapture ||
                        board.getPiece(san.toRow, san.toCol) != nullptr);
        return move;
    }

    // Fallback: coordenadas parciales (no debería ocurrir con PGN válido)
    return Move(san.fromRow >= 0 ? san.fromRow : 0,
                san.fromCol >= 0 ? san.fromCol : 0,
                san.toRow, san.toCol, san.original);
}

// ── reglas de movimiento (sin validación de jaque) ────────────────────────────

bool PGNParser::canPieceReach(const Board& board, PieceType type, PieceColor color,
                               int fromRow, int fromCol, int toRow, int toCol) const {
    if (toRow < 0 || toRow > 7 || toCol < 0 || toCol > 7) return false;

    // No puede capturar pieza propia
    Piece* target = board.getPiece(toRow, toCol);
    if (target && target->getColor() == color) return false;

    int dr = toRow - fromRow;
    int dc = toCol - fromCol;

    switch (type) {

    case PieceType::Pawn: {
        if (color == PieceColor::White) {
            if (dr == -1 && (dc == 1 || dc == -1))  // captura o en passant
                return target != nullptr ||
                       board.getPiece(fromRow, toCol) != nullptr;
            if (dc != 0) return false;
            if (dr == -1) return target == nullptr;
            if (dr == -2 && fromRow == 6)
                return target == nullptr &&
                       board.getPiece(fromRow - 1, fromCol) == nullptr;
        } else {
            if (dr == 1 && (dc == 1 || dc == -1))
                return target != nullptr ||
                       board.getPiece(fromRow, toCol) != nullptr;
            if (dc != 0) return false;
            if (dr == 1) return target == nullptr;
            if (dr == 2 && fromRow == 1)
                return target == nullptr &&
                       board.getPiece(fromRow + 1, fromCol) == nullptr;
        }
        return false;
    }

    case PieceType::Knight: {
        int adr = std::abs(dr), adc = std::abs(dc);
        return (adr == 2 && adc == 1) || (adr == 1 && adc == 2);
    }

    case PieceType::Bishop: {
        if (std::abs(dr) != std::abs(dc) || dr == 0) return false;
        return isPathClear(board, fromRow, fromCol, toRow, toCol);
    }

    case PieceType::Rook: {
        if ((dr != 0 && dc != 0) || (dr == 0 && dc == 0)) return false;
        return isPathClear(board, fromRow, fromCol, toRow, toCol);
    }

    case PieceType::Queen: {
        if (dr == 0 && dc == 0) return false;
        bool diag  = (std::abs(dr) == std::abs(dc));
        bool ortho = (dr == 0 || dc == 0);
        if (!diag && !ortho) return false;
        return isPathClear(board, fromRow, fromCol, toRow, toCol);
    }

    case PieceType::King:
        return std::abs(dr) <= 1 && std::abs(dc) <= 1 && (dr != 0 || dc != 0);

    default:
        return false;
    }
}

bool PGNParser::isPathClear(const Board& board,
                              int fromRow, int fromCol,
                              int toRow,   int toCol) const {
    int dr = (toRow > fromRow) ? 1 : (toRow < fromRow) ? -1 : 0;
    int dc = (toCol > fromCol) ? 1 : (toCol < fromCol) ? -1 : 0;

    int r = fromRow + dr;
    int c = fromCol + dc;

    while (r != toRow || c != toCol) {
        if (board.getPiece(r, c) != nullptr) return false;
        r += dr;
        c += dc;
    }
    return true;
}

// ── aplicar movimiento al tablero interno ──────────────────────────────────

void PGNParser::applyMoveToBoard(Board& board, const Move& move,
                                  const SanMove& san, PieceColor color) const {
    int fr = move.getFromRow(), fc = move.getFromCol();
    int tr = move.getToRow(),   tc = move.getToCol();

    if (san.isKingsideCastle) {
        int row = (color == PieceColor::White) ? 7 : 0;
        board.movePiece(row, 4, row, 6);
        board.movePiece(row, 7, row, 5);
        board.switchTurn();
        return;
    }
    if (san.isQueensideCastle) {
        int row = (color == PieceColor::White) ? 7 : 0;
        board.movePiece(row, 4, row, 2);
        board.movePiece(row, 0, row, 3);
        board.switchTurn();
        return;
    }

    // En passant: peón captura en diagonal hacia casilla vacía
    Piece* moving = board.getPiece(fr, fc);
    if (moving && moving->getType() == PieceType::Pawn && fc != tc &&
        board.getPiece(tr, tc) == nullptr) {
        delete board.capturePiece(fr, tc); // eliminar peón capturado
    }

    // Liberar pieza capturada para evitar memory leak (movePiece no la elimina)
    if (board.getPiece(tr, tc) != nullptr)
        delete board.capturePiece(tr, tc);

    board.movePiece(fr, fc, tr, tc);

    // Promoción: reemplazar peón con la pieza elegida
    if (san.promotion != PieceType::None) {
        Piece* promoted = nullptr;
        switch (san.promotion) {
            case PieceType::Queen:  promoted = new Queen(color);  break;
            case PieceType::Rook:   promoted = new Rook(color);   break;
            case PieceType::Bishop: promoted = new Bishop(color); break;
            case PieceType::Knight: promoted = new Knight(color); break;
            default: break;
        }
        if (promoted) board.setPiece(tr, tc, promoted);
    }

    board.switchTurn();
}

} // namespace chess
