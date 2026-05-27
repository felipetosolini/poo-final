#include "movelistwidget.h"

MoveListWidget::MoveListWidget(QWidget *parent)
    : QListWidget(parent)
{
    connect(this, &QListWidget::itemClicked, this, &MoveListWidget::onItemClicked);
}

void MoveListWidget::addMove(const QString& algebraic, int moveIndex) {
    addItem(algebraic);
    moveIndices.push_back(moveIndex);
}

void MoveListWidget::clearMoves() {
    QListWidget::clear();
    moveIndices.clear();
}

void MoveListWidget::setCurrentMove(int index) {
    if (index >= 0 && index < count()) {
        setCurrentRow(index);
    }
}

void MoveListWidget::onItemClicked(QListWidgetItem *item) {
    int row = QListWidget::row(item);
    if (row >= 0 && row < static_cast<int>(moveIndices.size())) {
        emit moveSelected(moveIndices[row]);
    }
}
