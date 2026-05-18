# Chess Insight AI — Planes de Trabajo

> Criterios de evaluación: POO 30 pts · GUI Qt 35 pts · Tecnologías externas 15 pts · Valor agregado 20 pts

---

## Setup Inicial — Hacer antes de que el equipo arranque

### Repositorio y estructura de carpetas
- [x] Crear repo en GitHub e invitar a los 5 integrantes
- [x] Crear estructura de carpetas del repo:
  ```
  /                  ← proyecto Qt (C++)
  backend/           ← proyecto FastAPI
  bin/               ← stockfish.exe va acá (no se commitea)
  ```
- [x] Configurar `.gitignore`: ignorar `bin/`, `build/`, `.env`, `*.user`, `.qtcreator/`

### Proyecto Qt
- [x] Verificar que el archivo `poo-final.pro` tenga los módulos necesarios:
  ```
  QT += core gui widgets network sql printsupport
  ```
- [x] Crear `main.cpp` con `QApplication` y apertura de `MainWindow` básica (app que abre y cierra)
- [x] Verificar que compila y corre vacío en Qt Creator

### Stockfish
- [x] Descargar binario desde https://stockfishchess.org/download/ (Windows, versión sin AVX512)
- [x] Colocar el ejecutable en `bin/stockfish.exe`
- [x] Verificar que corre: abrir una terminal, ejecutar `bin\stockfish.exe`, escribir `uci` y confirmar que responde `uciok`

### Backend
- [x] Crear carpeta `backend/` con estructura:
  ```
  backend/
    routers/
    schemas/
    models/
    services/
    core/
    main.py
    requirements.txt
    Dockerfile
    .env.example
  ```
- [x] Crear `requirements.txt` con dependencias base: `fastapi`, `uvicorn`, `sqlalchemy`, `pymysql`, `python-jose`, `bcrypt`
- [x] Crear `main.py` con app FastAPI vacía y ruta `GET /health` que devuelva `{"status": "ok"}`
- [x] Crear `.env.example` con variables: `DB_URL`, `JWT_SECRET`, `OPENAI_API_KEY`

### Docker
- [x] Crear `backend/Dockerfile` (imagen base `python:3.11-slim`, instala requirements, expone puerto 8000)
- [x] Crear `docker-compose.yml` en raíz del repo con servicios `api` (FastAPI) y `db` (MySQL 8)
- [ ] Levantar localmente con `docker compose up --build` y verificar que `GET /health` responde en `localhost:8000`

### Servidor VPS
> `ssh root@89.116.29.15` — contraseña en canal privado del grupo
> ⚠️ Pendiente: SSH tira "Permission denied" — usar consola VNC de Contabo o habilitar PasswordAuthentication en sshd_config. Script de setup listo en `setup-vps.sh`.

- [ ] Conectarse al servidor y verificar acceso SSH (bloqueado — ver nota)
- [ ] Instalar Docker y Docker Compose en el servidor
- [ ] Clonar el repo: `git clone https://github.com/felipetosolini/poo-final.git`
- [ ] Crear `.env` de producción en el servidor (no commitear)
- [ ] Levantar contenedores: `docker compose up -d`
- [ ] Verificar que la API responde en `http://89.116.29.15:8000/health`

---

## Área 1 — Interfaz Gráfica y Tablero Qt

> Cubre: Qt Designer + archivos `.ui`, QPainter, signals/slots, eventos mouse/teclado, QTimer, drag & drop, manejo de errores en UI.

### Archivos .ui y estructura de ventanas
- [ ] Crear `mainwindow.ui` en Qt Designer: layout principal con panel central (tablero), panel lateral izquierdo (piezas capturadas, evaluation bar) y panel derecho (timeline, análisis)
- [ ] Crear `loginwindow.ui`: formulario de login con campos usuario/contraseña, botón login y link a registro
- [ ] Crear `registerwindow.ui`: formulario de registro con validación visual
- [ ] Crear `analysissidebar.ui`: panel con pestañas para análisis del motor y explicación IA
- [ ] Crear `statisticsview.ui`: vista de historial y gráficos
- [ ] Cargar y promover cada `.ui` en su clase C++ correspondiente con `setupUi(this)`

### BoardWidget (QPainter + eventos)
- [ ] Implementar `BoardWidget` custom heredando `QWidget`, sobreescribir `paintEvent()` con `QPainter`
- [ ] Dibujar tablero 8x8 con colores alternados y coordenadas (a-h, 1-8)
- [ ] Cargar piezas como `QPixmap` desde recursos (SVG/PNG) por tipo y color
- [ ] Renderizar piezas en posición correcta según estado del `Board`
- [ ] Resaltar casilla seleccionada, últimos movimientos y movimientos válidos (overlay semitransparente)
- [ ] Sobreescribir `mousePressEvent()` y `mouseReleaseEvent()` para selección y movimiento por click
- [ ] Implementar drag & drop: `mouseMoveEvent()` + `QDrag` + `QMimeData`
- [ ] Sobreescribir `keyPressEvent()` para flechas de navegación (← anterior, → siguiente)
- [ ] Conectar señal propia `moveRequested(Move)` al `GameManager` vía signal/slot

### Navegación y timeline
- [ ] Implementar controles de navegación (botones: |← ← → →|) conectados por signals/slots al `GameManager`
- [ ] Crear `MoveListWidget` (QListWidget) con movimientos en notación algebraica
- [ ] Resaltar ítem activo en `MoveListWidget` al navegar
- [ ] Conectar click en ítem de `MoveListWidget` a salto directo de jugada
- [ ] Usar `QTimer` para reproducción automática (Play/Pause con intervalo configurable)

### Panel de piezas capturadas y evaluation bar
- [ ] Implementar `CapturedPiecesWidget`: mostrar piezas capturadas agrupadas por tipo con `QPainter`
- [ ] Implementar `EvaluationBarWidget`: barra vertical con `QPainter`, animada con `QPropertyAnimation`

### Manejo de errores en UI
- [ ] Mostrar `QMessageBox` con mensaje descriptivo ante: archivo PGN inválido, fallo de red, sesión expirada
- [ ] Deshabilitar controles durante operaciones asincrónicas, mostrar `QProgressBar` o spinner
- [ ] Validar campos vacíos en formularios antes de enviar petición HTTP

### UX / Pulido
- [ ] Registrar atajos de teclado con `QShortcut`: Ctrl+O (abrir PGN), Ctrl+E (exportar PDF), flechas (navegar)
- [ ] Aplicar hoja de estilos QSS global desde archivo `styles.qss` cargado en `main.cpp`
- [ ] Asegurar que la ventana sea redimensionable con layouts correctos (no posiciones fijas)

---

## Área 2 — Lógica del Juego y Modelado OO

> Cubre: namespace, inline/const, std::vector/string, punteros, funciones genéricas (templates), herencia, polimorfismo, funciones virtuales puras, clases abstractas, friend, encapsulamiento.

### Namespace y convenciones base
- [ ] Definir namespace `chess` para todas las clases del núcleo lógico
- [ ] Usar `const` en todos los métodos de solo lectura y parámetros que no se modifican
- [ ] Usar `inline` para getters/setters simples en los headers

### Clase abstracta `Piece` y jerarquía de herencia
- [ ] Definir clase abstracta `Piece` con métodos virtuales puros: `virtual std::string getSymbol() const = 0` y `virtual PieceType getType() const = 0`
- [ ] Declarar `virtual ~Piece()` en `Piece`
- [ ] Implementar subclases concretas heredando de `Piece`: `Pawn`, `Knight`, `Bishop`, `Rook`, `Queen`, `King`
- [ ] Sobreescribir `getSymbol()` y `getType()` en cada subclase (polimorfismo para rendering y lógica)
- [ ] Usar `std::vector<std::unique_ptr<Piece>>` para manejar las piezas del tablero

### Clase `Move`
- [ ] Implementar `Move`: campos `from`, `to` (como índices o struct `Square`), puntero a pieza, flag de captura, promoción, enroque, al paso
- [ ] Sobrecargar `operator==` para comparar movimientos
- [ ] Implementar `std::string toAlgebraic() const` y `std::string toUCI() const` (formato `e2e4`)

### Clase `Board`
- [ ] Implementar `Board`: matriz interna `Piece* board[8][8]` con aritmética de punteros para acceso
- [ ] Métodos: `getPiece(int row, int col)`, `setPiece(...)`, `isOccupied(...)`, `isInBounds(...)`
- [ ] Implementar `Board(const Board&)` (copia profunda) para clonar posiciones sin afectar el estado original
- [ ] Implementar `std::string toFEN() const` para exportar posición a formato FEN (requerido por Stockfish)

### Clases `Player` y `Game`
- [ ] Implementar `Player`: nombre (`std::string`), color, estadísticas acumuladas
- [ ] Implementar `Game`: `std::vector<Move>` de jugadas, referencia a jugadores, resultado, metadata PGN

### `PGNParser`
- [ ] Implementar `PGNParser`: leer archivo `.pgn` con `std::ifstream`
- [ ] Extraer headers (STR tags: Event, Site, Date, White, Black, Result)
- [ ] Tokenizar y convertir notación algebraica estándar (SAN) a objetos `Move`
- [ ] Ignorar variantes `(...)` y comentarios `{...}` — parsear solo la línea principal
- [ ] Aplicar cada movimiento sobre `Board` directamente (los movimientos del PGN ya son legales, no hay que validarlos)

### Funciones genéricas (templates)
- [ ] Implementar función template `std::optional<T> findFirst(const std::vector<T>&, Predicate)` para búsquedas en colecciones de piezas/movimientos
- [ ] Usar en `PGNParser` y `AnalysisService`

### `GameManager`
- [ ] Implementar `GameManager` heredando de `QObject` para poder emitir signals
- [ ] Mantener `currentMoveIndex` y `std::vector<Move>` de la partida activa
- [ ] Signals: `boardUpdated(Board)`, `moveNavigated(int index)`, `gameLoaded(Game)`
- [ ] Slots: `nextMove()`, `prevMove()`, `jumpToMove(int)`, `loadPGN(QString path)`

---

## Área 3 — Backend, Autenticación y Base de Datos

> Cubre: FastAPI con endpoints REST, MySQL con SELECT/INSERT, SQLite local, JWT, Docker + Docker Compose, AWS EC2, QtNetwork.

### Backend FastAPI
- [ ] Crear proyecto FastAPI con estructura: `routers/`, `schemas/`, `models/`, `services/`, `core/`
- [ ] `POST /auth/register` — hashear contraseña con bcrypt, INSERT en tabla `users`
- [ ] `POST /auth/login` — verificar hash, retornar JWT firmado
- [ ] `GET /matches` — SELECT partidas del usuario autenticado (filtrar por `user_id` del token)
- [ ] `POST /matches` — INSERT partida analizada con metadata y resultado
- [ ] `GET /stats/{user_id}` — SELECT agregado: precisión promedio, conteo de blunders, etc.
- [ ] Middleware de autenticación JWT usando `python-jose` en rutas protegidas
- [ ] Manejo de errores HTTP con `HTTPException` y respuestas JSON estructuradas
- [ ] Documentación automática disponible en `/docs` (Swagger UI)

### Base de datos MySQL
- [ ] Diseñar esquema relacional:
  - `users(id, username, email, password_hash, created_at)`
  - `matches(id, user_id, pgn, result, played_at, accuracy_white, accuracy_black)`
  - `analysis(id, match_id, move_number, eval_cp, eval_type, best_move, classification)`
- [ ] Integrar SQLAlchemy con modelos ORM para cada tabla
- [ ] Crear tablas con `Base.metadata.create_all()` al iniciar el servidor (sin Alembic)
- [ ] Implementar `UserRepository`, `MatchRepository`, `StatisticsRepository` con métodos SELECT e INSERT

### Persistencia local SQLite (Qt)
- [ ] Usar `QSqlDatabase` con driver SQLite en el cliente Qt
- [ ] Crear tabla `session(token, user_id, username)` para persistir la sesión entre reinicios
- [ ] Guardar token JWT y datos del usuario al hacer login exitoso
- [ ] Limpiar sesión al hacer logout

### Deploy en servidor Contabo VPS
- [ ] Agregar healthcheck al servicio `db` en `docker-compose.yml` para que `api` espere antes de iniciar
- [ ] Actualizar la URL base del `HttpClient` en Qt para apuntar a `89.116.29.15`
- [ ] (Opcional) Configurar acceso SSH por clave pública para cada integrante del grupo

### Comunicación Qt ↔ Backend (QtNetwork)
- [ ] Implementar `HttpClient` wrapper sobre `QNetworkAccessManager` con métodos `get()`, `post()`
- [ ] Adjuntar header `Authorization: Bearer <token>` en todas las peticiones autenticadas
- [ ] Implementar `AuthService` con slots para `login(user, pass)` y `registerUser(...)`; emitir signals `loginSuccess(UserData)` / `loginFailed(QString error)`
- [ ] Implementar `MatchHistoryService`: obtener historial y subir nueva partida analizada
- [ ] Manejar timeout de red con `QNetworkReply::finished` y verificar `error()` antes de parsear JSON

---

## Área 4 — Integración con Stockfish y Análisis

> Cubre: protocolo UCI completo vía QProcess, parsing de output, análisis posición a posición, Evaluation Bar, Accuracy Score.

### Setup y ciclo de vida del proceso Stockfish
- [ ] Implementar clase `StockfishEngine` heredando `QObject`; lanzar proceso con `QProcess` apuntando a `bin/stockfish.exe`
- [ ] Configurar `QProcess`: `setReadChannel(QProcess::StandardOutput)`, deshabilitar stderr para no interferir
- [ ] **Handshake UCI**: enviar `"uci\n"`, leer líneas hasta recibir `"uciok"` → motor listo
- [ ] **Sincronización**: enviar `"isready\n"`, leer hasta `"readyok"` antes de cualquier análisis
- [ ] **Nueva partida**: enviar `"ucinewgame\n"` al cargar cada partida nueva
- [ ] **Shutdown limpio**: enviar `"quit\n"` al cerrar la app o destruir `StockfishEngine`

### Configuración del motor (setoption)
- [ ] Enviar `"setoption name Threads value 4\n"` (ajustar a núcleos disponibles)
- [ ] Enviar `"setoption name Hash value 256\n"` (MB para tabla de transposición)
- [ ] Enviar `"setoption name Skill Level value 20\n"` para análisis a máxima fuerza
- [ ] Permitir configurar `UCI_Elo` (rango 1320–3190) para modo de práctica contra el motor

### Envío de posición y análisis
- [ ] Enviar posición con FEN: `"position fen <fen_string>\n"` (usar `Board::toFEN()`)
- [ ] Alternativamente: `"position startpos moves e2e4 e7e5 ...\n"` con movimientos en formato UCI largo (`e2e4`, no `e4`)
- [ ] Iniciar análisis a profundidad fija: `"go depth 20\n"`
- [ ] Iniciar análisis con tiempo límite por jugada: `"go movetime 3000\n"` (3 segundos)
- [ ] Detener análisis anticipadamente si es necesario: `"stop\n"` (el motor responde con `bestmove`)

### Parsing de output del motor
- [ ] Conectar `QProcess::readyReadStandardOutput` a slot `onStockfishOutput()`
- [ ] Parsear líneas `info ...` en tiempo real:
  - `depth <n>` — profundidad alcanzada
  - `score cp <n>` — evaluación en centipawns (positivo = ventaja blancas; negar para negras)
  - `score mate <n>` — mate en N movimientos
  - `pv <move1> <move2> ...` — línea principal recomendada
  - `nodes <n>` y `nps <n>` — estadísticas de rendimiento
- [ ] Parsear línea `bestmove <move> [ponder <move>]` como señal de fin de análisis
- [ ] Emitir signals: `evalUpdated(int cp)`, `bestMoveFound(QString move)`, `pvUpdated(QStringList pv)`

### `AnalysisService` — análisis completo de partida
- [ ] Implementar `AnalysisService`: iterar sobre cada movimiento de la partida
- [ ] Para cada posición: enviar FEN → `go depth 18` → esperar `bestmove` → guardar evaluación
- [ ] Calcular delta de evaluación entre la jugada jugada y la mejor jugada del motor
- [ ] Clasificar cada movimiento según delta (centipawns):
  - **Best / Excellent**: delta ≤ 10 cp
  - **Good**: delta ≤ 25 cp
  - **Inaccuracy**: delta 26–100 cp
  - **Mistake**: delta 101–300 cp
  - **Blunder**: delta > 300 cp
- [ ] Almacenar resultado en `std::vector<MoveAnalysis>` (eval, clasificación, mejor jugada, línea PV)
- [ ] Emitir signal `analysisComplete(QVector<MoveAnalysis>)` al finalizar

### Evaluation Bar
- [ ] Implementar `EvaluationBarWidget` con `QPainter`: barra vertical dividida blanco/negro
- [ ] Mapear centipawns a porcentaje visual (usar función sigmoide para suavizar extremos)
- [ ] Mostrar `M3`, `M-5`, etc. cuando hay mate detectado
- [ ] Animar transición entre evaluaciones con `QPropertyAnimation`

### Accuracy Score
- [ ] Calcular precisión por jugador: `accuracy = 100 * (1 - avg_error_rate)`
- [ ] Desglosar por fase: apertura (jugadas 1–10), medio juego (11–30), final (31+)
- [ ] Exponer métricas al `AnalysisPanel` vía signal `accuracyReady(double white, double black)`

---

## Área 5 — IA, Estadísticas y Exportación PDF

> Cubre: OpenAI API via QtNetwork, estadísticas con SELECT agregado, QPainter, exportación PDF con QPrinter.

### Integración OpenAI (QtNetwork)
- [ ] Implementar `AIExplanationService` heredando `QObject`
- [ ] Usar `QNetworkAccessManager` para `POST https://api.openai.com/v1/chat/completions`
- [ ] Adjuntar header `Authorization: Bearer $OPENAI_API_KEY` (leer de variable de entorno o config local)
- [ ] Construir prompt con contexto: FEN de la posición, jugada realizada, mejor jugada del motor, evaluación antes/después
- [ ] Parsear respuesta JSON con `QJsonDocument` y extraer `choices[0].message.content`
- [ ] Generar explicación de errores estratégicos en lenguaje natural
- [ ] Generar interpretación de posiciones (ventajas estructurales, actividad de piezas)
- [ ] Generar análisis textual de jugadas críticas identificadas por Stockfish
- [ ] Manejar rate limits: detectar HTTP 429, reintentar una vez tras 2 segundos con `QTimer::singleShot`
- [ ] Emitir signal `explanationReady(int moveIndex, QString explanation)` para actualizar UI

### Estadísticas
- [ ] Implementar `StatisticsService`: consultar historial de la BD local (SQLite) y/o backend
- [ ] SELECT: precisión promedio general (`AVG(accuracy_white)`, `AVG(accuracy_black)`)
- [ ] SELECT: evolución de precisión ordenada por fecha de partida
- [ ] SELECT: frecuencia de blunders por usuario (`COUNT WHERE classification = 'blunder'`)
- [ ] SELECT: rendimiento por rango de movimientos (partidas cortas vs largas)
- [ ] Exponer datos como `QVector<StatPoint>` a la vista

### `StatisticsView` (Qt)
- [ ] Implementar `StatisticsView` cargando `statisticsview.ui`
- [ ] Mostrar gráfico de evolución de precisión con `QPainter` (barras simples por partida, sin QChart)
- [ ] Mostrar tabla `QTableWidget` con historial: fecha, resultado, precisión blancas, precisión negras, blunders
- [ ] Conectar click en fila de la tabla a carga de esa partida en el tablero principal

### Exportación PDF
- [ ] Usar `QPrinter` + `QPainter` para generar PDF (sin dependencias externas)
- [ ] Sección 1: datos de la partida (jugadores, fecha, resultado, apertura)
- [ ] Sección 2: gráfico de evaluación a lo largo de la partida renderizado con `QPainter`
- [ ] Sección 3: tabla de movimientos con clasificación de errores (Best/Good/Inaccuracy/Mistake/Blunder) basada en Stockfish
- [ ] Usar `QFileDialog` para elegir ruta de guardado
- [ ] Probar exportación con partida real y ajustar márgenes y fuentes
