#pragma once

#include <QString>
#include <QVector>
#include <QPainter>
#include <QPrinter>
#include <QRect>
#include <QColor>
#include "analysisdata.h"
#include "chess/game.h"
#include "chess/move.h"

// Exporta el análisis de una partida a un archivo PDF usando QPrinter + QPainter.
//
// Secciones generadas:
//   1. Datos de la partida (jugadores, fecha, resultado, apertura)
//   2. Gráfico de evaluación centipawns a lo largo de la partida
//   3. Tabla de movimientos con clasificación de errores (una fila por jugada)
class PdfExporter
{
public:
    // Exporta la partida al archivo indicado.
    // Devuelve true si tuvo éxito, false si se canceló o hubo un error.
    //
    //   filePath — ruta de destino (.pdf)
    //   metadata — metadatos de la partida (jugadores, fecha, resultado…)
    //   moves    — lista de movimientos en notación algebraica
    //   analysis — análisis de Stockfish por movimiento (puede estar vacío)
    static bool exportToPdf(const QString& filePath,
                            const chess::GameMetadata& metadata,
                            const std::vector<chess::Move>& moves,
                            const QVector<MoveAnalysis>& analysis,
                            const QString& gameSummary = QString());

private:
    // Helpers de renderizado — cada uno devuelve el nuevo Y tras dibujar
    static int drawHeader(QPainter& p, const QRect& contentRect,
                          const chess::GameMetadata& meta, int startY);

    static void drawEvalChart(QPainter& p, const QRect& rect,
                              const QVector<MoveAnalysis>& analysis);

    static int drawMoveTable(QPainter& p, QPrinter& printer, int startY,
                             const QRect& contentRect,
                             const std::vector<chess::Move>& moves,
                             const QVector<MoveAnalysis>& analysis);

    static QColor colorForClassification(MoveClassification c);
    static bool   isWeakMove(MoveClassification c);
    static void   drawSummarySection(QPainter& p, QPrinter& printer, int startY,
                                     const QRect& contentRect,
                                     const QString& summary);
};
