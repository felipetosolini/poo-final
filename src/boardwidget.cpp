#include "boardwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QSvgRenderer>
#include <QBuffer>

BoardWidget::BoardWidget(QWidget *parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(400, 400);
    board.initStandardPosition();
    loadPiecePixmaps();
}

BoardWidget::~BoardWidget() {
}

void BoardWidget::setBoard(const chess::Board& b) {
    board = b;
    update();
}

void BoardWidget::clearSelection() {
    selectedRow = selectedCol = -1;
    validMoves.clear();
    highlights.clear();
    update();
}

void BoardWidget::highlightSquare(int row, int col, const QColor& color) {
    if (row >= 0 && row < 8 && col >= 0 && col < 8) {
        highlights[{row, col}] = color;
    }
}

void BoardWidget::setValidMoves(const std::vector<chess::Move>& moves) {
    validMoves = moves;
    update();
}

void BoardWidget::loadPiecePixmaps() {
    const char* prefixes[2] = {"w", "b"};
    const char* suffixes[6] = {"P", "N", "B", "R", "Q", "K"};

    for (int c = 0; c < 2; ++c) {
        for (int t = 0; t < 6; ++t) {
            const QString path = QString(":/pieces/%1%2.svg")
                                     .arg(prefixes[c]).arg(suffixes[t]);
            QSvgRenderer renderer(path);
            QPixmap pm(128, 128);
            pm.fill(Qt::transparent);
            if (renderer.isValid()) {
                QPainter p(&pm);
                renderer.render(&p);
            }
            piecePixmaps[c][t] = pm;
        }
    }
}

void BoardWidget::updateSquareSize() {
    int size = qMin(width(), height());
    squareSize = size / 8;
    boardX = (width() - size) / 2;
    boardY = (height() - size) / 2;
}

std::pair<int, int> BoardWidget::screenToBoard(QPoint pos) {
    int col = (pos.x() - boardX) / squareSize;
    int row = (pos.y() - boardY) / squareSize;
    
    if (col >= 0 && col < 8 && row >= 0 && row < 8) {
        return {row, col};
    }
    return {-1, -1};
}

QPoint BoardWidget::boardToScreen(int row, int col) {
    return QPoint(boardX + col * squareSize, boardY + row * squareSize);
}

QPixmap BoardWidget::getPiecePixmap(const chess::Piece* piece) const {
    if (!piece) return QPixmap();
    
    int color = (piece->getColor() == chess::PieceColor::White) ? 0 : 1;
    int type = static_cast<int>(piece->getType());
    
    if (type >= 0 && type < 6) {
        return piecePixmaps[color][type];
    }
    return QPixmap();
}

void BoardWidget::drawCoordinates(QPainter& painter) {
    painter.setFont(QFont("Arial", 10));
    painter.setPen(Qt::black);
    
    // Letras a-h (columnas)
    for (int col = 0; col < 8; col++) {
        QString letter = QString(QChar('a' + col));
        QPoint pos = boardToScreen(7, col);
        painter.drawText(pos.x() + squareSize - 15, boardY + height() + 10,
                         letter);
    }
    
    // Números 8-1 (filas)
    for (int row = 0; row < 8; row++) {
        QString number = QString::number(8 - row);
        QPoint pos = boardToScreen(row, 0);
        painter.drawText(boardX - 20, pos.y() + 15, number);
    }
}

void BoardWidget::paintEvent(QPaintEvent *event) {
    updateSquareSize();
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Dibujar tablero 8x8
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            QPoint topLeft = boardToScreen(row, col);
            
            // Color alterno
            bool isLight = (row + col) % 2 == 0;
            QColor color = isLight ? QColor(240, 217, 181) : QColor(181, 136, 99);
            
            // Resaltado
            if (highlights.count({row, col})) {
                color = highlights.at({row, col});
            }
            
            painter.fillRect(topLeft.x(), topLeft.y(), squareSize, squareSize, color);
            
            // Borde seleccionada
            if (row == selectedRow && col == selectedCol) {
                painter.setPen(QPen(Qt::yellow, 3));
                painter.drawRect(topLeft.x(), topLeft.y(), squareSize - 1, squareSize - 1);
                painter.setPen(Qt::black);
            }
            
            // Marcador de movimientos válidos
            for (const auto& move : validMoves) {
                if (move.getToRow() == row && move.getToCol() == col) {
                    painter.setBrush(Qt::green);
                    painter.drawEllipse(topLeft.x() + squareSize / 2 - 6,
                                       topLeft.y() + squareSize / 2 - 6, 12, 12);
                    painter.setBrush(Qt::NoBrush);
                }
            }
            
            // Dibujar pieza
            chess::Piece* piece = board.getPiece(row, col);
            if (piece && !(isDragging && row == selectedRow && col == selectedCol)) {
                const QPixmap& pm = getPiecePixmap(piece);
                if (!pm.isNull()) {
                    painter.drawPixmap(
                        QRect(topLeft.x(), topLeft.y(), squareSize, squareSize),
                        pm, pm.rect());
                }
            }
        }
    }
    
    // Dibujar coordenadas
    drawCoordinates(painter);
    
    // Dibujar pieza siendo arrastrada
    if (isDragging && !draggedPiece.isNull()) {
        painter.drawPixmap(mapFromGlobal(QCursor::pos()) - QPoint(squareSize/2, squareSize/2),
                          draggedPiece);
    }
}

void BoardWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton) return;
    
    auto [row, col] = screenToBoard(event->pos());
    
    if (row >= 0 && col >= 0) {
        chess::Piece* piece = board.getPiece(row, col);
        
        if (piece) {
            selectedRow = row;
            selectedCol = col;
            isDragging = true;

            const QPixmap& pm = getPiecePixmap(piece);
            draggedPiece = pm.scaled(squareSize, squareSize,
                                     Qt::KeepAspectRatio, Qt::SmoothTransformation);

            emit squareClicked(row, col);
        } else {
            clearSelection();
        }
        
        update();
    }
}

void BoardWidget::mouseMoveEvent(QMouseEvent *event) {
    if (!isDragging || selectedRow < 0) return;
    update();
}

void BoardWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton || selectedRow < 0) return;
    
    isDragging = false;
    draggedPiece = QPixmap();
    
    auto [toRow, toCol] = screenToBoard(event->pos());
    
    if (toRow >= 0 && toCol >= 0) {
        chess::Move move(selectedRow, selectedCol, toRow, toCol);
        emit moveRequested(move);
    }
    
    clearSelection();
    update();
}

void BoardWidget::keyPressEvent(QKeyEvent *event) {
    // Navegación con flechas
    if (event->key() == Qt::Key_Left) {
        emit moveRequested(chess::Move(0, 0, 0, 0, "nav_prev"));
    } else if (event->key() == Qt::Key_Right) {
        emit moveRequested(chess::Move(0, 0, 0, 0, "nav_next"));
    }
}

void BoardWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    update();
}
