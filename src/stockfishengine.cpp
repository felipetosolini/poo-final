#include "stockfishengine.h"
#include <QStringList>

StockfishEngine::StockfishEngine(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this))
{
    m_process->setReadChannel(QProcess::StandardOutput);

    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &StockfishEngine::onReadyRead);
    connect(m_process, &QProcess::errorOccurred,
            this, &StockfishEngine::onProcessError);
}

StockfishEngine::~StockfishEngine()
{
    shutdown();
}

bool StockfishEngine::start(const QString& executablePath)
{
    if (m_process->state() != QProcess::NotRunning)
        return true;

    m_process->start(executablePath, QStringList());
    if (!m_process->waitForStarted(3000)) {
        emit engineError("No se pudo iniciar Stockfish en: " + executablePath);
        return false;
    }

    // Handshake UCI
    m_awaitingUciOk = true;
    send("uci");
    return true;
}

void StockfishEngine::shutdown()
{
    if (m_process->state() == QProcess::Running) {
        send("quit");
        m_process->waitForFinished(2000);
        if (m_process->state() == QProcess::Running)
            m_process->kill();
    }
    m_ready = false;
}

void StockfishEngine::newGame()
{
    send("ucinewgame");
}

void StockfishEngine::setPosition(const QString& fen)
{
    send("position fen " + fen);
}

void StockfishEngine::analyze(int depth)
{
    send("go depth " + QString::number(depth));
}

void StockfishEngine::analyzeForTime(int milliseconds)
{
    send("go movetime " + QString::number(milliseconds));
}

void StockfishEngine::stopAnalysis()
{
    send("stop");
}

// --- private ---

void StockfishEngine::send(const QString& command)
{
    if (m_process->state() != QProcess::Running)
        return;
    m_process->write((command + "\n").toUtf8());
}

void StockfishEngine::onReadyRead()
{
    while (m_process->canReadLine()) {
        QString line = QString::fromUtf8(m_process->readLine()).trimmed();
        if (!line.isEmpty())
            parseLine(line);
    }
}

void StockfishEngine::parseLine(const QString& line)
{
    if (m_awaitingUciOk && line == "uciok") {
        m_awaitingUciOk = false;
        // Configurar opciones y pedir readyok
        send("setoption name Threads value 4");
        send("setoption name Hash value 256");
        send("setoption name Skill Level value 20");
        m_awaitingReadyOk = true;
        send("isready");
        return;
    }

    if (m_awaitingReadyOk && line == "readyok") {
        m_awaitingReadyOk = false;
        m_ready = true;
        emit engineReady();
        return;
    }

    if (line.startsWith("info ")) {
        parseInfoLine(line);
        return;
    }

    if (line.startsWith("bestmove ")) {
        // "bestmove e2e4 ponder e7e5" — solo nos interesa la primera jugada
        QStringList parts = line.split(' ');
        if (parts.size() >= 2)
            emit bestMoveFound(parts[1]);
        return;
    }
}

void StockfishEngine::parseInfoLine(const QString& line)
{
    QStringList tokens = line.split(' ');
    int depth = 0;
    bool hasScore = false;

    for (int i = 0; i < tokens.size(); ++i) {
        const QString& tok = tokens[i];

        if (tok == "depth" && i + 1 < tokens.size()) {
            depth = tokens[i + 1].toInt();
            emit depthReached(depth);
        }
        else if (tok == "score" && i + 2 < tokens.size()) {
            const QString& scoreType = tokens[i + 1];
            int value = tokens[i + 2].toInt();

            if (scoreType == "cp") {
                hasScore = true;
                emit evalUpdated(value);
            }
            else if (scoreType == "mate") {
                hasScore = true;
                emit mateDetected(value);
                // Convertir a centipawns extremos para la barra
                emit evalUpdated(value > 0 ? 30000 : -30000);
            }
        }
        else if (tok == "pv" && i + 1 < tokens.size()) {
            QStringList pv = tokens.mid(i + 1);
            emit pvUpdated(pv);
        }
    }

    Q_UNUSED(hasScore)
}

void StockfishEngine::onProcessError(QProcess::ProcessError error)
{
    Q_UNUSED(error)
    emit engineError("Error en proceso Stockfish: " + m_process->errorString());
}
