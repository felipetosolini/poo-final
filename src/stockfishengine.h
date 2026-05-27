#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

class StockfishEngine : public QObject {
    Q_OBJECT

public:
    explicit StockfishEngine(QObject *parent = nullptr);
    ~StockfishEngine();

    // Ciclo de vida
    bool start(const QString& executablePath);
    void shutdown();

    // Control de partida
    void newGame();
    void setPosition(const QString& fen);
    void analyze(int depth = 20);
    void analyzeForTime(int milliseconds = 3000);
    void stopAnalysis();

    bool isReady() const { return m_ready; }

signals:
    void engineReady();
    void evalUpdated(int cp);          // centipawns, positivo = ventaja blancas
    void mateDetected(int movesToMate); // positivo = blancas dan mate, negativo = negras
    void bestMoveFound(QString move);  // e.g. "e2e4"
    void pvUpdated(QStringList pv);    // línea principal
    void depthReached(int depth);
    void engineError(QString message);

private slots:
    void onReadyRead();
    void onProcessError(QProcess::ProcessError error);

private:
    QProcess *m_process = nullptr;
    bool m_ready = false;
    bool m_awaitingUciOk = false;
    bool m_awaitingReadyOk = false;

    void send(const QString& command);
    void parseLine(const QString& line);
    void parseInfoLine(const QString& line);
};
