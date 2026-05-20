#pragma once
#include <QString>

namespace Config {

// URL base del backend. En desarrollo local: "http://localhost:8000"
// En producción apunta al VPS donde corre docker compose.
inline const QString API_BASE_URL = "http://89.116.29.15:8000";

inline const QString STOCKFISH_PATH = "bin/stockfish.exe";

} // namespace Config
