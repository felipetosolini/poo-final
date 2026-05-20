#include "evaluationbarwidget.h"
#include <QPainter>
#include <cmath>

EvaluationBarWidget::EvaluationBarWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(50, 300);
}

// Se ha movido a inline en el header para permitir la animación Q_PROPERTY

void EvaluationBarWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    int barHeight = height() - 40;
    int barWidth = width() - 20;
    int barX = 10;
    int barY = 10;
    
    // Fondo (neutro)
    painter.fillRect(barX, barY, barWidth, barHeight, Qt::gray);
    
    // Parte blanca (ventaja blancas)
    float whiteHeight = (50.0f + currentEval / 2.0f) / 100.0f * barHeight;
    painter.fillRect(barX, barY + barHeight - whiteHeight, barWidth, 
                     static_cast<int>(whiteHeight), Qt::white);
    
    // Borde
    painter.setPen(QPen(Qt::black, 2));
    painter.drawRect(barX, barY, barWidth, barHeight);
    
    // Etiquetas
    painter.setFont(QFont("Arial", 8));
    painter.setPen(Qt::black);
    
    painter.drawText(barX + 5, barY - 5, QString::number(static_cast<int>(currentEval)));
    painter.drawText(barX + 5, barY + barHeight + 20, "0.00");
}
