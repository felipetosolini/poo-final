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
    const int barY   = 0;
    const int barW   = width();
    const int barH   = height();

    // Fondo negro (ventaja negras)
    p.fillRect(barX, barY, barW, barH, QColor(40, 40, 40));

    // Parte blanca proporcional a m_evalNorm (crece desde abajo)
    const int whiteH  = static_cast<int>(m_evalNorm * barH);
    const int whiteY  = barY + barH - whiteH;
    p.fillRect(barX, whiteY, barW, whiteH, Qt::white);

    // Línea central (igualdad)
    const int midY = barY + barH / 2;
    p.setPen(QPen(QColor(150, 150, 150), 1, Qt::DashLine));
    p.drawLine(barX, midY, barX + barW, midY);

    // Borde
    p.setPen(QPen(Qt::black, 1));
    p.drawRect(barX, barY, barW - 1, barH - 1);

    // ── Etiqueta ──────────────────────────────────────────────────────────
    // Cuando blancas ganan: etiqueta en la sección blanca (abajo), texto negro
    // Cuando negras ganan:  etiqueta en la sección negra (arriba), texto blanco
    const QString label      = evalLabel();
    QFont         labelFont("Monospace", 8, QFont::Bold);
    p.setFont(labelFont);

    const bool whiteWinning = (m_evalNorm >= 0.5f);
    if (whiteWinning) {
        // Sección blanca está abajo — posicionar etiqueta en la parte inferior
        const int labelY = barY + barH - labelH - 2;
        p.setPen(Qt::black);
        p.drawText(QRect(barX + 1, labelY, barW - 2, labelH),
                   Qt::AlignCenter, label);
    } else {
        // Sección negra está arriba — posicionar etiqueta en la parte superior
        const int labelY = barY + 2;
        p.setPen(Qt::white);
        p.drawText(QRect(barX + 1, labelY, barW - 2, labelH),
                   Qt::AlignCenter, label);
    }
}

float EvaluationBarWidget::sigmoid(int cp)
{
    // Función sigmoide escalada: 50 cp ≈ punto de inflexión suave
    return static_cast<float>(1.0 / (1.0 + std::exp(-cp / 400.0)));
}

QString EvaluationBarWidget::evalLabel() const
{
    if (m_isMate) {
        // M3 = blancas dan mate en 3, -M3 = negras dan mate en 3
        return m_mateIn > 0 ? QString("M%1").arg(m_mateIn)
                            : QString("-M%1").arg(qAbs(m_mateIn));
    }
    const double pawns = m_rawCp / 100.0;
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
