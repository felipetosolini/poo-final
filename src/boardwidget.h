#pragma once

#include <QWidget>
#include <QPixmap>
#include <QPoint>
#include <map>
#include <utility>
#include <vector>
#include "chess/board.h"
#include "chess/move.h"

class BoardWidget : public QWidget {
    Q_OBJECT

public:
    explicit BoardWidget(QWidget *parent = nullptr);
    ~BoardWidget();

    void setBoard(const chess::Board& b);
    chess::Board getBoard() const { return board; }

    void setInteractive(bool interactive);

    // Selección y movimientos válidos
    void clearSelection();
    void highlightSquare(int row, int col, const QColor& color);
    void setValidMoves(const std::vector<chess::Move>& moves);

signals:
    void moveRequested(const chess::Move& move);
    void squareClicked(int row, int col);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    chess::Board board;
    
    // Piezas cargadas como pixmaps
    QPixmap piecePixmaps[2][6];  // [color (0=white, 1=black)][piece type 0-5]
    
    // Estados de UI
    int selectedRow = -1, selectedCol = -1;
    QPoint dragStartPos;
    QPixmap draggedPiece;
    bool isDragging = false;
    
    // Dimensiones dinámicas
    int squareSize = 0;
    int boardX = 0, boardY = 0;
    
    bool m_interactive = true;

    // Resaltado
    std::vector<chess::Move> validMoves;
    std::map<std::pair<int, int>, QColor> highlights;
    
    // Métodos privados
    void loadPiecePixmaps();
    std::pair<int, int> screenToBoard(QPoint pos);
    QPoint boardToScreen(int row, int col);
    void updateSquareSize();
    void drawCoordinates(QPainter& painter);
    QPixmap getPiecePixmap(const chess::Piece* piece) const;
};
