#include "config.h"

namespace Config {
    // Definición de la API key de OpenAI.
    // Se carga desde variable de entorno; nunca se hardcodea ni se commitea.
    QString OPENAI_API_KEY;
}
