#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "config.h"
#include "chess/pgnparser.h"
#include <QPushButton>
#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>
#include <QMenuBar>
#include <QAction>
#include <QTemporaryFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QCoreApplication>
#include "pdfexporter.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Chess Insight AI");
    setMinimumSize(1200, 800);

    // Asignar widgets del .ui a los punteros miembro
    boardWidget          = ui->boardWidget;
    moveListWidget       = ui->moveListWidget;
    capturedPiecesWidget = ui->capturedPiecesWidget;
    evaluationBarWidget  = ui->evaluationBarWidget;
    analysisSidebar      = ui->analysisSidebar;

    // ObjectNames requeridos por styles.qss
    ui->btnStart->setObjectName("navButton");
    ui->btnPrev->setObjectName("navButton");
    ui->btnNext->setObjectName("navButton");
    ui->btnEnd->setObjectName("navButton");
    ui->btnPlay->setObjectName("secondaryButton");
    ui->exportButton->setObjectName("secondaryButton");

    // Widgets no-UI
    gameManager    = new GameManager(this);
    loginWindow    = new LoginWindow(this);
    registerWindow = new RegisterWindow(this);

    // Servicios de red y sesión (Área 3)
    sessionManager      = new SessionManager();
    httpClient          = new HttpClient(this);
    authService         = new AuthService(httpClient, this);
    matchHistoryService = new MatchHistoryService(httpClient, this);

    // Área 5 — IA, estadísticas y PDF
    aiExplanationService = new AIExplanationService(this);
    statisticsService    = new StatisticsService(httpClient, this);
    statisticsView       = new StatisticsView();

    // Stockfish + análisis (Área 4)
    stockfishEngine = new StockfishEngine(this);
    analysisService = new AnalysisService(stockfishEngine, this);

    setupConnections();
    setupShortcuts();
    applyStyles();

    const QString sfPath = QCoreApplication::applicationDirPath() + "/" + Config::STOCKFISH_PATH;
    stockfishEngine->start(sfPath);

    // Verificar sesión guardada: si existe, saltar el login
    const auto saved = sessionManager->loadSession();
    if (saved.has_value()) {
        httpClient->setToken(saved->token);
        showMainWindow();
    } else {
        showLoginWindow();
    }
}

MainWindow::~MainWindow()
{
    delete sessionManager;
    delete ui;
}


void MainWindow::setupConnections()
{
    // Botones del .ui
    connect(ui->btnStart,    &QPushButton::clicked, this, &MainWindow::onStartGame);
    connect(ui->btnPrev,     &QPushButton::clicked, this, &MainWindow::onPreviousMove);
    connect(ui->btnNext,     &QPushButton::clicked, this, &MainWindow::onNextMove);
    connect(ui->btnEnd,      &QPushButton::clicked, this, &MainWindow::onEndGame);
    connect(ui->btnPlay,     &QPushButton::clicked, this, &MainWindow::onPlayPause);
    connect(ui->openButton,  &QPushButton::clicked, this, &MainWindow::onOpenPGN);
    connect(ui->exportButton, &QPushButton::clicked, this, &MainWindow::onExportPDF);
    connect(ui->logoutButton, &QPushButton::clicked, this, &MainWindow::onLogout);

    // Menú Usuario
    connect(ui->actionEstadisticas, &QAction::triggered, this, &MainWindow::onShowStatistics);
    connect(ui->actionCerrarSesion, &QAction::triggered, this, &MainWindow::onLogout);

    // GameManager → UI
    connect(gameManager, &GameManager::boardUpdated,  this, &MainWindow::onBoardUpdated);
    connect(gameManager, &GameManager::moveNavigated, this, &MainWindow::onMoveNavigated);

    // BoardWidget → GameManager
    connect(boardWidget, &BoardWidget::moveRequested, this, &MainWindow::onMoveRequested);

    // MoveListWidget → GameManager
    connect(moveListWidget, &MoveListWidget::moveSelected,
            gameManager, &GameManager::goToMove);

    // Auth — ventanas
    connect(loginWindow,    &LoginWindow::loginRequested,    this, &MainWindow::onLoginRequested);
    connect(loginWindow,    &LoginWindow::registerRequested, this, &MainWindow::onRegisterRequested);
    connect(registerWindow, &RegisterWindow::backToLogin,    this, &MainWindow::onBackToLogin);
    connect(registerWindow, &RegisterWindow::registerRequested, this,
            [this](const QString& user, const QString& email, const QString& pwd) {
                authService->registerUser(user, email, pwd);
            });

    // AuthService → MainWindow
    connect(authService, &AuthService::loginSuccess,    this, &MainWindow::onLoginSuccess);
    connect(authService, &AuthService::loginFailed,     this, &MainWindow::onLoginFailed);
    connect(authService, &AuthService::registerSuccess, this, &MainWindow::onRegisterSuccess);
    connect(authService, &AuthService::registerFailed,  this, &MainWindow::onRegisterFailed);

    // Área 5 — IA
    connect(aiExplanationService, &AIExplanationService::explanationReady,
            this,                 &MainWindow::onAIExplanationReady);
    connect(aiExplanationService, &AIExplanationService::requestFailed,
            this, [this](int, const QString& error) {
                analysisSidebar->setAIExplanation("No se pudo obtener la explicacion:\n" + error);
            });
    connect(aiExplanationService, &AIExplanationService::gameSummaryReady,
            this,                 &MainWindow::onGameSummaryReady);
    connect(aiExplanationService, &AIExplanationService::gameSummaryFailed,
            this, [this](const QString& error) {
                Q_UNUSED(error)
                m_gameSummary.clear();
            });

    // Área 5 — Estadísticas
    connect(statisticsService, &StatisticsService::historyLoaded,
            statisticsView,    &StatisticsView::setHistory);
    connect(statisticsService, &StatisticsService::statsLoaded,
            statisticsView,    &StatisticsView::setUserStats);
    connect(statisticsView, &StatisticsView::matchSelected,
            this,            &MainWindow::onMatchSelectedFromStats);

    // StockfishEngine → UI (Área 4)
    connect(stockfishEngine, &StockfishEngine::engineReady,
            this, &MainWindow::onEngineReady);
    connect(stockfishEngine, &StockfishEngine::evalUpdated,
            evaluationBarWidget, &EvaluationBarWidget::updateEval);
    connect(stockfishEngine, &StockfishEngine::mateDetected,
            evaluationBarWidget, &EvaluationBarWidget::setMate);
    connect(stockfishEngine, &StockfishEngine::engineError,
            [this](const QString& msg) {
                analysisSidebar->setEngineAnalysis("Stockfish: " + msg);
            });

    // AnalysisService → UI (Área 4)
    connect(analysisService, &AnalysisService::progressUpdated,
            this, &MainWindow::onAnalysisProgress);
    connect(analysisService, &AnalysisService::moveAnalyzed,
            this, &MainWindow::onMoveAnalyzed);
    connect(analysisService, &AnalysisService::analysisComplete,
            this, &MainWindow::onAnalysisComplete);

    // Reproducción automática
    playTimer = new QTimer(this);
    playTimer->setInterval(playbackSpeed);
    connect(playTimer, &QTimer::timeout, this, &MainWindow::onNextMove);
}

void MainWindow::setupShortcuts()
{
    shortcutOpenPGN  = new QShortcut(Qt::CTRL + Qt::Key_O, this);
    shortcutExportPDF = new QShortcut(Qt::CTRL + Qt::Key_E, this);
    shortcutPrevMove = new QShortcut(Qt::Key_Left,  this);
    shortcutNextMove = new QShortcut(Qt::Key_Right, this);

    connect(shortcutOpenPGN,   &QShortcut::activated, this, &MainWindow::onOpenPGN);
    connect(shortcutExportPDF, &QShortcut::activated, this, &MainWindow::onExportPDF);
    connect(shortcutPrevMove,  &QShortcut::activated, this, &MainWindow::onPreviousMove);
    connect(shortcutNextMove,  &QShortcut::activated, this, &MainWindow::onNextMove);
}

void MainWindow::applyStyles()
{
    // El stylesheet global se carga desde :/styles.qss en main.cpp
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Left)
        onPreviousMove();
    else if (event->key() == Qt::Key_Right)
        onNextMove();
    else
        QMainWindow::keyPressEvent(event);
}

// ── Slots de GameManager ──────────────────────────────────────────────────────

void MainWindow::onBoardUpdated(const chess::Board& board)
{
    boardWidget->setBoard(board);
    capturedPiecesWidget->updateFromBoard(board);
}

void MainWindow::onMoveNavigated(int index)
{
    moveListWidget->setCurrentMove(index);
    showMoveAnalysis(index);
    if (isPlaying && index >= gameManager->getTotalMoves() - 1) {
        isPlaying = false;
        playTimer->stop();
    }
}

// ── Slots de BoardWidget ──────────────────────────────────────────────────────

void MainWindow::onMoveRequested(const chess::Move& move)
{
    if (move.getAlgebraic() == "nav_prev")
        onPreviousMove();
    else if (move.getAlgebraic() == "nav_next")
        onNextMove();
    else
        gameManager->makeMove(move);
}

// ── Acciones de UI ───────────────────────────────────────────────────────────

void MainWindow::onOpenPGN()
{
    QString fileName = QFileDialog::getOpenFileName(
        this, "Abrir partida PGN", "", "Archivos PGN (*.pgn)");
    if (fileName.isEmpty())
        return;

    chess::PGNParser parser;
    auto games = parser.parseFile(fileName);
    if (games.empty()) {
        QMessageBox::warning(this, "PGN", "No se encontraron partidas válidas en el archivo.");
        return;
    }

    const chess::Game& game  = games.front();
    const auto&        moves = game.getMoves();
    if (moves.empty()) {
        QMessageBox::warning(this, "PGN", "La partida no contiene movimientos.");
        return;
    }

    gameManager->setMetadata(game.getMetadata());
    gameManager->loadGame(moves);

    moveListWidget->clearMoves();
    for (int i = 0; i < static_cast<int>(moves.size()); ++i)
        moveListWidget->addMove(moves[i].getAlgebraic(), i);

    gameManager->goToMove(-1);

    gameBoardStates = game.getBoardStates();
    m_currentAnalysis.clear();
    m_analysisSummary.clear();
    m_aiExplanationCache.clear();
    analysisSidebar->clear();
    boardWidget->setInteractive(false);
    isPlaying = false;
    playTimer->stop();

    if (stockfishEngine->isReady()) {
        analysisSidebar->setEngineAnalysis("Starting analysis...");
        analysisService->analyzeGame(gameBoardStates, moves, Config::STOCKFISH_DEPTH);
    } else {
        m_pendingMoves = moves;
        analysisSidebar->setEngineAnalysis("Waiting for Stockfish...");
    }
}

void MainWindow::onExportPDF()
{
    const QString fileName = QFileDialog::getSaveFileName(
        this, "Exportar análisis a PDF", "", "PDF (*.pdf)");
    if (fileName.isEmpty()) return;

    const bool ok = PdfExporter::exportToPdf(
        fileName,
        gameManager->getMetadata(),
        gameManager->getMoves(),
        m_currentAnalysis,
        m_gameSummary);

    if (ok) {
        QMessageBox::information(this, "PDF exportado",
                                 "El análisis se exportó correctamente a:\n" + fileName);
    } else {
        QMessageBox::warning(this, "Error al exportar", "No se pudo crear el archivo PDF.");
    }
}

void MainWindow::onPreviousMove() { gameManager->previousMove(); }
void MainWindow::onNextMove()     { gameManager->nextMove(); }
void MainWindow::onStartGame()    { gameManager->startGame(); }
void MainWindow::onEndGame()      { gameManager->endGame(); }

void MainWindow::onPlayPause()
{
    isPlaying = !isPlaying;
    if (isPlaying)
        playTimer->start();
    else
        playTimer->stop();
}

// ── Auth ─────────────────────────────────────────────────────────────────────

void MainWindow::onLoginRequested(const QString& username, const QString& password)
{
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Ingresá usuario y contraseña.");
        return;
    }
    authService->login(username, password);
}

void MainWindow::onLoginSuccess(const UserData& user)
{
    httpClient->setToken(user.token);
    sessionManager->saveSession(user);
    showMainWindow();
}

void MainWindow::onLoginFailed(const QString& error)
{
    QMessageBox::warning(this, "Error al iniciar sesión", error);
}

void MainWindow::onRegisterSuccess()
{
    QMessageBox::information(this, "Registro exitoso",
                             "Cuenta creada. Iniciá sesión para continuar.");
    onBackToLogin();
}

void MainWindow::onRegisterFailed(const QString& error)
{
    QMessageBox::warning(this, "Error de registro", error);
}

void MainWindow::onLogout()
{
    sessionManager->clearSession();
    httpClient->setToken(QString());
    showLoginWindow();
}

void MainWindow::onRegisterRequested()
{
    loginWindow->hide();
    registerWindow->show();
}

void MainWindow::onBackToLogin()
{
    registerWindow->hide();
    loginWindow->show();
}

void MainWindow::showLoginWindow()
{
    hide();
    loginWindow->show();
}

void MainWindow::showMainWindow()
{
    loginWindow->hide();
    show();
}

// ── Área 5 — IA, estadísticas y PDF ─────────────────────────────────────────

void MainWindow::onAIExplanationReady(int moveIndex, const QString& explanation)
{
    m_aiExplanationCache[moveIndex] = explanation;
    analysisSidebar->setAIExplanation(explanation);
}

void MainWindow::onGameSummaryReady(const QString& summary)
{
    m_gameSummary = summary;
}

void MainWindow::onShowStatistics()
{
    const auto saved = sessionManager->loadSession();
    if (saved.has_value()) {
        statisticsService->fetchStats(saved->userId);
        statisticsService->fetchMatchHistory();
    }

    statisticsView->setWindowTitle("Estadísticas — Chess Insight AI");
    statisticsView->resize(900, 650);
    statisticsView->show();
    statisticsView->raise();
}

void MainWindow::onMatchSelectedFromStats(int matchId)
{
    QNetworkReply *reply = httpClient->get(QString("/matches/%1").arg(matchId));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::warning(this, "Error", "No se pudo cargar la partida.");
            return;
        }
        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        const QString pgn       = doc.object().value("pgn").toString();
        if (pgn.isEmpty()) return;

        QTemporaryFile tmp;
        tmp.setAutoRemove(true);
        if (tmp.open()) {
            tmp.write(pgn.toUtf8());
            tmp.flush();
            chess::PGNParser parser;
            auto games = parser.parseFile(tmp.fileName());
            if (!games.empty()) {
                const chess::Game& game = games.front();
                gameManager->setMetadata(game.getMetadata());
                gameManager->loadGame(game.getMoves());
                moveListWidget->clearMoves();
                int idx = 0;
                for (const auto& mv : game.getMoves())
                    moveListWidget->addMove(mv.getAlgebraic(), idx++);
                m_currentAnalysis.clear();
                analysisSidebar->clear();
                statisticsView->hide();
            }
        }
    });
}

// ── Área 4 — Stockfish / Análisis ────────────────────────────────────────────

void MainWindow::onEngineReady()
{
    if (!m_pendingMoves.empty()) {
        analysisSidebar->setEngineAnalysis("Iniciando análisis...");
        analysisService->analyzeGame(gameBoardStates, m_pendingMoves, Config::STOCKFISH_DEPTH);
        m_pendingMoves.clear();
    } else {
        analysisSidebar->setEngineAnalysis("Stockfish listo.\nAbrí un PGN para analizar.");
    }
}

void MainWindow::onAnalysisProgress(int current, int total)
{
    analysisSidebar->setEngineAnalysis(
        QString("Analizando jugada %1 de %2...").arg(current + 1).arg(total));
}

void MainWindow::onMoveAnalyzed(int moveIndex, MoveAnalysis analysis)
{
    while (m_currentAnalysis.size() <= moveIndex)
        m_currentAnalysis.append(MoveAnalysis{});
    m_currentAnalysis[moveIndex] = analysis;

    if (moveIndex == gameManager->getCurrentMoveIndex())
        showMoveAnalysis(moveIndex);
}

void MainWindow::onAnalysisComplete(QVector<MoveAnalysis> results, AccuracyScore accuracy)
{
    m_currentAnalysis = results;

    QString summary;
    summary += "=== Análisis completo ===\n\n";
    summary += QString("Precisión Blancas : %1%\n").arg(accuracy.white, 0, 'f', 1);
    summary += QString("Precisión Negras  : %1%\n").arg(accuracy.black, 0, 'f', 1);
    summary += QString("Blunders Blancas  : %1  Errores: %2\n")
                    .arg(accuracy.whiteBlunders).arg(accuracy.whiteMistakes);
    summary += QString("Blunders Negras   : %1  Errores: %2\n")
                    .arg(accuracy.blackBlunders).arg(accuracy.blackMistakes);
    summary += "\n--- Por fase (Blancas) ---\n";
    summary += QString("Apertura : %1%\n").arg(accuracy.whiteOpening, 0, 'f', 1);
    summary += QString("Medio    : %1%\n").arg(accuracy.whiteMidgame, 0, 'f', 1);
    summary += QString("Final    : %1%\n").arg(accuracy.whiteEndgame, 0, 'f', 1);
    summary += "--- Por fase (Negras) ---\n";
    summary += QString("Apertura : %1%\n").arg(accuracy.blackOpening, 0, 'f', 1);
    summary += QString("Medio    : %1%\n").arg(accuracy.blackMidgame, 0, 'f', 1);
    summary += QString("Final    : %1%\n").arg(accuracy.blackEndgame, 0, 'f', 1);

    m_analysisSummary = summary;
    showMoveAnalysis(gameManager->getCurrentMoveIndex());

    // Solicitar resumen de IA en segundo plano para el PDF
    m_gameSummary.clear();
    aiExplanationService->requestGameSummary(gameManager->getMetadata(), results);
}

// ── Helpers ───────────────────────────────────────────────────────────────────

void MainWindow::showMoveAnalysis(int index)
{
    if (index < 0 || index >= m_currentAnalysis.size()) {
        if (!m_analysisSummary.isEmpty())
            analysisSidebar->setEngineAnalysis(m_analysisSummary);
        return;
    }

    const MoveAnalysis& ma = m_currentAnalysis[index];

    evaluationBarWidget->updateEval(ma.evalBefore);

    QString text;
    text += QString("Jugada %1\n").arg(index + 1);
    text += QString("Clasificación : %1\n")
                .arg(MoveAnalysis::classificationToString(ma.classification));
    text += QString("Mejor jugada  : %1\n").arg(ma.bestMove);
    text += QString("Evaluación    : %1 cp\n").arg(ma.evalBefore);
    text += QString("Delta         : %1 cp\n").arg(ma.delta);
    if (!ma.pv.isEmpty())
        text += "Línea: " + ma.pv.join(" ");

    if (!m_analysisSummary.isEmpty())
        text += "\n\n" + m_analysisSummary;

    analysisSidebar->setEngineAnalysis(text);

    // Solicitar explicación de IA solo para movimientos con error o peor
    if (ma.classification >= MoveClassification::Inaccuracy) {
        if (m_aiExplanationCache.contains(index)) {
            analysisSidebar->setAIExplanation(m_aiExplanationCache[index]);
        } else {
            const QString fen = ma.fen.isEmpty() ? gameManager->getCurrentBoard().toFen() : ma.fen;
            aiExplanationService->requestExplanation(
                index, fen, ma.playedMove, ma.bestMove,
                ma.evalBefore, ma.evalAfter, ma.classification);
        }
    } else {
        analysisSidebar->setAIExplanation(
            QString("Jugada %1 — %2. Buen movimiento.")
                .arg(index + 1)
                .arg(MoveAnalysis::classificationToString(ma.classification)));
    }
}
