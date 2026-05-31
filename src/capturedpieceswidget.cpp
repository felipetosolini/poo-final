#include "capturedpieceswidget.h"
#include "chess/piece.h"
#include <QPainter>
#include <map>

CapturedPiecesWidget::CapturedPiecesWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(80);
}

void CapturedPiecesWidget::clear() {
    capturedByBlack.clear();
    capturedByWhite.clear();
    update();
}

void CapturedPiecesWidget::updateFromBoard(const chess::Board& board) {
    std::map<chess::PieceType, int> whiteCounts, blackCounts;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            chess::Piece* p = board.getPiece(r, c);
            if (!p) continue;
            if (p->getColor() == chess::PieceColor::White)
                whiteCounts[p->getType()]++;
            else
                blackCounts[p->getType()]++;
        }
    }

    struct PieceInfo { chess::PieceType type; int initial; const char* wsym; const char* bsym; };
    static const PieceInfo kPieces[] = {
        {chess::PieceType::Pawn,   8, "♙", "♟"},
        {chess::PieceType::Knight, 2, "♘", "♞"},
        {chess::PieceType::Bishop, 2, "♗", "♝"},
        {chess::PieceType::Rook,   2, "♖", "♜"},
        {chess::PieceType::Queen,  1, "♕", "♛"},
        {chess::PieceType::King,   1, "♔", "♚"},
    };

    capturedByBlack.clear();
    capturedByWhite.clear();

    for (const auto& pi : kPieces) {
        int missingWhite = pi.initial - (whiteCounts.count(pi.type) ? whiteCounts.at(pi.type) : 0);
        for (int i = 0; i < missingWhite; ++i)
            capturedByBlack.append(QString::fromUtf8(pi.wsym));

        int missingBlack = pi.initial - (blackCounts.count(pi.type) ? blackCounts.at(pi.type) : 0);
        for (int i = 0; i < missingBlack; ++i)
            capturedByWhite.append(QString::fromUtf8(pi.bsym));
    }

    update();
}

void CapturedPiecesWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    QFont labelFont("Segoe UI", 9);
    QFont pieceFont("Segoe UI", 14);
    const QColor labelColor(120, 120, 120);
    const QColor pieceColor(210, 210, 210);

    int x = 8, y = 16;

    painter.setFont(labelFont);
    painter.setPen(labelColor);
    painter.drawText(x, y, "Capturadas por Negras:");
    y += 2;

    painter.setFont(pieceFont);
    painter.setPen(pieceColor);
    for (const QString& sym : capturedByBlack) {
        painter.drawText(x, y + 18, sym);
        x += 20;
        if (x > width() - 24) { x = 8; y += 22; }
    }

    y += 26;
    x = 8;

    painter.setFont(labelFont);
    painter.setPen(labelColor);
    painter.drawText(x, y, "Capturadas por Blancas:");
    y += 2;

    painter.setFont(pieceFont);
    painter.setPen(pieceColor);
    for (const QString& sym : capturedByWhite) {
        painter.drawText(x, y + 18, sym);
        x += 20;
        if (x > width() - 24) { x = 8; y += 22; }
    }
}
