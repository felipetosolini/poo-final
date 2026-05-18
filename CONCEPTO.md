# Chess Insight AI — Documento Conceptual

**Materia:** Programación Orientada a Objetos  
**Stack:** C++ · Qt · FastAPI · MySQL · Stockfish · OpenAI · Docker

---

## ¿Qué es el proyecto?

Una aplicación de escritorio para cargar partidas de ajedrez, verlas de forma visual e interactiva, y obtener un análisis detallado de cada jugada usando un motor profesional de ajedrez e inteligencia artificial.

La idea es recrear la experiencia de plataformas como Chess.com o Lichess, pero construida por nosotros desde cero aplicando los conceptos de POO de la materia.

---

## ¿Qué puede hacer?

| Feature | Descripción |
|---|---|
| Visualizador de partidas | Tablero 8x8 con piezas gráficas, navegación jugada por jugada, piezas capturadas |
| Análisis con Stockfish | Motor profesional que evalúa cada posición y sugiere la mejor jugada |
| Barra de evaluación | Indicador visual de quién tiene ventaja y cuánta, en tiempo real |
| Accuracy Score | Calcula qué tan bien jugó cada jugador comparando con las jugadas del motor |
| Explicaciones con IA | OpenAI convierte el análisis técnico en texto entendible |
| Historial y estadísticas | Precisión promedio, evolución del rendimiento, frecuencia de errores |
| Exportación PDF | Reporte en PDF con datos de la partida, gráfico de evaluación y tabla de clasificación de movimientos de Stockfish |

---

## ¿Cómo está organizado el sistema?

```
App de escritorio (Qt/C++)
    ├── Tablero + navegación
    ├── Stockfish (corre en la misma PC, proceso separado)
    └── Conexión HTTP al backend

Backend (FastAPI + MySQL)  ←→  Servidor VPS (Docker)
    ├── Login / Registro
    ├── Historial de partidas
    └── Estadísticas

SQLite (local)
    └── Solo guarda la sesión activa (token de login)
```

**Flujo típico:** el usuario abre la app → hace login → carga un archivo PGN → Stockfish analiza cada jugada → OpenAI explica los errores → se guarda en el servidor.

---

## ¿Cómo se aplica POO?

### Clases abstractas y funciones virtuales puras
`Piece` es una clase abstracta: define que toda pieza debe tener un símbolo y un tipo, pero no implementa nada. `Pawn`, `Knight`, `Bishop`, `Rook`, `Queen` y `King` heredan de ella y completan la implementación.

### Herencia
Todas las piezas heredan de `Piece`. `GameManager` hereda de `QObject` para poder usar signals y slots. Los widgets heredan de `QWidget`.

### Polimorfismo
Al renderizar el tablero se itera sobre un vector de `Piece*`. Cada pieza responde distinto a `getSymbol()` y `getType()` sin que el código de renderizado necesite saber exactamente qué pieza es.

### Encapsulamiento
`Board` tiene su matriz interna como privada. Nadie la toca directamente, solo a través de `getPiece()` y `setPiece()`. Lo mismo con `StockfishEngine`: el proceso interno es privado, la interfaz pública es la señal `evalUpdated()`.

### Signals y Slots (Qt)
`GameManager` emite señales como `boardUpdated()` al navegar jugadas. `BoardWidget` escucha y se redibuja. `StockfishEngine` emite `evalUpdated(int cp)` y `EvaluationBarWidget` actualiza la barra. Los objetos no dependen directamente entre sí.

### Funciones genéricas (templates)
Se implementa `findFirst<T>()` para buscar el primer elemento que cumpla una condición en cualquier vector. Se usa para buscar piezas en el tablero y jugadas en el historial.

### Namespace
Todas las clases del núcleo lógico (`Board`, `Piece`, `Move`, `Game`) van dentro del namespace `chess` para evitar conflictos con clases de Qt u otras librerías.

### Const, inline y punteros
`const` en todos los métodos que no modifican el objeto. `inline` en getters simples. La matriz del tablero usa aritmética de punteros para acceder a las casillas.

---

## Tecnologías

| Tecnología | Rol |
|---|---|
| C++ + Qt | Lenguaje principal y framework de la app de escritorio |
| Qt Designer (.ui) | Diseño visual de pantallas, cada ventana tiene su archivo `.ui` |
| Stockfish | Motor de ajedrez externo, corre como proceso separado, se comunica por texto |
| OpenAI API | Genera explicaciones en lenguaje natural de los errores |
| FastAPI | Backend REST para login, registro e historial |
| MySQL | Base de datos del servidor (usuarios, partidas, análisis) |
| SQLite | Base de datos local, solo para guardar la sesión activa |
| Docker | Levanta el backend y MySQL con un solo comando |
| Contabo VPS | Servidor donde corre el backend en producción |

---

## División por integrante

| Integrante | Área |
|---|---|
| 1 | Interfaz gráfica: tablero, paneles, navegación, drag & drop |
| 2 | Lógica del juego: Piece, Board, Move, PGNParser, GameManager |
| 3 | Backend: FastAPI, MySQL, JWT, Docker, deploy en VPS |
| 4 | Stockfish: protocolo UCI, análisis, Evaluation Bar, Accuracy, Timeline |
| 5 | IA, estadísticas y exportación PDF |
