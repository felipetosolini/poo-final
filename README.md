# Chess Insight AI
### Plataforma Inteligente de Análisis de Partidas de Ajedrez

**Trabajo Práctico Integrador — Programación Orientada a Objetos**

---

## Integrantes

| Nombre | Área |
|---|---|
| Felipe Tosolini | Interfaz gráfica y tablero Qt |
| Agustina Revuelta | Lógica del juego y modelado OO |
| Ignacio Nievas | Backend, autenticación y base de datos |
| Lautaro Robledo | Integración con Stockfish y análisis |
| Facundo Toloza | IA, estadísticas y exportación PDF |

---

## Descripción general

Chess Insight AI es una aplicación de escritorio en C++ y Qt para cargar, reproducir y analizar partidas de ajedrez usando un motor profesional e inteligencia artificial generativa.

El sistema busca replicar la experiencia de plataformas modernas de entrenamiento como Chess.com o Lichess, construida desde cero aplicando los conceptos de POO de la materia.

---

## Funcionalidades

### 1. Gestión de usuarios
- Login y registro con autenticación vía FastAPI + MySQL
- Persistencia local de sesión con SQLite
- Historial personalizado por usuario

### 2. Carga y visualización de partidas
- Carga de archivos `.pgn`
- Tablero 8×8 con piezas gráficas
- Navegación completa: avanzar, retroceder, saltar a cualquier jugada
- Visualización de piezas capturadas y resaltado de movimientos

### 3. Reproducción interactiva
- Reproductor con timeline de movimientos
- Controles de navegación y reconstrucción del tablero en cualquier jugada
- Drag & drop y atajos de teclado

### 4. Análisis automático con Stockfish
- Evaluación de cada posición en centipawns
- Detección de errores: Best / Good / Inaccuracy / Mistake / Blunder
- Sugerencia de mejores movimientos y líneas recomendadas

### 5. Evaluation Bar
- Barra dinámica de ventaja/desventaja actualizada en cada jugada
- Animación fluida y soporte para posiciones de mate

### 6. Accuracy Score
- Cálculo de precisión por jugador comparando con las jugadas del motor
- Desglose por fase: apertura, medio juego y final

### 7. Explicaciones inteligentes con IA
- OpenAI convierte el análisis técnico de Stockfish en texto comprensible
- Explicación de errores estratégicos, interpretación de posiciones y jugadas críticas

### 8. Historial y estadísticas
- Historial de partidas: fecha, resultado, precisión, cantidad de errores
- Estadísticas generales: precisión promedio, evolución del rendimiento, frecuencia de blunders

### 9. Exportación PDF
- Datos de la partida (jugadores, fecha, resultado, apertura)
- Gráfico de evaluación a lo largo de la partida
- Tabla de movimientos con clasificación de errores basada en Stockfish

---

## Tecnologías

| Capa | Tecnología |
|---|---|
| Frontend | C++ · Qt · Qt Designer |
| Backend | FastAPI · Python |
| Bases de datos | MySQL (servidor) · SQLite (local) |
| Motor de análisis | Stockfish |
| IA generativa | OpenAI API |
| Infraestructura | Docker · Docker Compose · GitHub |
| Deploy | Contabo VPS |

---

## Arquitectura modular

```
App de escritorio (Qt/C++)
    ├── BoardWidget + navegación
    ├── Stockfish (proceso local, protocolo UCI)
    └── HttpClient → Backend

Backend (FastAPI + MySQL) — Docker en VPS
    ├── Autenticación JWT
    ├── Historial de partidas
    └── Estadísticas

SQLite (local)
    └── Sesión activa (token de login)
```

### Módulos principales

| Módulo | Clases |
|---|---|
| Núcleo lógico | `Board`, `Piece`, `Move`, `GameManager`, `PGNParser` |
| Análisis | `StockfishEngine`, `AnalysisService`, `AIExplanationService` |
| Persistencia | `UserRepository`, `MatchRepository`, `StatisticsRepository` |
| GUI Qt | `MainWindow`, `BoardWidget`, `AnalysisPanel`, `StatisticsView` |
| Backend | `AuthAPI`, `MatchHistoryAPI`, `StatisticsAPI` |

---

## Aplicación de POO

| Concepto | Aplicación en el proyecto |
|---|---|
| Clase abstracta | `Piece` con métodos virtuales puros `getSymbol()` y `getType()` |
| Herencia | `Pawn`, `Knight`, `Bishop`, `Rook`, `Queen`, `King` heredan de `Piece` |
| Polimorfismo | Renderizado del tablero itera `vector<Piece*>` sin saber el tipo concreto |
| Encapsulamiento | `Board` expone solo `getPiece()`/`setPiece()`; la matriz interna es privada |
| Signals y Slots | `GameManager` emite `boardUpdated()`; `BoardWidget` escucha y se redibuja |
| Templates | `findFirst<T>()` para búsquedas genéricas en vectores de piezas y jugadas |
| Namespace | Todo el núcleo lógico vive en `namespace chess` |
| `const` / `inline` | Getters simples inline; `const` en todos los métodos de solo lectura |
| Punteros | Matriz del tablero con aritmética de punteros; `unique_ptr` para piezas |

---

## Criterios de evaluación

| Área | Puntos |
|---|---|
| Diseño orientado a objetos (POO) | 30 |
| GUI Qt — diseño, funcionalidad e interacción | 35 |
| Tecnologías externas (backend, BD, APIs, Docker) | 15 |
| Valor agregado y creatividad | 20 |
| **Total** | **100** |
