#include "aiexplanationservice.h"
#include "config.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QTimer>
#include <QProcessEnvironment>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>

static QString loadKeyFromEnvFile()
{
    // Busca backend/.env relativo al ejecutable
    // build/Desktop_Qt_.../debug/ → ../../.. → poo-final/
    const QStringList candidates = {
        QCoreApplication::applicationDirPath() + "/../../../backend/.env",
        QCoreApplication::applicationDirPath() + "/../../backend/.env",
        QCoreApplication::applicationDirPath() + "/../backend/.env",
        QCoreApplication::applicationDirPath() + "/backend/.env",
    };
    for (const QString& path : candidates) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;
        QTextStream in(&file);
        while (!in.atEnd()) {
            const QString line = in.readLine().trimmed();
            if (line.startsWith("OPENAI_API_KEY="))
                return line.mid(QString("OPENAI_API_KEY=").length()).trimmed();
        }
    }
    return {};
}

AIExplanationService::AIExplanationService(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{
    if (Config::OPENAI_API_KEY.isEmpty())
        Config::OPENAI_API_KEY =
            QProcessEnvironment::systemEnvironment().value("OPENAI_API_KEY");

    if (Config::OPENAI_API_KEY.isEmpty())
        Config::OPENAI_API_KEY = loadKeyFromEnvFile();

}

void AIExplanationService::requestExplanation(int moveIndex,
                                               const QString& fen,
                                               const QString& playedMove,
                                               const QString& bestMove,
                                               int evalBefore,
                                               int evalAfter,
                                               MoveClassification classification)
{
    sendRequest(moveIndex, fen, playedMove, bestMove, evalBefore, evalAfter, classification);
}

void AIExplanationService::sendRequest(int moveIndex,
                                        const QString& fen,
                                        const QString& playedMove,
                                        const QString& bestMove,
                                        int evalBefore,
                                        int evalAfter,
                                        MoveClassification classification,
                                        int retryCount)
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

    QNetworkReply* reply = m_manager->post(
        request, QJsonDocument(body).toJson(QJsonDocument::Compact));

    // Cada reply lleva su propio contexto capturado en la lambda.
    // No hay estado compartido, por lo que múltiples requests simultáneos
    // no se interfieren entre sí.
    connect(reply, &QNetworkReply::finished, this,
            [this, reply, moveIndex, fen, playedMove, bestMove,
             evalBefore, evalAfter, classification, retryCount]() {
        reply->deleteLater();

        const int httpStatus =
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        // HTTP 429 — rate limit: reintentar una sola vez tras 2 segundos
        if (httpStatus == 429 && retryCount < 1) {
            QTimer::singleShot(2000, this, [this, moveIndex, fen, playedMove,
                                             bestMove, evalBefore, evalAfter,
                                             classification]() {
                sendRequest(moveIndex, fen, playedMove, bestMove,
                            evalBefore, evalAfter, classification, 1);
            });
            return;
        }

        if (reply->error() != QNetworkReply::NoError) {
            emit requestFailed(moveIndex, reply->errorString());
            return;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (doc.isNull() || !doc.isObject()) {
            emit requestFailed(moveIndex, "Invalid JSON response from OpenAI");
            return;
        }

        const QString explanation = doc.object()
                                        .value("choices").toArray()
                                        .first().toObject()
                                        .value("message").toObject()
                                        .value("content").toString()
                                        .trimmed();

        if (explanation.isEmpty()) {
            emit requestFailed(moveIndex, "Empty response from OpenAI");
            return;
        }

        emit explanationReady(moveIndex, explanation);
    });
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
