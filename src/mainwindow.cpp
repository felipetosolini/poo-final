#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "config.h"
#include "chess/pgnparser.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Chess Insight AI");
    setMinimumSize(1200, 800);

    // Widgets
    gameManager          = new GameManager(this);
    boardWidget          = new BoardWidget(this);
    moveListWidget       = new MoveListWidget(this);
    capturedPiecesWidget = new CapturedPiecesWidget(this);
    evaluationBarWidget  = new EvaluationBarWidget(this);
    analysisSidebar      = new AnalysisSidebarWidget(this);
    loginWindow          = new LoginWindow(this);
    registerWindow       = new RegisterWindow(this);

    // Stockfish + análisis
    stockfishEngine = new StockfishEngine(this);
    analysisService = new AnalysisService(stockfishEngine, this);

    setupUI();
    setupConnections();
    setupShortcuts();
    applyStyles();

    // Arrancar Stockfish (si el binario no existe, emite engineError en setupConnections)
    stockfishEngine->start(Config::STOCKFISH_PATH);

    showLoginWindow();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ── UI ───────────────────────────────────────────────────────────────────────

void MainWindow::setupUI()
{
    auto centralWidget = new QWidget(this);
    auto mainLayout    = new QHBoxLayout(centralWidget);

    // Panel izquierdo: tablero + piezas capturadas
    auto leftPanel  = new QWidget();
    auto leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->addWidget(new QLabel("Chess Board"));
    leftLayout->addWidget(boardWidget, 1);
    leftLayout->addWidget(capturedPiecesWidget);
    mainLayout->addWidget(leftPanel, 2);

    // Panel central: navegación + lista de movimientos
    auto centerPanel  = new QWidget();
    auto centerLayout = new QVBoxLayout(centerPanel);

    auto navLayout = new QHBoxLayout();
    auto btnStart  = new QPushButton("|<");
    auto btnPrev   = new QPushButton("<");
    auto btnNext   = new QPushButton(">");
    auto btnEnd    = new QPushButton(">|");
    auto btnPlay   = new QPushButton("Play");

    connect(btnStart, &QPushButton::clicked, this, &MainWindow::onStartGame);
    connect(btnPrev,  &QPushButton::clicked, this, &MainWindow::onPreviousMove);
    connect(btnNext,  &QPushButton::clicked, this, &MainWindow::onNextMove);
    connect(btnEnd,   &QPushButton::clicked, this, &MainWindow::onEndGame);
    connect(btnPlay,  &QPushButton::clicked, this, &MainWindow::onPlayPause);

    navLayout->addWidget(btnStart);
    navLayout->addWidget(btnPrev);
    navLayout->addWidget(btnNext);
    navLayout->addWidget(btnEnd);
    navLayout->addWidget(btnPlay);

    centerLayout->addLayout(navLayout);
    centerLayout->addWidget(moveListWidget);

    auto openButton = new QPushButton("Open PGN");
    connect(openButton, &QPushButton::clicked, this, &MainWindow::onOpenPGN);
    centerLayout->addWidget(openButton);

    mainLayout->addWidget(centerPanel, 1);

    // Panel derecho: barra de evaluación + sidebar análisis
    auto rightPanel  = new QWidget();
    auto rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->addWidget(new QLabel("Evaluation"));
    rightLayout->addWidget(evaluationBarWidget);
    rightLayout->addWidget(analysisSidebar);
    mainLayout->addWidget(rightPanel, 1);

    setCentralWidget(centralWidget);
}

void MainWindow::setupConnections()
{
    // GameManager → UI
    connect(gameManager, &GameManager::boardUpdated,   this, &MainWindow::onBoardUpdated);
    connect(gameManager, &GameManager::moveNavigated,  this, &MainWindow::onMoveNavigated);

    // BoardWidget → GameManager
    connect(boardWidget, &BoardWidget::moveRequested, this, &MainWindow::onMoveRequested);

    // MoveListWidget → GameManager
    connect(moveListWidget, &MoveListWidget::moveSelected,
            gameManager, &GameManager::goToMove);

    // Auth
    connect(loginWindow,    &LoginWindow::loginRequested,    this, &MainWindow::onLoginRequested);
    connect(loginWindow,    &LoginWindow::registerRequested, this, &MainWindow::onRegisterRequested);
    connect(registerWindow, &RegisterWindow::backToLogin,    this, &MainWindow::onBackToLogin);
    connect(registerWindow, &RegisterWindow::registerRequested,
            [this](const QString&, const QString&, const QString&) {
                QMessageBox::information(this, "Success", "Registration successful! Please log in.");
                onBackToLogin();
            });

    // StockfishEngine → UI
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

    // AnalysisService → UI
    connect(analysisService, &AnalysisService::progressUpdated,
            this, &MainWindow::onAnalysisProgress);
    connect(analysisService, &AnalysisService::moveAnalyzed,
            this, &MainWindow::onMoveAnalyzed);
    connect(analysisService, &AnalysisService::analysisComplete,
            this, &MainWindow::onAnalysisComplete);
}

void MainWindow::setupShortcuts()
{
    shortcutOpenPGN  = new QShortcut(Qt::CTRL + Qt::Key_O, this);
    shortcutExportPDF = new QShortcut(Qt::CTRL + Qt::Key_E, this);
    shortcutPrevMove = new QShortcut(Qt::Key_Left,  this);
    shortcutNextMove = new QShortcut(Qt::Key_Right, this);

    connect(shortcutOpenPGN,  &QShortcut::activated, this, &MainWindow::onOpenPGN);
    connect(shortcutExportPDF, &QShortcut::activated, this, &MainWindow::onExportPDF);
    connect(shortcutPrevMove, &QShortcut::activated, this, &MainWindow::onPreviousMove);
    connect(shortcutNextMove, &QShortcut::activated, this, &MainWindow::onNextMove);
}

void MainWindow::applyStyles()
{
    setStyleSheet(R"(
        QWidget            { background-color: #f5f5f5; color: #333333; }
        QPushButton        { background-color: #4CAF50; color: white; border: none;
                             border-radius: 4px; padding: 8px 16px; font-weight: bold; }
        QPushButton:hover  { background-color: #45a049; }
        QPushButton:pressed{ background-color: #3d8b40; }
        QLineEdit, QTextEdit { border: 1px solid #cccccc; border-radius: 4px; padding: 5px; }
    )");
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
}

void MainWindow::onMoveNavigated(int index)
{
    moveListWidget->setCurrentMove(index);
    showMoveAnalysis(index);
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
        this, "Open PGN File", "", "PGN Files (*.pgn)");
    if (fileName.isEmpty())
        return;

    chess::PGNParser parser;
    auto games = parser.parseFile(fileName);
    if (games.empty()) {
        QMessageBox::warning(this, "PGN", "No valid games found in file.");
        return;
    }

    const chess::Game& game  = games.front();
    const auto&        moves = game.getMoves();
    if (moves.empty()) {
        QMessageBox::warning(this, "PGN", "Game has no moves.");
        return;
    }

    // Cargar en GameManager
    gameManager->loadGame(moves);

    // Poblar lista de movimientos
    moveListWidget->clearMoves();
    for (int i = 0; i < static_cast<int>(moves.size()); ++i)
        moveListWidget->addMove(moves[i].getAlgebraic(), i);

    gameManager->goToMove(-1);

    // Construir estados del tablero para el análisis
    gameBoardStates = buildBoardStates(moves);
    analysisResults.clear();
    analysisSidebar->clear();

    // Lanzar análisis si Stockfish está listo
    if (stockfishEngine->isReady()) {
        analysisSidebar->setEngineAnalysis("Starting analysis...");
        analysisService->analyzeGame(gameBoardStates, moves, Config::STOCKFISH_DEPTH);
    } else {
        analysisSidebar->setEngineAnalysis(
            "Stockfish not ready.\nMake sure bin/stockfish.exe exists.");
    }
}

void MainWindow::onExportPDF()
{
    QMessageBox::information(this, "Export", "PDF export coming soon...");
}

void MainWindow::onPreviousMove() { gameManager->previousMove(); }
void MainWindow::onNextMove()     { gameManager->nextMove(); }
void MainWindow::onStartGame()    { gameManager->startGame(); }
void MainWindow::onEndGame()      { gameManager->endGame(); }

void MainWindow::onPlayPause()
{
    isPlaying = !isPlaying;
    if (isPlaying)
        QTimer::singleShot(playbackSpeed, this, &MainWindow::onNextMove);
}

// ── Auth ─────────────────────────────────────────────────────────────────────

void MainWindow::onLoginRequested(const QString& username, const QString& password)
{
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter username and password");
        return;
    }
    QMessageBox::information(this, "Login", "Login successful (demo)");
    showMainWindow();
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

// ── Slots de Stockfish / Análisis ────────────────────────────────────────────

void MainWindow::onEngineReady()
{
    analysisSidebar->setEngineAnalysis("Stockfish ready.\nOpen a PGN to start analysis.");
}

void MainWindow::onAnalysisProgress(int current, int total)
{
    analysisSidebar->setEngineAnalysis(
        QString("Analyzing move %1 / %2...").arg(current + 1).arg(total));
}

void MainWindow::onMoveAnalyzed(int moveIndex, MoveAnalysis analysis)
{
    // Guardar resultado (extender vector si hace falta)
    while (analysisResults.size() <= moveIndex)
        analysisResults.append(MoveAnalysis{});
    analysisResults[moveIndex] = analysis;

    // Actualizar sidebar si es el movimiento activo
    if (moveIndex == gameManager->getCurrentMoveIndex())
        showMoveAnalysis(moveIndex);
}

void MainWindow::onAnalysisComplete(QVector<MoveAnalysis> results, AccuracyScore accuracy)
{
    analysisResults = results;

    QString summary;
    summary += "=== Analysis Complete ===\n\n";
    summary += QString("White accuracy : %1%\n").arg(accuracy.white,        0, 'f', 1);
    summary += QString("Black accuracy : %1%\n").arg(accuracy.black,        0, 'f', 1);
    summary += QString("White blunders : %1  mistakes: %2\n")
                    .arg(accuracy.whiteBlunders).arg(accuracy.whiteMistakes);
    summary += QString("Black blunders : %1  mistakes: %2\n")
                    .arg(accuracy.blackBlunders).arg(accuracy.blackMistakes);
    summary += "\n--- By phase (White) ---\n";
    summary += QString("Opening  : %1%\n").arg(accuracy.whiteOpening,  0, 'f', 1);
    summary += QString("Midgame  : %1%\n").arg(accuracy.whiteMidgame,  0, 'f', 1);
    summary += QString("Endgame  : %1%\n").arg(accuracy.whiteEndgame,  0, 'f', 1);
    summary += "--- By phase (Black) ---\n";
    summary += QString("Opening  : %1%\n").arg(accuracy.blackOpening,  0, 'f', 1);
    summary += QString("Midgame  : %1%\n").arg(accuracy.blackMidgame,  0, 'f', 1);
    summary += QString("Endgame  : %1%\n").arg(accuracy.blackEndgame,  0, 'f', 1);

    analysisSidebar->setEngineAnalysis(summary);

    // Mostrar análisis del movimiento activo
    showMoveAnalysis(gameManager->getCurrentMoveIndex());
}

// ── Helpers ───────────────────────────────────────────────────────────────────

void MainWindow::showMoveAnalysis(int index)
{
    if (index < 0 || index >= analysisResults.size())
        return;

    const MoveAnalysis& ma = analysisResults[index];

    // Actualizar barra de evaluación con el eval de esta posición
    evaluationBarWidget->updateEval(ma.evalBefore);

    QString text;
    text += QString("Move %1\n").arg(index + 1);
    text += QString("Classification : %1\n")
                .arg(MoveAnalysis::classificationToString(ma.classification));
    text += QString("Best move      : %1\n").arg(ma.bestMove);
    text += QString("Eval           : %1 cp\n").arg(ma.evalBefore);
    text += QString("Delta          : %1 cp\n").arg(ma.delta);
    if (!ma.pv.isEmpty())
        text += "Line: " + ma.pv.join(" ");

    analysisSidebar->setEngineAnalysis(text);
}

std::vector<chess::Board> MainWindow::buildBoardStates(const std::vector<chess::Move>& moves)
{
    std::vector<chess::Board> boards;
    boards.reserve(moves.size());

    chess::Board board;
    board.initStandardPosition();

    for (const auto& mv : moves) {
        boards.push_back(board); // posición ANTES de este movimiento
        board.movePiece(mv.getFromRow(), mv.getFromCol(),
                        mv.getToRow(),   mv.getToCol());
        board.switchTurn();
    }

    return boards;
}
