# CLAUDE.md — Chess Insight AI

Aplicación de escritorio C++/Qt para análisis de partidas de ajedrez con Stockfish e IA.
TP Integrador de Programación Orientada a Objetos.

---

## Estructura del repo

```
/                        ← proyecto Qt (C++), archivo poo-final.pro
src/                     ← todo el código fuente C++
resources/               ← recursos Qt (.qrc, SVGs de piezas, styles.qss)
bin/                     ← stockfish.exe (NO se commitea, cada uno lo descarga)
backend/                 ← proyecto FastAPI (Python)
  routers/               ← endpoints REST
  schemas/               ← Pydantic models
  models/                ← SQLAlchemy ORM models
  services/              ← lógica de negocio
  core/                  ← config, JWT, dependencias
  main.py
  requirements.txt
  Dockerfile
  .env.example           ← copiar a .env y completar con valores reales
build/                   ← generado por Qt Creator, ignorado en git
docker-compose.yml       ← levanta api + db MySQL
setup-vps.sh             ← script de deploy en el servidor
```

---

## Cómo correr el proyecto

### App Qt (C++)
Abrir `poo-final.pro` en Qt Creator y compilar con el kit MinGW 64-bit.
Requiere Qt 6 con módulos: `core gui widgets network sql printsupport`.

### Backend local
```bash
cp backend/.env.example backend/.env   # completar OPENAI_API_KEY
docker compose up --build -d
curl http://localhost:8000/health       # debe devolver {"status":"ok"}
curl http://localhost:8000/docs        # Swagger UI
```

### Stockfish
Descargar desde https://stockfishchess.org/download/ y colocar en `bin/stockfish.exe`.
No se commitea. El código lo busca en esa ruta relativa al ejecutable.

---

## Arquitectura

```
App Qt (C++)
  ├── BoardWidget + GameManager      ← lógica de UI y navegación
  ├── StockfishEngine (QProcess)     ← proceso local, protocolo UCI por stdin/stdout
  └── HttpClient (QNetworkAccessManager) → Backend

Backend FastAPI + MySQL — Docker en VPS 89.116.29.15
  ├── POST /auth/register  POST /auth/login
  ├── GET/POST /matches
  └── GET /stats/{user_id}

SQLite local (Qt)
  └── Tabla session(token, user_id, username) — persiste el login entre reinicios
```

---

## Integrantes y áreas

| Integrante | Área | Archivos principales |
|---|---|---|
| Felipe Tosolini | Área 1 — GUI Qt y tablero | `src/mainwindow.*`, `src/boardwidget.*`, `src/*.ui` |
| Agustina Revuelta | Área 2 — Lógica OO del juego | `src/piece.*`, `src/board.*`, `src/move.*`, `src/pgnparser.*` |
| Ignacio Nievas | Área 3 — Backend y BD | `backend/`, `docker-compose.yml` |
| Lautaro Robledo | Área 4 — Stockfish y análisis | `src/stockfishengine.*`, `src/analysisservice.*` |
| Facundo Toloza | Área 5 — IA, estadísticas y PDF | `src/aiexplanationservice.*`, `src/statisticsview.*` |

---

## Convenciones C++ / Qt

- Todo el núcleo lógico va dentro de `namespace chess { }` — clases `Board`, `Piece`, `Move`, `Game`, `Player`, `PGNParser`
- Las clases Qt (`GameManager`, widgets) van fuera del namespace porque heredan de `QObject`/`QWidget`
- `const` en todos los métodos que no modifican el objeto: `QString name() const`
- Getters simples van `inline` en el header
- Punteros inteligentes para piezas: `std::vector<std::unique_ptr<chess::Piece>>`
- La matriz interna del tablero usa aritmética de punteros: `Piece* board[8][8]`
- Nombres de archivos en minúsculas con el nombre de la clase: `boardwidget.h`, `boardwidget.cpp`
- Un `.ui` por ventana/panel, cargado con `setupUi(this)` en el constructor

### Jerarquía de clases obligatoria (POO)
```
chess::Piece  ← clase abstracta, métodos virtuales puros
  ├── chess::Pawn
  ├── chess::Knight
  ├── chess::Bishop
  ├── chess::Rook
  ├── chess::Queen
  └── chess::King

GameManager : QObject         ← emite signals, no va en namespace chess
BoardWidget : QWidget         ← sobreescribe paintEvent, mousePressEvent, etc.
StockfishEngine : QObject     ← wrappea QProcess con protocolo UCI
```

### Signals y slots — patrón a seguir
```cpp
// En GameManager:
signals:
    void boardUpdated(chess::Board board);
    void moveNavigated(int index);

// Conexión en MainWindow:
connect(gameManager, &GameManager::boardUpdated,
        boardWidget,  &BoardWidget::onBoardUpdated);
```

No usar `SIGNAL()`/`SLOT()` en string — siempre la forma con puntero a función.

### Template obligatorio
```cpp
// En src/utils.h — usar en PGNParser y AnalysisService
template<typename T, typename Predicate>
std::optional<T> findFirst(const std::vector<T>& vec, Predicate pred) {
    auto it = std::find_if(vec.begin(), vec.end(), pred);
    return it != vec.end() ? std::optional<T>(*it) : std::nullopt;
}
```

---

## Convenciones Python / FastAPI

- Pydantic v2 para schemas (`model_config = ConfigDict(from_attributes=True)`)
- SQLAlchemy 2.0 con `Session` de `sqlalchemy.orm`
- Rutas protegidas reciben `current_user` via `Depends(get_current_user)`
- Errores con `raise HTTPException(status_code=..., detail="...")`
- No usar Alembic — las tablas se crean con `Base.metadata.create_all()` al iniciar

```
routers/auth.py      → POST /auth/register, POST /auth/login
routers/matches.py   → GET /matches, POST /matches
routers/stats.py     → GET /stats/{user_id}
core/security.py     → JWT: create_token(), verify_token()
core/database.py     → engine, SessionLocal, get_db()
models/user.py       → ORM User
models/match.py      → ORM Match, Analysis
```

---

## Protocolo UCI con Stockfish

Toda la comunicación es por texto a través de `QProcess` stdin/stdout:

```
→ uci                    ← esperar "uciok"
→ isready                ← esperar "readyok"
→ ucinewgame
→ position fen <fen>
→ go depth 18            ← parsear líneas "info ..." hasta "bestmove <mv>"
→ quit                   ← al destruir StockfishEngine
```

`StockfishEngine` emite: `evalUpdated(int cp)`, `bestMoveFound(QString)`, `pvUpdated(QStringList)`

La evaluación en centipawns: positivo = ventaja blancas, negar para negras.

---

## Clasificación de movimientos (AnalysisService)

| Categoría | Delta cp vs mejor jugada |
|---|---|
| Best / Excellent | ≤ 10 |
| Good | ≤ 25 |
| Inaccuracy | 26 – 100 |
| Mistake | 101 – 300 |
| Blunder | > 300 |

---

## Variables de entorno (backend/.env)

```
DB_URL=mysql+pymysql://chessuser:chesspassword@db:3306/chessdb
JWT_SECRET=<string largo y aleatorio>
OPENAI_API_KEY=sk-...
```

En la app Qt la API key de OpenAI se lee desde una config local o variable de entorno, nunca se hardcodea ni se commitea.

---

## Criterios de evaluación

| Área | Pts | Qué cubre |
|---|---|---|
| POO | 30 | Clases abstractas, herencia, polimorfismo, templates, namespace, const/inline, punteros |
| GUI Qt | 35 | `.ui` files, QPainter, signals/slots, eventos mouse/teclado, QTimer, drag & drop |
| Tecnologías externas | 15 | FastAPI + MySQL, SQLite local, JWT, Docker, deploy VPS, QtNetwork |
| Valor agregado | 20 | OpenAI API, Accuracy Score, EvaluationBar animada, exportación PDF |

**Cada concepto de POO tiene que estar visible en el código.** Si no se ve, no suma.

---

## Reglas generales

- No pushear `backend/.env`, `bin/stockfish.exe`, ni carpeta `build/` — están en `.gitignore`
- No hardcodear la IP del servidor en el código — usar una constante en `src/config.h`
- No agregar dependencias externas de C++ — solo Qt y stdlib
- El backend no tiene lógica de análisis de ajedrez — eso lo hace Stockfish localmente en la app Qt
- Los archivos `.ui` se editan con Qt Designer, no a mano
- Antes de mergear a main, verificar que la app compila limpio (sin warnings)
