#include "evaluationbarwidget.h"
#include <QPainter>
#include <QFont>
#include <cmath>

EvaluationBarWidget::EvaluationBarWidget(QWidget *parent)
    : QWidget(parent)
    , m_animation(new QPropertyAnimation(this, "evalNorm", this))
{
    setMinimumSize(36, 200);
    m_animation->setDuration(300);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);
}

void EvaluationBarWidget::updateEval(int cp)
{
    m_isMate = false;
    m_rawCp  = cp;
    animateTo(sigmoid(cp));
}

void EvaluationBarWidget::setMate(int movesToMate)
{
    m_isMate = true;
    m_mateIn = movesToMate;
    m_rawCp  = movesToMate > 0 ? 30000 : -30000;
    animateTo(movesToMate > 0 ? 1.0f : 0.0f);
}

void EvaluationBarWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int labelH = 20;
    const int barX   = 0;
    const int barY   = labelH;
    const int barW   = width();
    const int barH   = height() - labelH * 2;

    // Fondo negro (ventaja negras)
    p.fillRect(barX, barY, barW, barH, QColor(40, 40, 40));

    // Parte blanca proporcional a m_evalNorm
    int whiteH = static_cast<int>(m_evalNorm * barH);
    p.fillRect(barX, barY + (barH - whiteH), barW, whiteH, Qt::white);

    // Línea central
    p.setPen(QPen(QColor(150, 150, 150), 1, Qt::DashLine));
    p.drawLine(barX, barY + barH / 2, barX + barW, barY + barH / 2);

    // Borde
    p.setPen(QPen(Qt::black, 1));
    p.drawRect(barX, barY, barW - 1, barH - 1);

    // Etiqueta de evaluación
    QString label = evalLabel();
    QFont font("Monospace", 8, QFont::Bold);
    p.setFont(font);

    // Color de texto contrasta con el fondo donde se dibuja
    bool textInWhite = (m_evalNorm > 0.5f);
    p.setPen(textInWhite ? Qt::black : Qt::white);
    p.drawText(QRect(barX, barY + barH - labelH, barW, labelH),
               Qt::AlignCenter, label);
}

float EvaluationBarWidget::sigmoid(int cp)
{
    // Función sigmoide escalada: 50 cp ≈ punto de inflexión suave
    return static_cast<float>(1.0 / (1.0 + std::exp(-cp / 400.0)));
}

QString EvaluationBarWidget::evalLabel() const
{
    if (m_isMate) {
        return m_mateIn > 0 ? QString("M%1").arg(m_mateIn)
                            : QString("M%1").arg(m_mateIn); // e.g. "M3" o "M-3"
    }
    double pawns = m_rawCp / 100.0;
    if (pawns >= 0)
        return QString("+%1").arg(pawns, 0, 'f', 1);
    return QString("%1").arg(pawns, 0, 'f', 1);
}

void EvaluationBarWidget::animateTo(float targetNorm)
{
    if (m_animation->state() == QAbstractAnimation::Running)
        m_animation->stop();
    m_animation->setStartValue(m_evalNorm);
    m_animation->setEndValue(targetNorm);
    m_animation->start();
}
