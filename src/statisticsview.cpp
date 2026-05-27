#include "statisticsview.h"

#include <QVBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QFont>

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
StatisticsView::StatisticsView(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void StatisticsView::setupUI()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(10);

    // Título
    auto *title = new QLabel("Estadísticas", this);
    QFont tf = title->font();
    tf.setPointSize(15);
    tf.setBold(true);
    title->setFont(tf);
    layout->addWidget(title);

    // Resumen numérico
    m_summaryLabel = new QLabel("Cargando estadísticas…", this);
    m_summaryLabel->setWordWrap(true);
    layout->addWidget(m_summaryLabel);

    // Título del gráfico
    m_chartTitle = new QLabel("Precisión por partida", this);
    QFont ct = m_chartTitle->font();
    ct.setBold(true);
    m_chartTitle->setFont(ct);
    layout->addWidget(m_chartTitle);

    // Espacio fijo para que paintEvent dibuje el gráfico
    layout->addSpacing(CHART_HEIGHT + 8);

    // Tabla de historial
    m_table = new QTableWidget(0, 4, this);
    m_table->setHorizontalHeaderLabels({"Fecha", "Resultado", "Prec. Blancas (%)", "Prec. Negras (%)"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    layout->addWidget(m_table, 1);

    connect(m_table, &QTableWidget::cellClicked,
            this,    &StatisticsView::onTableRowClicked);
}

// ---------------------------------------------------------------------------
// Setters públicos
// ---------------------------------------------------------------------------
void StatisticsView::setHistory(const QVector<StatPoint>& history)
{
    m_history = history;
    updateTable();
    update();
}

void StatisticsView::setUserStats(const UserStats& stats)
{
    m_stats = stats;
    updateSummary();
}

// ---------------------------------------------------------------------------
// Actualizar resumen de texto
// ---------------------------------------------------------------------------
void StatisticsView::updateSummary()
{
    m_summaryLabel->setText(
        QString("Partidas jugadas: %1  |  Precisión Blancas: %2%  |  "
                "Precisión Negras: %3%  |  Blunders: %4  |  Mistakes: %5  |  Inaccuracies: %6")
            .arg(m_stats.totalMatches)
            .arg(m_stats.avgAccuracyWhite, 0, 'f', 1)
            .arg(m_stats.avgAccuracyBlack, 0, 'f', 1)
            .arg(m_stats.totalBlunders)
            .arg(m_stats.totalMistakes)
            .arg(m_stats.totalInaccuracies));
}

// ---------------------------------------------------------------------------
// Actualizar tabla de historial
// ---------------------------------------------------------------------------
void StatisticsView::updateTable()
{
    m_table->setRowCount(m_history.size());
    for (int i = 0; i < m_history.size(); ++i) {
        const StatPoint& pt = m_history[i];
        m_table->setItem(i, 0, new QTableWidgetItem(pt.date));
        m_table->setItem(i, 1, new QTableWidgetItem(pt.result));
        m_table->setItem(i, 2, new QTableWidgetItem(
            QString::number(pt.accuracyWhite, 'f', 1)));
        m_table->setItem(i, 3, new QTableWidgetItem(
            QString::number(pt.accuracyBlack, 'f', 1)));
    }
}

// ---------------------------------------------------------------------------
// paintEvent — dibuja el gráfico de barras con QPainter
// ---------------------------------------------------------------------------
void StatisticsView::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    if (m_history.isEmpty()) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Calcular el área del gráfico (debajo del chartTitle label)
    const int chartTop = m_chartTitle->geometry().bottom() + 4;
    QRect chartRect(CHART_MARGIN, chartTop,
                    width() - 2 * CHART_MARGIN, CHART_HEIGHT);

    drawAccuracyChart(painter, chartRect);
}

void StatisticsView::drawAccuracyChart(QPainter& painter, const QRect& rect) const
{
    // Fondo
    painter.fillRect(rect, QColor(248, 248, 248));
    painter.setPen(QPen(QColor(180, 180, 180)));
    painter.drawRect(rect);

    // Líneas de cuadrícula horizontales cada 25 %
    QFont smallFont;
    smallFont.setPointSize(8);
    painter.setFont(smallFont);

    for (int pct : {25, 50, 75, 100}) {
        int y = rect.bottom() - static_cast<int>(pct / 100.0 * rect.height());
        painter.setPen(QPen(QColor(200, 200, 200), 1, Qt::DashLine));
        painter.drawLine(rect.left(), y, rect.right(), y);
        painter.setPen(QPen(Qt::darkGray));
        painter.drawText(2, y + 4, QString::number(pct) + "%");
    }

    if (m_history.isEmpty()) return;

    const int   n         = m_history.size();
    const double slotW    = static_cast<double>(rect.width()) / n;
    const double barW     = qMax(3.0, slotW * 0.35);

    for (int i = 0; i < n; ++i) {
        const double cx = rect.left() + slotW * i + slotW * 0.5;

        // Barra blancas (azul)
        const int hW = static_cast<int>(m_history[i].accuracyWhite / 100.0 * rect.height());
        const QRect barW_rect(
            static_cast<int>(cx - barW - 1), rect.bottom() - hW,
            static_cast<int>(barW), hW);
        painter.fillRect(barW_rect, QColor(70, 130, 200, 190));

        // Barra negras (gris oscuro)
        const int hB = static_cast<int>(m_history[i].accuracyBlack / 100.0 * rect.height());
        const QRect barB_rect(
            static_cast<int>(cx + 1), rect.bottom() - hB,
            static_cast<int>(barW), hB);
        painter.fillRect(barB_rect, QColor(50, 50, 50, 190));
    }

    // Leyenda
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(70, 130, 200, 190));
    painter.drawRect(rect.right() - 130, rect.top() + 8, 12, 12);
    painter.setBrush(QColor(50, 50, 50, 190));
    painter.drawRect(rect.right() - 130, rect.top() + 26, 12, 12);

    painter.setPen(Qt::black);
    painter.drawText(rect.right() - 115, rect.top() + 19, "Blancas");
    painter.drawText(rect.right() - 115, rect.top() + 37, "Negras");
}

// ---------------------------------------------------------------------------
// Slot: click en fila de la tabla
// ---------------------------------------------------------------------------
void StatisticsView::onTableRowClicked(int row, int /*column*/)
{
    if (row >= 0 && row < m_history.size()) {
        emit matchSelected(m_history[row].matchId);
    }
}
