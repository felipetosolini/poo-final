#include "analysisservice.h"
#include "stockfishengine.h"
#include <QTimer>
#include <cmath>

AnalysisService::AnalysisService(StockfishEngine* engine, QObject *parent)
    : QObject(parent)
    , m_engine(engine)
{
    connect(m_engine, &StockfishEngine::evalUpdated,
            this, &AnalysisService::onEvalUpdated);
    connect(m_engine, &StockfishEngine::bestMoveFound,
            this, &AnalysisService::onBestMoveFound);
    connect(m_engine, &StockfishEngine::pvUpdated,
            this, &AnalysisService::onPvUpdated);
}

void AnalysisService::analyzeGame(const std::vector<chess::Board>& boards,
                                   const std::vector<chess::Move>& moves,
                                   int depth)
{
    if (m_running || boards.empty())
        return;

    m_boards    = boards;
    m_moves     = moves;
    m_depth     = depth;
    m_currentIndex = 0;
    m_running   = true;
    m_cancelled = false;
    m_results.clear();

    m_engine->newGame();
    analyzeNext();
}

void AnalysisService::cancelAnalysis()
{
    if (!m_running)
        return;
    m_cancelled = true;
    m_engine->stopAnalysis();
}

// --- slots del motor ---

void AnalysisService::onEvalUpdated(int cp)
{
    if (!m_running) return;
    m_bestEval = cp;
}

void AnalysisService::onPvUpdated(QStringList pv)
{
    if (!m_running) return;
    m_pv = pv;
    if (!pv.isEmpty())
        m_bestMove = pv.first();
}

void AnalysisService::onBestMoveFound(QString move)
{
    if (!m_running) return;
    if (m_bestMove.isEmpty())
        m_bestMove = move;
    onPositionDone();
}

// --- lógica interna ---

void AnalysisService::analyzeNext()
{
    if (!m_running) return;

    if (m_cancelled || m_currentIndex >= static_cast<int>(m_boards.size())) {
        m_running = false;
        if (m_cancelled) {
            emit analysisCancelled();
        } else {
            m_accuracy = computeAccuracy(m_results, static_cast<int>(m_moves.size()));
            emit analysisComplete(m_results, m_accuracy);
        }
        return;
    }

    // Limpiar estado de esta posición
    m_bestEval = 0;
    m_bestMove.clear();
    m_pv.clear();

    QString fen = m_boards[m_currentIndex].toFen();
    m_currentFen = fen;

    // Validar que el FEN tenga ambos reyes antes de enviarlo a Stockfish.
    // Un FEN sin rey crashea el proceso de Stockfish.
    auto fenHasBothKings = [](const QString& f) {
        int space = f.indexOf(' ');
        QString pos = space >= 0 ? f.left(space) : f;
        return pos.contains('K') && pos.contains('k');
    };
    if (!fenHasBothKings(fen)) {
        // Posición inválida — saltar sin analizar
        m_currentIndex++;
        QTimer::singleShot(0, this, &AnalysisService::analyzeNext);
        return;
    }

    m_engine->setPosition(fen);
    m_engine->analyze(m_depth);

    emit progressUpdated(m_currentIndex, static_cast<int>(m_boards.size()));
}

void AnalysisService::onPositionDone()
{
    if (m_cancelled) {
        m_running = false;
        emit analysisCancelled();
        return;
    }

    int idx = m_currentIndex;

    // evalBefore = evaluación de la posición actual (qué podría haber hecho)
    // Para calcular el delta necesitamos también la evaluación de la posición
    // DESPUÉS del movimiento jugado. Usamos el evalBefore del siguiente paso.
    // Estrategia: guardamos provisional y completamos cuando tengamos el siguiente.
    MoveAnalysis ma;
    ma.moveIndex  = idx;
    ma.evalBefore = m_bestEval;
    ma.fen        = m_currentFen;
    ma.bestMove   = m_bestMove;
    ma.pv         = m_pv;

    // Si hay un resultado previo, podemos calcular su delta ahora:
    // delta = evalBefore_actual - evalAfter_anterior (desde perspectiva del jugador)
    if (!m_results.isEmpty()) {
        MoveAnalysis& prev = m_results.last();
        // La evaluación del motor está siempre desde la perspectiva de las blancas.
        // Cuando las negras mueven, un valor más negativo es peor para ellas.
        // El turno de quien movió está implícito en el índice (par = blancas, impar = negras).
        bool whiteJustMoved = (idx % 2 == 1); // blancas mueven en posiciones de índice par,
                                               // así que si estamos en idx impar, blancas acaban de mover.
        int evalAfterMove = m_bestEval;
        prev.evalAfter = evalAfterMove;

        int delta;
        if (whiteJustMoved) {
            // Blancas querían maximizar; delta = bestPossible - actualOutcome
            delta = prev.evalBefore - evalAfterMove;
        } else {
            // Negras querían minimizar; delta = actualOutcome - bestPossible
            delta = evalAfterMove - prev.evalBefore;
        }
        prev.delta = qMax(0, delta);
        prev.classification = classify(prev.delta);
    }

    m_results.append(ma);
    emit moveAnalyzed(idx, ma);

    m_currentIndex++;
    // Defer to next event loop tick so we're not writing to Stockfish's stdin
    // from within the readyReadStandardOutput handler (avoids ordering issues on Windows).
    QTimer::singleShot(0, this, &AnalysisService::analyzeNext);
}

MoveClassification AnalysisService::classify(int delta) const
{
    if (delta == 0)   return MoveClassification::Best;
    if (delta <= 10)  return MoveClassification::Excellent;
    if (delta <= 25)  return MoveClassification::Good;
    if (delta <= 100) return MoveClassification::Inaccuracy;
    if (delta <= 300) return MoveClassification::Mistake;
    return MoveClassification::Blunder;
}

AccuracyScore AnalysisService::computeAccuracy(const QVector<MoveAnalysis>& results,
                                                int /*totalMoves*/) const
{
    // Usamos la fórmula: accuracy = 100 * exp(-0.003 * avgDelta)
    // (derivada del modelo de lichess; da ~100 para delta=0, ~74 para delta=100)
    auto phaseFor = [](int idx) -> int {
        int moveNum = idx / 2 + 1; // idx par = blancas, impar = negras
        if (moveNum <= 10) return 0; // apertura
        if (moveNum <= 30) return 1; // medio juego
        return 2;                    // final
    };

    struct PhaseAccum { double sumDelta = 0; int count = 0; };
    PhaseAccum white[3], black[3];
    int wBlunders = 0, bBlunders = 0, wMistakes = 0, bMistakes = 0;

    for (const MoveAnalysis& ma : results) {
        bool isWhite = (ma.moveIndex % 2 == 0);
        int phase = phaseFor(ma.moveIndex);
        PhaseAccum& acc = isWhite ? white[phase] : black[phase];
        acc.sumDelta += ma.delta;
        acc.count++;

        if (ma.classification == MoveClassification::Blunder) {
            isWhite ? wBlunders++ : bBlunders++;
        } else if (ma.classification == MoveClassification::Mistake) {
            isWhite ? wMistakes++ : bMistakes++;
        }
    }

    auto calcAccuracy = [](const PhaseAccum& a) -> double {
        if (a.count == 0) return 100.0;
        double avgDelta = a.sumDelta / a.count;
        return 100.0 * std::exp(-0.003 * avgDelta);
    };

    auto weightedAccuracy = [&](const PhaseAccum phases[3]) -> double {
        double total = 0; int cnt = 0;
        for (int i = 0; i < 3; ++i) { total += phases[i].sumDelta; cnt += phases[i].count; }
        if (cnt == 0) return 100.0;
        return 100.0 * std::exp(-0.003 * (total / cnt));
    };

    AccuracyScore score;
    score.white          = weightedAccuracy(white);
    score.black          = weightedAccuracy(black);
    score.whiteOpening   = calcAccuracy(white[0]);
    score.whiteMidgame   = calcAccuracy(white[1]);
    score.whiteEndgame   = calcAccuracy(white[2]);
    score.blackOpening   = calcAccuracy(black[0]);
    score.blackMidgame   = calcAccuracy(black[1]);
    score.blackEndgame   = calcAccuracy(black[2]);
    score.whiteBlunders  = wBlunders;
    score.blackBlunders  = bBlunders;
    score.whiteMistakes  = wMistakes;
    score.blackMistakes  = bMistakes;
    return score;
}
