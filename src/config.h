#pragma once
#include <QString>

namespace Config {

    // API Backend — apunta al VPS en producción
    inline const QString API_BASE_URL = "http://89.116.29.15:8000";
    constexpr const char* API_HEALTH_ENDPOINT = "/health";
    constexpr const char* API_AUTH_LOGIN     = "/auth/login";
    constexpr const char* API_AUTH_REGISTER  = "/auth/register";
    constexpr const char* API_MATCHES        = "/matches";
    constexpr const char* API_STATS          = "/stats";

    // Stockfish
    constexpr const char* STOCKFISH_PATH  = "bin/stockfish.exe";
    constexpr int         STOCKFISH_DEPTH = 18;

    // UI
    constexpr int DEFAULT_BOARD_SIZE = 800;
    constexpr int PLAYBACK_SPEED_MS  = 1000;

    // Base de datos local
    constexpr const char* DB_PATH = "chess_session.db";

    // OpenAI API — se carga desde variable de entorno, nunca hardcodeada
    extern QString OPENAI_API_KEY;

} // namespace Config
