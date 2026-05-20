#pragma once

#include <QWidget>
#include <QPropertyAnimation>

class EvaluationBarWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(float evaluation READ evaluation WRITE setEvaluation NOTIFY evaluationChanged)

public:
    explicit EvaluationBarWidget(QWidget *parent = nullptr);

    float evaluation() const { return currentEval; }
    void setEvaluation(float eval) { currentEval = eval; update(); }

signals:
    void evaluationChanged();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    float currentEval = 0.0f;  // -100 (mate negro) a +100 (mate blanco)
    QPropertyAnimation *animation = nullptr;
};
