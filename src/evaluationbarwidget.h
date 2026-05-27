#pragma once

#include <QWidget>
#include <QPropertyAnimation>
#include <QString>

class EvaluationBarWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(float evalNorm READ evalNorm WRITE setEvalNorm NOTIFY evalNormChanged)

public:
    explicit EvaluationBarWidget(QWidget *parent = nullptr);

    float evalNorm() const { return m_evalNorm; }
    void setEvalNorm(float v) { m_evalNorm = v; update(); }

public slots:
    // cp en centipawns (positivo = blancas, negativo = negras)
    // Pasar 30000 / -30000 para indicar mate
    void updateEval(int cp);
    void setMate(int movesToMate); // positivo = blancas dan mate

signals:
    void evalNormChanged();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    // Valor normalizado [0, 1]: 0.5 = tablas, 1 = blancas ganan, 0 = negras ganan
    float m_evalNorm = 0.5f;
    int   m_rawCp    = 0;
    bool  m_isMate   = false;
    int   m_mateIn   = 0;

    QPropertyAnimation *m_animation = nullptr;

    // Sigmoide: mapea centipawns [-infinito, +infinito] → [0, 1]
    static float sigmoid(int cp);
    QString evalLabel() const;

    void animateTo(float targetNorm);
};
