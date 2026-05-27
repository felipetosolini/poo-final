#include "aiexplanationservice.h"
#include "config.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QTimer>
#include <QProcessEnvironment>

AIExplanationService::AIExplanationService(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{
    // Cargar la API key desde variable de entorno si no está configurada todavía
    if (Config::OPENAI_API_KEY.isEmpty()) {
        Config::OPENAI_API_KEY =
            QProcessEnvironment::systemEnvironment().value("OPENAI_API_KEY");
    }

    connect(m_manager, &QNetworkAccessManager::finished,
            this,      &AIExplanationService::onReplyFinished);
}

void AIExplanationService::requestExplanation(int moveIndex,
                                               const QString& fen,
                                               const QString& playedMove,
                                               const QString& bestMove,
                                               int evalBefore,
                                               int evalAfter,
                                               MoveClassification classification)
{
    m_retryCount         = 0;
    m_pendingMoveIndex   = moveIndex;
    m_lastFen            = fen;
    m_lastPlayedMove     = playedMove;
    m_lastBestMove       = bestMove;
    m_lastEvalBefore     = evalBefore;
    m_lastEvalAfter      = evalAfter;
    m_lastClassification = classification;

    sendRequest(moveIndex, fen, playedMove, bestMove, evalBefore, evalAfter, classification);
}

void AIExplanationService::sendRequest(int moveIndex,
                                        const QString& fen,
                                        const QString& playedMove,
                                        const QString& bestMove,
                                        int evalBefore,
                                        int evalAfter,
                                        MoveClassification classification)
{
    if (Config::OPENAI_API_KEY.isEmpty()) {
        emit requestFailed(moveIndex, "OpenAI API key not configured. "
                                      "Set the OPENAI_API_KEY environment variable.");
        return;
    }

    QNetworkRequest request(QUrl("https://api.openai.com/v1/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization",
                         ("Bearer " + Config::OPENAI_API_KEY).toUtf8());

    QJsonObject messageObj;
    messageObj["role"]    = "user";
    messageObj["content"] = buildPrompt(fen, playedMove, bestMove,
                                        evalBefore, evalAfter, classification);

    QJsonObject body;
    body["model"]      = "gpt-4o-mini";
    body["max_tokens"] = 200;
    body["messages"]   = QJsonArray{ messageObj };

    m_manager->post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));
}

void AIExplanationService::onReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();

    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    // HTTP 429 — rate limit: reintentar una sola vez tras 2 segundos
    if (httpStatus == 429 && m_retryCount < 1) {
        ++m_retryCount;
        QTimer::singleShot(2000, this, [this]() {
            sendRequest(m_pendingMoveIndex,
                        m_lastFen, m_lastPlayedMove, m_lastBestMove,
                        m_lastEvalBefore, m_lastEvalAfter, m_lastClassification);
        });
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        emit requestFailed(m_pendingMoveIndex, reply->errorString());
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (doc.isNull() || !doc.isObject()) {
        emit requestFailed(m_pendingMoveIndex, "Invalid JSON response from OpenAI");
        return;
    }

    const QString explanation = doc.object()
                                    .value("choices").toArray()
                                    .first().toObject()
                                    .value("message").toObject()
                                    .value("content").toString()
                                    .trimmed();

    if (explanation.isEmpty()) {
        emit requestFailed(m_pendingMoveIndex, "Empty response from OpenAI");
        return;
    }

    emit explanationReady(m_pendingMoveIndex, explanation);
}

QString AIExplanationService::buildPrompt(const QString& fen,
                                          const QString& playedMove,
                                          const QString& bestMove,
                                          int evalBefore,
                                          int evalAfter,
                                          MoveClassification classification) const
{
    const double evalBeforePawns = evalBefore / 100.0;
    const double evalAfterPawns  = evalAfter  / 100.0;
    const QString classStr       = classificationToString(classification);

    return QString(
        "You are a chess coach. Briefly explain in 2-3 sentences (no markdown, no bullet "
        "points) why the move played was classified as '%1'.\n\n"
        "Position (FEN): %2\n"
        "Move played: %3\n"
        "Best move: %4\n"
        "Evaluation before: %5 pawns\n"
        "Evaluation after:  %6 pawns\n\n"
        "Focus on the strategic or tactical reason for the classification. "
        "Keep the explanation concise and beginner-friendly."
    ).arg(classStr,
          fen,
          playedMove,
          bestMove,
          QString::number(evalBeforePawns, 'f', 2),
          QString::number(evalAfterPawns,  'f', 2));
}
