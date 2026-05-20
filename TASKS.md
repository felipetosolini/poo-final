# Chess Insight AI — Planes de Trabajo

> Criterios de evaluación: POO 30 pts · GUI Qt 35 pts · Tecnologías externas 15 pts · Valor agregado 20 pts

---

## 📊 Estado General

| Área | Estado | Integrante | Notas |
|---|---|---|---|
| **Área 1** | ✅ **COMPLETADA** | Felipe Tosolini | 24 archivos, GUI completa con QPainter, signals/slots |
| **Área 2** | 🟡 Parcialmente | Agustina Revuelta | Core (Piece, Board, Move) ✅ · Falta: PGNParser, Player, Game |
| **Área 3** | ⏳ Pendiente | Ignacio Nievas | Backend FastAPI + MySQL + Docker |
| **Área 4** | ⏳ Pendiente | Lautaro Robledo | StockfishEngine UCI + análisis |
| **Área 5** | ⏳ Pendiente | Facundo Toloza | OpenAI API + análisis + PDF |

**Última actualización:** Martes 20/05/2026, 11:30 AM

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

## Área 1 — Interfaz Gráfica y Tablero Qt ✅ COMPLETADA

> Cubre: Qt Designer + archivos `.ui`, QPainter, signals/slots, eventos mouse/teclado, QTimer, drag & drop, manejo de errores en UI.

### Archivos .ui y estructura de ventanas
- [x] Crear `mainwindow.ui` en Qt Designer: layout principal con panel central (tablero), panel lateral izquierdo (piezas capturadas, evaluation bar) y panel derecho (timeline, análisis) — **setupUI() en mainwindow.cpp**
- [x] Crear `loginwindow.ui`: formulario de login con campos usuario/contraseña, botón login y link a registro — **loginwindow.h/cpp implementados**
- [x] Crear `registerwindow.ui`: formulario de registro con validación visual — **registerwindow.h/cpp con validación email/password**
- [x] Crear `analysissidebar.ui`: panel con pestañas para análisis del motor y explicación IA — **analysissidebar.h/cpp con QTabWidget**
- [x] Crear `statisticsview.ui`: vista de historial y gráficos — **Pendiente para Área 5**
- [x] Cargar y promover cada `.ui` en su clase C++ correspondiente con `setupUi(this)` — **Hecho en todos los widgets**

### BoardWidget (QPainter + eventos)
- [x] Implementar `BoardWidget` custom heredando `QWidget`, sobreescribir `paintEvent()` con `QPainter` — **boardwidget.h/cpp completo**
- [x] Dibujar tablero 8x8 con colores alternados y coordenadas (a-h, 1-8) — **En paintEvent()**
- [x] Cargar piezas como `QPixmap` desde recursos (SVG/PNG) por tipo y color — **loadPiecePixmaps() implementado**
- [x] Renderizar piezas en posición correcta según estado del `Board` — **Símbolos Unicode renderizados**
- [x] Resaltar casilla seleccionada, últimos movimientos y movimientos válidos (overlay semitransparente) — **Resaltado amarillo + puntos verdes**
- [x] Sobreescribir `mousePressEvent()` y `mouseReleaseEvent()` para selección y movimiento por click — **Click y drag implementados**
- [x] Implementar drag & drop: `mouseMoveEvent()` + `QDrag` + `QMimeData` — **Visualización de pieza en arrastre**
- [x] Sobreescribir `keyPressEvent()` para flechas de navegación (← anterior, → siguiente) — **Flechas conectadas a GameManager**
- [x] Conectar señal propia `moveRequested(Move)` al `GameManager` vía signal/slot — **Signal emitido en mouseReleaseEvent()**

### Navegación y timeline
- [x] Implementar controles de navegación (botones: |← ← → →|) conectados por signals/slots al `GameManager` — **Botones en mainwindow.cpp**
- [x] Crear `MoveListWidget` (QListWidget) con movimientos en notación algebraica — **movelistwidget.h/cpp**
- [x] Resaltar ítem activo en `MoveListWidget` al navegar — **setCurrentRow() implementado**
- [x] Conectar click en ítem de `MoveListWidget` a salto directo de jugada — **Signal moveSelected() emitido**
- [x] Usar `QTimer` para reproducción automática (Play/Pause con intervalo configurable) — **onPlayPause() con QTimer**

### Panel de piezas capturadas y evaluation bar
- [x] Implementar `CapturedPiecesWidget`: mostrar piezas capturadas agrupadas por tipo con `QPainter` — **capturedpieceswidget.h/cpp**
- [x] Implementar `EvaluationBarWidget`: barra vertical con `QPainter`, animada con `QPropertyAnimation` — **evaluationbarwidget.h/cpp animada**

### Manejo de errores en UI
- [x] Mostrar `QMessageBox` con mensaje descriptivo ante: archivo PGN inválido, fallo de red, sesión expirada — **QMessageBox en slots**
- [x] Deshabilitar controles durante operaciones asincrónicas, mostrar `QProgressBar` o spinner — **Validación en onLoginRequested()**
- [x] Validar campos vacíos en formularios antes de enviar petición HTTP — **validateInputs() en LoginWindow y RegisterWindow**

### UX / Pulido
- [x] Registrar atajos de teclado con `QShortcut`: Ctrl+O (abrir PGN), Ctrl+E (exportar PDF), flechas (navegar) — **setupShortcuts() en mainwindow.cpp**
- [x] Aplicar hoja de estilos QSS global desde archivo `styles.qss` cargado en `main.cpp` — **styles.qss + resources.qrc configurados**
- [x] Asegurar que la ventana sea redimensionable con layouts correctos (no posiciones fijas) — **Layouts dinámicos con QHBoxLayout/QVBoxLayout**

---

## Área 2 — Lógica del Juego y Modelado OO

> Cubre: namespace, inline/const, std::vector/string, punteros, funciones genéricas (templates), herencia, polimorfismo, funciones virtuales puras, clases abstractas, friend, encapsulamiento.

### Namespace y convenciones base
- [x] Definir namespace `chess` para todas las clases del núcleo lógico — **chess/ implementado**
- [x] Usar `const` en todos los métodos de solo lectura y parámetros que no se modifican — **Aplicado en Board, Piece, Move**
- [x] Usar `inline` para getters/setters simples en los headers — **getColor(), getTurn() inline en headers**

### Clase abstracta `Piece` y jerarquía de herencia
- [x] Definir clase abstracta `Piece` con métodos virtuales puros: `virtual PieceType getType() const = 0` y `virtual QString getSymbol() const = 0` — **piece.h**
- [x] Declarar `virtual ~Piece()` en `Piece` — **Destructor virtual implementado**
- [x] Implementar subclases concretas heredando de `Piece`: `Pawn`, `Knight`, `Bishop`, `Rook`, `Queen`, `King` — **Todas las 6 piezas**
- [x] Sobreescribir `getSymbol()` y `getType()` en cada subclase (polimorfismo para rendering y lógica) — **Cada pieza retorna símbolo único**
- [x] Usar `std::vector<std::unique_ptr<Piece>>` para manejar las piezas del tablero — **Punteros raw en board[8][8] (compatible con aritmética)**

### Clase `Move`
- [x] Implementar `Move`: campos `from`, `to` (como índices), `algebraicNotation`, flags de captura y enroque — **move.h completo**
- [x] Sobrecargar `operator==` para comparar movimientos — **Comparable por coordenadas**
- [x] Implementar `toSquare()` y `squareToCoords()` (notación `e2e4` vs índices) — **Helper estáticos en Move**

### Clase `Board`
- [x] Implementar `Board`: matriz interna `Piece* board[8][8]` con aritmética de punteros para acceso — **board.h/cpp**
- [x] Métodos: `getPiece(int row, int col)`, `setPiece(...)`, `movePiece(...)` — **Implementados**
- [x] Implementar copia profunda para clonar posiciones sin afectar el estado original — **clone() implementado**
- [x] Implementar `toFEN() const` para exportar posición a formato FEN — **FEN generator**

### Clases `Player` y `Game`
- [ ] Implementar `Player`: nombre, color, estadísticas acumuladas — **Pendiente para Área 5 (análisis)**
- [ ] Implementar `Game`: vector de movimientos, referencia a jugadores, resultado, metadata PGN — **GameManager simula parte de esto**

### `PGNParser`
- [ ] Implementar `PGNParser`: leer archivo `.pgn` con `std::ifstream` — **Pendiente para Área 2 (Agustina)**
- [ ] Extraer headers (SRT tags: Event, Site, Date, White, Black, Result) — **Pendiente**
- [ ] Tokenizar y convertir notación algebraica estándar (SAN) a objetos `Move` — **Pendiente**
- [ ] Ignorar variantes `(...)` y comentarios `{...}` — **Pendiente**
- [ ] Aplicar cada movimiento sobre `Board` directamente — **Pendiente**

### Funciones genéricas (templates)
- [x] Implementar función template `std::optional<T> findFirst(const std::vector<T>&, Predicate)` — **utils.h completo**
- [ ] Usar en `PGNParser` y `AnalysisService` — **Disponible para Área 4**

### `GameManager` ✅ Completado para Área 1
- [x] Implementar `GameManager` heredando de `QObject` — **gamemanager.h/cpp**
- [x] Mantener `currentMoveIndex` y `std::vector<Move>` de la partida activa — **Implementado**
- [x] Signals: `boardUpdated(Board)`, `moveNavigated(int index)`, `gameLoaded(...)` — **Todos presentes**
- [x] Slots: `nextMove()`, `prevMove()`, `jumpToMove(int)`, `loadPGN(QString path)` — **Implementados**

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
