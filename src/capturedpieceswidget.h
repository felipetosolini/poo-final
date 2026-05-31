#pragma once

#include <QWidget>
#include <QString>
#include <QVector>
#include "chess/board.h"

class CapturedPiecesWidget : public QWidget {
    Q_OBJECT

public:
    explicit CapturedPiecesWidget(QWidget *parent = nullptr);

    void updateFromBoard(const chess::Board& board);
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<QString> capturedByBlack;  // white pieces taken by black
    QVector<QString> capturedByWhite;  // black pieces taken by white
};
