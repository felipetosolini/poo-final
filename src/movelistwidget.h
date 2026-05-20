#pragma once

#include <QListWidget>
#include "chess/move.h"

class MoveListWidget : public QListWidget {
    Q_OBJECT

public:
    explicit MoveListWidget(QWidget *parent = nullptr);

    void addMove(const QString& algebraic, int moveIndex);
    void clearMoves();
    void setCurrentMove(int index);

signals:
    void moveSelected(int index);

private slots:
    void onItemClicked(QListWidgetItem *item);

private:
    std::vector<int> moveIndices;
};
