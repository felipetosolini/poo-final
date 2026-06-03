#include "pdfexporter.h"

#include <QPrinter>
#include <QPainter>
#include <QFont>
#include <QPen>
#include <QColor>
#include <QRect>

// ---------------------------------------------------------------------------
// Helpers locales
// ---------------------------------------------------------------------------
static QString evalToString(int cp)
{
    const double pawns = cp / 100.0;
    if (pawns >= 0)
        return QString("+%1").arg(QString::number(pawns, 'f', 2));
    return QString::number(pawns, 'f', 2);
}

static QString deltaToString(int delta)
{
    if (delta == 0) return QString::fromUtf8("\u00B10");
    return QString("-%1").arg(delta);
}

// ---------------------------------------------------------------------------
// Punto de entrada público
// ---------------------------------------------------------------------------
bool PdfExporter::exportToPdf(const QString& filePath,
                               const chess::GameMetadata& metadata,
                               const std::vector<chess::Move>& moves,
                               const QVector<MoveAnalysis>& analysis,
                               const QString& gameSummary)
{
    // ScreenResolution (96 dpi) evita el bug de escala con HighResolution.
    // Con HighResolution (1200 dpi) + painter.scale() los tamaños de fuente
    // quedan desfasados 12x respecto a las coordenadas lógicas.
    QPrinter printer(QPrinter::ScreenResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize::A4);
    printer.setPageOrientation(QPageLayout::Portrait);

    QPainter painter;
    if (!painter.begin(&printer)) {
        return false;
    }

    // Sin escala adicional: con ScreenResolution las coordenadas ya son 96dpi
    const QRect pageRect    = printer.pageRect(QPrinter::DevicePixel).toRect();
    const int   margin      = 40;
    const QRect contentRect = pageRect.adjusted(margin, margin, -margin, -margin);

    int curY = contentRect.top();

    // ------------------------------------------------------------------
    // Sección 1: encabezado con datos de la partida
    // ------------------------------------------------------------------
    curY = drawHeader(painter, contentRect, metadata, curY);
    curY += 20;

    // ------------------------------------------------------------------
    // Sección 2: gráfico de evaluación (solo si hay análisis)
    // ------------------------------------------------------------------
    if (!analysis.isEmpty()) {
        QFont sectionFont;
        sectionFont.setPointSize(11);
        sectionFont.setBold(true);
        painter.setFont(sectionFont);
        painter.setPen(Qt::black);
        painter.drawText(contentRect.left(), curY + 14, "Gráfico de evaluación");
        curY += 22;

        QRect chartRect(contentRect.left(), curY, contentRect.width(), 130);
        drawEvalChart(painter, chartRect, analysis);
        curY += 145;
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
        painter.drawText(contentRect.left(), curY + 14, "Análisis de movimientos");
        curY += 22;

        drawMoveTable(painter, printer, curY, contentRect, moves, analysis);
    }

    // ------------------------------------------------------------------
    // Sección 4: resumen de IA (si está disponible)
    // ------------------------------------------------------------------
    if (!gameSummary.isEmpty()) {
        drawSummarySection(painter, printer, 0, contentRect, gameSummary);
    }

    painter.end();
    return true;
}

// ---------------------------------------------------------------------------
// Sección 1: encabezado — devuelve el nuevo Y tras dibujar
// ---------------------------------------------------------------------------
int PdfExporter::drawHeader(QPainter& p, const QRect& contentRect,
                             const chess::GameMetadata& meta, int startY)
{
    int y = startY;

    QFont titleFont;
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    p.setFont(titleFont);
    p.setPen(Qt::black);
    p.drawText(contentRect.left(), y + 20, "Chess Insight AI \xe2\x80\x94 An\xc3\xa1lisis de partida");
    y += 30;

    p.setPen(QPen(QColor(100, 100, 100), 1));
    p.drawLine(contentRect.left(), y, contentRect.right(), y);
    y += 10;

    QFont dataFont;
    dataFont.setPointSize(10);
    p.setPen(Qt::black);

    auto drawRow = [&](const QString& label, const QString& value) {
        QFont boldFont = dataFont;
        boldFont.setBold(true);
        p.setFont(boldFont);
        p.drawText(contentRect.left(), y + 13, label);
        p.setFont(dataFont);
        p.drawText(contentRect.left() + 95, y + 13, value);
        y += 17;
    };

    drawRow("Evento:",    meta.event.isEmpty()      ? "\xe2\x80\x93" : meta.event);
    drawRow("Blancas:",   meta.whiteName.isEmpty()  ? "\xe2\x80\x93" : meta.whiteName);
    drawRow("Negras:",    meta.blackName.isEmpty()   ? "\xe2\x80\x93" : meta.blackName);
    drawRow("Fecha:",     meta.date.isEmpty()        ? "\xe2\x80\x93" : meta.date);
    drawRow("Apertura:",  meta.opening.isEmpty()     ? "\xe2\x80\x93" : meta.opening);
    drawRow("Resultado:", meta.result.isEmpty()      ? "\xe2\x80\x93" : meta.result);

    return y;
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
// Sección 3: tabla de movimientos — una fila por jugada individual
// con columnas: #, color, jugada, clasificación, eval, Δcp, mejor jugada
// Soporta paginación automática.
// ---------------------------------------------------------------------------
int PdfExporter::drawMoveTable(QPainter& p, QPrinter& printer, int startY,
                                const QRect& contentRect,
                                const std::vector<chess::Move>& moves,
                                const QVector<MoveAnalysis>& analysis)
{
    // Offsets de columna (relativos a contentRect.left())
    const int X_NR     =   0;   // Número de jugada
    const int X_COLOR  =  32;   // Cuadrado indicador de color
    const int X_MOVE   =  52;   // Notación algebraica
    const int X_CLASIF = 155;   // Clasificación
    const int X_EVAL   = 255;   // Evaluación en peones
    const int X_DELTA  = 315;   // Δ vs mejor jugada
    const int X_BEST   = 380;   // Mejor jugada (solo errores)
    const int ROW_H    =  17;
    const int HDR_H    =  18;

    const int pageBottom = contentRect.bottom();

    // --- Dibuja cabecera de tabla ---
    auto drawTableHeader = [&](int y) -> int {
        QFont hf;
        hf.setPointSize(8);
        hf.setBold(true);
        p.setFont(hf);
        p.fillRect(contentRect.left(), y, contentRect.width(), HDR_H, QColor(45, 85, 145));
        p.setPen(Qt::white);
        const int tx = contentRect.left();
        const int ty = y + 13;
        p.drawText(tx + X_NR,    ty, "#");
        p.drawText(tx + X_COLOR, ty, "Color");
        p.drawText(tx + X_MOVE,  ty, "Jugada");
        p.drawText(tx + X_CLASIF,ty, "Clasificaci\xc3\xb3n");
        p.drawText(tx + X_EVAL,  ty, "Eval");
        p.drawText(tx + X_DELTA, ty, "\xce\x94 cp");
        p.drawText(tx + X_BEST,  ty, "Mejor jugada");
        return y + HDR_H;
    };

    int y = drawTableHeader(startY);

    QFont rowFont;
    rowFont.setPointSize(8);
    p.setFont(rowFont);

    const int total  = static_cast<int>(moves.size());
    int       rowIdx = 0;

    for (int i = 0; i < total; ++i) {
        // Paginación automática
        if (y + ROW_H > pageBottom) {
            printer.newPage();
            y = contentRect.top();
            y = drawTableHeader(y);
            p.setFont(rowFont);
        }

        const bool isWhite = (i % 2 == 0);
        const int  moveNum = i / 2 + 1;

        // Fondo alternado por fila
        const QColor rowBg = (rowIdx % 2 == 0) ? QColor(255, 255, 255)
                                                : QColor(245, 245, 250);
        p.fillRect(contentRect.left(), y, contentRect.width(), ROW_H, rowBg);
        ++rowIdx;

        const int tx = contentRect.left();
        const int ty = y + 12;

        // Número de jugada (solo en la fila de blancas)
        p.setPen(Qt::black);
        if (isWhite) {
            p.drawText(tx + X_NR, ty, QString::number(moveNum) + ".");
        }

        // Cuadrado de color del jugador
        const QRect colorSquare(tx + X_COLOR, y + 3, 11, 11);
        if (isWhite) {
            p.fillRect(colorSquare, Qt::white);
            p.setPen(QPen(Qt::black, 1));
            p.drawRect(colorSquare);
        } else {
            p.setPen(Qt::NoPen);
            p.fillRect(colorSquare, QColor(40, 40, 40));
        }
        p.setPen(Qt::black);

        // Notación algebraica
        p.drawText(tx + X_MOVE, ty, moves[i].getAlgebraic());

        // Datos de análisis Stockfish
        if (i < analysis.size()) {
            const MoveAnalysis& ma = analysis[i];

            // Clasificación con color semántico
            p.setPen(colorForClassification(ma.classification));
            p.drawText(tx + X_CLASIF, ty, classificationToString(ma.classification));

            // Evaluación en peones (verde = ventaja, rojo = desventaja)
            p.setPen(ma.evalAfter >= 0 ? QColor(0, 130, 0) : QColor(180, 0, 0));
            p.drawText(tx + X_EVAL, ty, evalToString(ma.evalAfter));

            // Delta centipawns (pérdida vs mejor jugada)
            p.setPen(ma.delta <= 10 ? QColor(80, 80, 80) : QColor(180, 60, 0));
            p.drawText(tx + X_DELTA, ty, deltaToString(ma.delta));

            // Mejor jugada: solo mostrar cuando es un error significativo
            if (isWeakMove(ma.classification) && !ma.bestMove.isEmpty()) {
                p.setPen(QColor(100, 100, 100));
                p.drawText(tx + X_BEST, ty, ma.bestMove);
            }

            p.setPen(Qt::black);
        }

        // Línea separadora horizontal fina
        p.setPen(QPen(QColor(215, 215, 215), 1));
        p.drawLine(contentRect.left(), y + ROW_H - 1,
                   contentRect.right(), y + ROW_H - 1);

        y += ROW_H;
    }

    return y;
}

// ---------------------------------------------------------------------------
// Color por clasificación
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

// ---------------------------------------------------------------------------
// true si el movimiento merece mostrar la mejor jugada alternativa
// ---------------------------------------------------------------------------
bool PdfExporter::isWeakMove(MoveClassification c)
{
    return c == MoveClassification::Inaccuracy
        || c == MoveClassification::Mistake
        || c == MoveClassification::Blunder;
}

// ---------------------------------------------------------------------------
// Sección 4: resumen de IA — siempre en nueva página
// ---------------------------------------------------------------------------
void PdfExporter::drawSummarySection(QPainter& p, QPrinter& printer, int /*startY*/,
                                      const QRect& contentRect,
                                      const QString& summary)
{
    printer.newPage();
    int y = contentRect.top();

    // Título de sección
    QFont titleFont;
    titleFont.setPointSize(13);
    titleFont.setBold(true);
    p.setFont(titleFont);
    p.setPen(Qt::black);
    p.drawText(contentRect.left(), y + 16, "Resumen de la partida (generado por IA)");
    y += 24;

    // Línea separadora
    p.setPen(QPen(QColor(100, 100, 100), 1));
    p.drawLine(contentRect.left(), y, contentRect.right(), y);
    y += 14;

    // Fondo suave para el bloque de texto
    const int padding = 12;
    const QRect textBoxRect(contentRect.left(), y,
                            contentRect.width(), contentRect.height() - (y - contentRect.top()));
    p.fillRect(textBoxRect.adjusted(0, 0, 0, -20), QColor(248, 248, 252));
    p.setPen(QPen(QColor(180, 180, 200), 1));
    p.drawRect(textBoxRect.adjusted(0, 0, 0, -20));

    // Texto del resumen con word-wrap
    QFont bodyFont;
    bodyFont.setPointSize(10);
    p.setFont(bodyFont);
    p.setPen(QColor(30, 30, 30));

    const QRect textRect = textBoxRect.adjusted(padding, padding,
                                                 -padding, -padding - 20);
    p.drawText(textRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, summary);
}
