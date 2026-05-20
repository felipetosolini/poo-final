#include "capturedpieceswidget.h"
#include <QPainter>

CapturedPiecesWidget::CapturedPiecesWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(150);
}

void CapturedPiecesWidget::addCapturedPiece(const chess::Piece* piece) {
    if (!piece) return;
    
    if (piece->getColor() == chess::PieceColor::White) {
        capturedWhite.push_back(piece->clone());
    } else {
        capturedBlack.push_back(piece->clone());
    }
    
    update();
}

void CapturedPiecesWidget::clearCapturedPieces() {
    for (auto p : capturedWhite) delete p;
    for (auto p : capturedBlack) delete p;
    capturedWhite.clear();
    capturedBlack.clear();
    update();
}

void CapturedPiecesWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setFont(QFont("Arial", 14, QFont::Bold));
    
    int x = 10;
    int y = 10;
    
    // Piezas capturadas de blancas (capturadas por negras)
    painter.drawText(x, y, "Captured by Black:");
    y += 25;
    
    for (const auto& piece : capturedBlack) {
        painter.setPen(Qt::black);
        painter.drawText(x, y, QString(piece->getSymbol()) + " ");
        x += 30;
        if (x > width() - 50) {
            x = 10;
            y += 25;
        }
    }
    
    // Espaciador
    y += 35;
    x = 10;
    
    // Piezas capturadas de negras (capturadas por blancas)
    painter.drawText(x, y, "Captured by White:");
    y += 25;
    
    for (const auto& piece : capturedWhite) {
        painter.setPen(Qt::white);
        painter.drawText(x, y, QString(piece->getSymbol()) + " ");
        x += 30;
        if (x > width() - 50) {
            x = 10;
            y += 25;
        }
    }
}
