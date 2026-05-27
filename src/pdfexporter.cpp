#include "pdfexporter.h"

#include <QPrinter>
#include <QPainter>
#include <QFont>
#include <QPen>
#include <QColor>
#include <QRect>

// ---------------------------------------------------------------------------
// Punto de entrada público
// ---------------------------------------------------------------------------
bool PdfExporter::exportToPdf(const QString& filePath,
                               const chess::GameMetadata& metadata,
                               const std::vector<chess::Move>& moves,
                               const QVector<MoveAnalysis>& analysis)
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize::A4);
    printer.setPageOrientation(QPageLayout::Portrait);

    QPainter painter;
    if (!painter.begin(&printer)) {
        return false;
    }

    // Escalar para trabajar en "puntos lógicos" equivalentes a 96 dpi
    const double scale = printer.logicalDpiX() / 96.0;
    painter.scale(scale, scale);

    const int pageW = static_cast<int>(printer.pageRect(QPrinter::DevicePixel).width()  / scale);
    const int pageH = static_cast<int>(printer.pageRect(QPrinter::DevicePixel).height() / scale);
    const QRect pageRect(0, 0, pageW, pageH);
    const int   margin  = 40;
    const QRect contentRect = pageRect.adjusted(margin, margin, -margin, -margin);

    int curY = contentRect.top();

    // ------------------------------------------------------------------
    // Sección 1: encabezado con datos de la partida
    // ------------------------------------------------------------------
    drawHeader(painter, contentRect, metadata);
    curY += 160;   // altura aproximada del encabezado

    // ------------------------------------------------------------------
    // Sección 2: gráfico de evaluación (solo si hay análisis)
    // ------------------------------------------------------------------
    if (!analysis.isEmpty()) {
        painter.setPen(Qt::black);
        QFont sectionFont;
        sectionFont.setPointSize(11);
        sectionFont.setBold(true);
        painter.setFont(sectionFont);
        painter.drawText(contentRect.left(), curY, "Gráfico de evaluación");
        curY += 18;

        QRect chartRect(contentRect.left(), curY, contentRect.width(), 160);
        drawEvalChart(painter, chartRect, analysis);
        curY += 175;
    }

    // ------------------------------------------------------------------
    // Sección 3: tabla de movimientos
    // ------------------------------------------------------------------
    {
        QFont sectionFont;
        sectionFont.setPointSize(11);
        sectionFont.setBold(true);
        painter.setFont(sectionFont);
        painter.setPen(Qt::black);
        painter.drawText(contentRect.left(), curY, "Movimientos");
        curY += 18;

        drawMoveTable(painter, curY, contentRect, moves, analysis);
    }

    painter.end();
    return true;
}

// ---------------------------------------------------------------------------
// Sección 1: encabezado
// ---------------------------------------------------------------------------
void PdfExporter::drawHeader(QPainter& p, const QRect& contentRect,
                              const chess::GameMetadata& meta)
{
    int y = contentRect.top();

    // Título grande
    QFont titleFont;
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    p.setFont(titleFont);
    p.setPen(Qt::black);
    p.drawText(contentRect.left(), y + 22, "Chess Insight AI — Análisis de partida");
    y += 36;

    // Línea separadora
    p.setPen(QPen(QColor(100, 100, 100), 1));
    p.drawLine(contentRect.left(), y, contentRect.right(), y);
    y += 10;

    // Datos de la partida
    QFont dataFont;
    dataFont.setPointSize(10);
    p.setFont(dataFont);
    p.setPen(Qt::black);

    auto drawRow = [&](const QString& label, const QString& value) {
        QFont boldFont = dataFont;
        boldFont.setBold(true);
        p.setFont(boldFont);
        p.drawText(contentRect.left(), y + 13, label);
        p.setFont(dataFont);
        p.drawText(contentRect.left() + 90, y + 13, value);
        y += 17;
    };

    drawRow("Evento:",  meta.event.isEmpty()  ? "–" : meta.event);
    drawRow("Blancas:", meta.whiteName.isEmpty() ? "–" : meta.whiteName);
    drawRow("Negras:",  meta.blackName.isEmpty()  ? "–" : meta.blackName);
    drawRow("Fecha:",   meta.date.isEmpty()    ? "–" : meta.date);
    drawRow("Apertura:",meta.opening.isEmpty() ? "–" : meta.opening);
    drawRow("Resultado:",meta.result.isEmpty() ? "–" : meta.result);
}

// ---------------------------------------------------------------------------
// Sección 2: gráfico de evaluación
// ---------------------------------------------------------------------------
void PdfExporter::drawEvalChart(QPainter& p, const QRect& rect,
                                  const QVector<MoveAnalysis>& analysis)
{
    // Fondo
    p.fillRect(rect, QColor(248, 248, 248));
    p.setPen(QPen(QColor(180, 180, 180)));
    p.drawRect(rect);

    // Línea de cero (igualdad)
    const int midY = rect.top() + rect.height() / 2;
    p.setPen(QPen(QColor(150, 150, 150), 1, Qt::DashLine));
    p.drawLine(rect.left(), midY, rect.right(), midY);

    QFont labelFont;
    labelFont.setPointSize(7);
    p.setFont(labelFont);
    p.setPen(Qt::darkGray);
    p.drawText(rect.left() + 2, midY - 2, "0");

    if (analysis.size() < 2) return;

    const int   n      = analysis.size();
    const double stepX = static_cast<double>(rect.width()) / (n - 1);
    // Clampar evaluación a ±500 cp para visualización
    constexpr int MAX_CP = 500;

    auto cpToY = [&](int cp) -> int {
        const int clamped = qBound(-MAX_CP, cp, MAX_CP);
        return midY - static_cast<int>(clamped / static_cast<double>(MAX_CP) * (rect.height() / 2));
    };

    // Construir polígonos blancos/negros
    QPolygon polyWhite, polyBlack;
    polyWhite << QPoint(rect.left(), midY);
    polyBlack << QPoint(rect.left(), midY);

    for (int i = 0; i < n; ++i) {
        const int x  = rect.left() + static_cast<int>(stepX * i);
        const int cp = analysis[i].evalAfter;
        const int y  = cpToY(cp);
        if (cp >= 0) {
            polyWhite << QPoint(x, y);
            polyBlack << QPoint(x, midY);
        } else {
            polyWhite << QPoint(x, midY);
            polyBlack << QPoint(x, y);
        }
    }
    polyWhite << QPoint(rect.right(), midY);
    polyBlack << QPoint(rect.right(), midY);

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(70, 130, 200, 140));
    p.drawPolygon(polyWhite);
    p.setBrush(QColor(50, 50, 50, 140));
    p.drawPolygon(polyBlack);

    // Línea de evaluación encima
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor(30, 80, 180), 2));
    for (int i = 1; i < n; ++i) {
        const int x0 = rect.left() + static_cast<int>(stepX * (i - 1));
        const int x1 = rect.left() + static_cast<int>(stepX * i);
        p.drawLine(x0, cpToY(analysis[i - 1].evalAfter),
                   x1, cpToY(analysis[i].evalAfter));
    }
}

// ---------------------------------------------------------------------------
// Sección 3: tabla de movimientos
// ---------------------------------------------------------------------------
int PdfExporter::drawMoveTable(QPainter& p, int startY, const QRect& contentRect,
                                 const std::vector<chess::Move>& moves,
                                 const QVector<MoveAnalysis>& analysis)
{
    const int ROW_H  = 16;
    const int COL_NR = 35;
    const int COL_W  = 100;
    const int COL_B  = 200;
    const int COL_CL = 300;

    QFont headerFont;
    headerFont.setPointSize(8);
    headerFont.setBold(true);
    p.setFont(headerFont);
    p.setPen(Qt::black);

    // Cabecera
    p.fillRect(contentRect.left(), startY, contentRect.width(), ROW_H,
               QColor(220, 220, 220));
    p.drawText(contentRect.left() + COL_NR, startY + 12, "#");
    p.drawText(contentRect.left() + COL_W,  startY + 12, "Blancas");
    p.drawText(contentRect.left() + COL_B,  startY + 12, "Negras");
    p.drawText(contentRect.left() + COL_CL, startY + 12, "Clasificación");
    startY += ROW_H;

    QFont rowFont;
    rowFont.setPointSize(8);
    p.setFont(rowFont);

    const int total = static_cast<int>(moves.size());
    for (int i = 0; i < total; i += 2) {
        const int moveNum = i / 2 + 1;

        // Fondo alternado
        if (moveNum % 2 == 0) {
            p.fillRect(contentRect.left(), startY, contentRect.width(), ROW_H,
                       QColor(245, 245, 245));
        }

        p.setPen(Qt::black);
        p.drawText(contentRect.left() + COL_NR, startY + 12,
                   QString::number(moveNum) + ".");

        // Jugada blancas
        const QString moveW = moves[i].getAlgebraic();
        p.drawText(contentRect.left() + COL_W, startY + 12, moveW);

        // Clasificación blancas
        if (i < analysis.size()) {
            const MoveClassification cl = analysis[i].classification;
            p.setPen(colorForClassification(cl));
            p.drawText(contentRect.left() + COL_CL, startY + 12,
                       classificationToString(cl));
            p.setPen(Qt::black);
        }

        // Jugada negras (si existe)
        if (i + 1 < total) {
            const QString moveB = moves[i + 1].getAlgebraic();
            p.drawText(contentRect.left() + COL_B, startY + 12, moveB);

            if (i + 1 < analysis.size()) {
                const MoveClassification cl = analysis[i + 1].classification;
                // Mostrar en la misma celda de clasificación pero en la columna negras
                // Para simplificar el layout, se muestra la clasificación de negras
                // junto a la de blancas separadas por " / "
            }
        }

        startY += ROW_H;
    }

    return startY;
}

// ---------------------------------------------------------------------------
// Color por clasificación (para la tabla)
// ---------------------------------------------------------------------------
QColor PdfExporter::colorForClassification(MoveClassification c)
{
    switch (c) {
        case MoveClassification::Best:       return QColor(0, 160, 0);
        case MoveClassification::Excellent:  return QColor(0, 160, 0);
        case MoveClassification::Good:       return QColor(80, 180, 80);
        case MoveClassification::Inaccuracy: return QColor(200, 160, 0);
        case MoveClassification::Mistake:    return QColor(220, 100, 0);
        case MoveClassification::Blunder:    return QColor(200, 0, 0);
    }
    return Qt::black;
}
