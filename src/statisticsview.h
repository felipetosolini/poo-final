#pragma once

#include <QWidget>
#include <QVector>
#include <QTableWidget>
#include <QLabel>
#include "analysisdata.h"
#include "statisticsservice.h"

// Vista de estadísticas del usuario.
// Muestra:
//   - Resumen numérico (precisión promedio, blunders, etc.)
//   - Gráfico de barras de precisión por partida (dibujado con QPainter)
//   - Tabla de historial de partidas (QTableWidget)
//
// Emite matchSelected(int matchId) cuando el usuario hace click en una fila.
class StatisticsView : public QWidget
{
    Q_OBJECT

public:
    explicit StatisticsView(QWidget *parent = nullptr);

    void setHistory(const QVector<StatPoint>& history);
    void setUserStats(const UserStats& stats);

signals:
    // Emitido cuando el usuario hace click en una partida de la tabla
    void matchSelected(int matchId);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onTableRowClicked(int row, int column);

private:
    QVector<StatPoint> m_history;
    UserStats          m_stats;
    QTableWidget      *m_table        = nullptr;
    QLabel            *m_summaryLabel = nullptr;
    QLabel            *m_chartTitle   = nullptr;

    static constexpr int CHART_HEIGHT = 180;
    static constexpr int CHART_MARGIN = 48;
    static constexpr int HEADER_AREA  = 120;  // px reservados para labels sobre el gráfico

    void setupUI();
    void updateTable();
    void updateSummary();
    void drawAccuracyChart(QPainter& painter, const QRect& rect) const;
};
