#pragma once

#include <QWidget>
#include <vector>
#include "chess/piece.h"

class CapturedPiecesWidget : public QWidget {
    Q_OBJECT

public:
    explicit CapturedPiecesWidget(QWidget *parent = nullptr);

    void addCapturedPiece(const chess::Piece* piece);
    void clearCapturedPieces();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    std::vector<chess::Piece*> capturedWhite;
    std::vector<chess::Piece*> capturedBlack;
};
