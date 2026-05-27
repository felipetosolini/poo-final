#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>
#include <QMenuBar>
#include <QAction>
#include <QTemporaryFile>
#include <QJsonDocument>
#include <QJsonObject>
#include "pdfexporter.h"
#include "chess/pgnparser.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Chess Insight AI");
    setMinimumSize(1200, 800);
    
    // Inicializar componentes
    gameManager = new GameManager(this);
    boardWidget = new BoardWidget(this);
    moveListWidget = new MoveListWidget(this);
    capturedPiecesWidget = new CapturedPiecesWidget(this);
    evaluationBarWidget = new EvaluationBarWidget(this);
    analysisSidebar = new AnalysisSidebarWidget(this);
    loginWindow = new LoginWindow(this);
    registerWindow = new RegisterWindow(this);

    // Servicios de red y sesión (Área 3)
    sessionManager      = new SessionManager();
    httpClient          = new HttpClient(this);
    authService         = new AuthService(httpClient, this);
    matchHistoryService = new MatchHistoryService(httpClient, this);

    // Área 5 — IA, estadísticas y PDF
    aiExplanationService = new AIExplanationService(this);
    statisticsService    = new StatisticsService(httpClient, this);
    statisticsView       = new StatisticsView();  // ventana independiente

    setupUI();
    setupConnections();
    setupShortcuts();
    applyStyles();

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

void MainWindow::setupUI() {
    // Layout principal
    auto centralWidget = new QWidget(this);
    auto mainLayout = new QHBoxLayout(centralWidget);
    
    // Panel izquierdo (tablero + info)
    auto leftPanel = new QWidget();
    auto leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->addWidget(new QLabel("Chess Board"));
    leftLayout->addWidget(boardWidget, 1);
    leftLayout->addWidget(capturedPiecesWidget);
    mainLayout->addWidget(leftPanel, 2);
    
    // Panel central (timeline)
    auto centerPanel = new QWidget();
    auto centerLayout = new QVBoxLayout(centerPanel);
    
    auto navLayout = new QHBoxLayout();
    auto btnStart = new QPushButton("|<");
    auto btnPrev = new QPushButton("<");
    auto btnNext = new QPushButton(">");
    auto btnEnd = new QPushButton(">|");
    auto btnPlay = new QPushButton("Play");
    
    connect(btnStart, &QPushButton::clicked, this, &MainWindow::onStartGame);
    connect(btnPrev, &QPushButton::clicked, this, &MainWindow::onPreviousMove);
    connect(btnNext, &QPushButton::clicked, this, &MainWindow::onNextMove);
    connect(btnEnd, &QPushButton::clicked, this, &MainWindow::onEndGame);
    connect(btnPlay, &QPushButton::clicked, this, &MainWindow::onPlayPause);
    
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
    
    // Panel derecho (evaluación + análisis)
    auto rightPanel = new QWidget();
    auto rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->addWidget(new QLabel("Evaluation"));
    rightLayout->addWidget(evaluationBarWidget);
    rightLayout->addWidget(analysisSidebar);
    mainLayout->addWidget(rightPanel, 1);
    
    setCentralWidget(centralWidget);

    // Menú con opción de logout y estadísticas
    QMenu* userMenu = menuBar()->addMenu("Usuario");
    QAction* statsAction  = new QAction("Estadísticas", this);
    QAction* logoutAction = new QAction("Cerrar sesión", this);
    connect(statsAction,  &QAction::triggered, this, &MainWindow::onShowStatistics);
    connect(logoutAction, &QAction::triggered, this, &MainWindow::onLogout);
    userMenu->addAction(statsAction);
    userMenu->addSeparator();
    userMenu->addAction(logoutAction);
}

void MainWindow::setupConnections() {
    // GameManager → UI
    connect(gameManager, &GameManager::boardUpdated, this, &MainWindow::onBoardUpdated);
    connect(gameManager, &GameManager::moveNavigated, this, &MainWindow::onMoveNavigated);
    
    // BoardWidget → GameManager
    connect(boardWidget, &BoardWidget::moveRequested, this, &MainWindow::onMoveRequested);
    
    // Auth — ventanas
    connect(loginWindow,    &LoginWindow::loginRequested,    this, &MainWindow::onLoginRequested);
    connect(loginWindow,    &LoginWindow::registerRequested, this, &MainWindow::onRegisterRequested);
    connect(registerWindow, &RegisterWindow::backToLogin,    this, &MainWindow::onBackToLogin);
    connect(registerWindow, &RegisterWindow::registerRequested, this,
            [this](const QString& user, const QString& email, const QString& pwd) {
                authService->registerUser(user, email, pwd);
            });

    // AuthService → MainWindow
    connect(authService, &AuthService::loginSuccess,   this, &MainWindow::onLoginSuccess);
    connect(authService, &AuthService::loginFailed,    this, &MainWindow::onLoginFailed);
    connect(authService, &AuthService::registerSuccess, this, &MainWindow::onRegisterSuccess);
    connect(authService, &AuthService::registerFailed,  this, &MainWindow::onRegisterFailed);

    // Área 5 — IA
    connect(aiExplanationService, &AIExplanationService::explanationReady,
            this,                 &MainWindow::onAIExplanationReady);

    // Área 5 — Estadísticas
    connect(statisticsService, &StatisticsService::historyLoaded,
            statisticsView,    &StatisticsView::setHistory);
    connect(statisticsService, &StatisticsService::statsLoaded,
            statisticsView,    &StatisticsView::setUserStats);
    connect(statisticsView, &StatisticsView::matchSelected,
            this,            &MainWindow::onMatchSelectedFromStats);
}

void MainWindow::setupShortcuts() {
    shortcutOpenPGN = new QShortcut(Qt::CTRL + Qt::Key_O, this);
    connect(shortcutOpenPGN, &QShortcut::activated, this, &MainWindow::onOpenPGN);
    
    shortcutExportPDF = new QShortcut(Qt::CTRL + Qt::Key_E, this);
    connect(shortcutExportPDF, &QShortcut::activated, this, &MainWindow::onExportPDF);
    
    shortcutPrevMove = new QShortcut(Qt::Key_Left, this);
    connect(shortcutPrevMove, &QShortcut::activated, this, &MainWindow::onPreviousMove);
    
    shortcutNextMove = new QShortcut(Qt::Key_Right, this);
    connect(shortcutNextMove, &QShortcut::activated, this, &MainWindow::onNextMove);
}

void MainWindow::applyStyles() {
    // Estilos básicos (se pueden cargar desde styles.qss después)
    QString style = R"(
        QWidget {
            background-color: #f5f5f5;
            color: #333333;
        }
        QPushButton {
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #45a049;
        }
        QPushButton:pressed {
            background-color: #3d8b40;
        }
        QLineEdit, QTextEdit {
            border: 1px solid #cccccc;
            border-radius: 4px;
            padding: 5px;
        }
    )";
    setStyleSheet(style);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Left) {
        onPreviousMove();
    } else if (event->key() == Qt::Key_Right) {
        onNextMove();
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

// Slots
void MainWindow::onBoardUpdated(const chess::Board& board) {
    boardWidget->setBoard(board);
}

void MainWindow::onMoveNavigated(int index) {
    moveListWidget->setCurrentMove(index);
}

void MainWindow::onMoveRequested(const chess::Move& move) {
    // Si es navegación
    if (move.getAlgebraic() == "nav_prev") {
        onPreviousMove();
    } else if (move.getAlgebraic() == "nav_next") {
        onNextMove();
    } else {
        gameManager->makeMove(move);
    }
}

void MainWindow::onOpenPGN() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open PGN File", "", "PGN Files (*.pgn)");
    if (fileName.isEmpty()) return;

    chess::PGNParser parser;
    chess::Game game = parser.parseFile(fileName);

    if (!game.isValid()) {
        QMessageBox::warning(this, "PGN Error", "No se pudo parsear el archivo PGN.");
        return;
    }

    gameManager->setMetadata(game.getMetadata());
    gameManager->loadGame(game.getMoves());
    moveListWidget->clearMoves();
    int idx = 0;
    for (const auto& mv : game.getMoves()) {
        moveListWidget->addMove(mv.getAlgebraic(), idx++);
    }
    m_currentAnalysis.clear();
    analysisSidebar->clear();
}

void MainWindow::onExportPDF() {
    const QString fileName = QFileDialog::getSaveFileName(
        this, "Exportar análisis a PDF", "", "PDF (*.pdf)");
    if (fileName.isEmpty()) return;

    const bool ok = PdfExporter::exportToPdf(
        fileName,
        gameManager->getMetadata(),
        gameManager->getMoves(),
        m_currentAnalysis);

    if (ok) {
        QMessageBox::information(this, "PDF exportado",
                                 "El análisis fue exportado correctamente:\n" + fileName);
    } else {
        QMessageBox::warning(this, "Error", "No se pudo crear el archivo PDF.");
    }
}

void MainWindow::onPreviousMove() {
    gameManager->previousMove();
}

void MainWindow::onNextMove() {
    gameManager->nextMove();
}

void MainWindow::onStartGame() {
    gameManager->startGame();
}

void MainWindow::onEndGame() {
    gameManager->endGame();
}

void MainWindow::onPlayPause() {
    isPlaying = !isPlaying;
    if (isPlaying) {
        QTimer::singleShot(playbackSpeed, this, &MainWindow::onNextMove);
    }
}

void MainWindow::onLoginRequested(const QString& username, const QString& password) {
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter username and password");
        return;
    }
    // Delegar al servicio — respuesta llega por onLoginSuccess / onLoginFailed
    authService->login(username, password);
}

void MainWindow::onLoginSuccess(const UserData& user) {
    httpClient->setToken(user.token);
    sessionManager->saveSession(user);
    showMainWindow();
}

void MainWindow::onLoginFailed(const QString& error) {
    QMessageBox::warning(this, "Login failed", error);
}

void MainWindow::onRegisterSuccess() {
    QMessageBox::information(this, "Registro exitoso",
                             "Cuenta creada. Iniciá sesión para continuar.");
    onBackToLogin();
}

void MainWindow::onRegisterFailed(const QString& error) {
    QMessageBox::warning(this, "Error de registro", error);
}

void MainWindow::onLogout() {
    sessionManager->clearSession();
    httpClient->setToken(QString());
    showLoginWindow();
}

void MainWindow::onRegisterRequested() {
    loginWindow->hide();
    registerWindow->show();
}

void MainWindow::onBackToLogin() {
    registerWindow->hide();
    loginWindow->show();
}

void MainWindow::showLoginWindow() {
    hide();
    loginWindow->show();
}

void MainWindow::showMainWindow() {
    loginWindow->hide();
    show();
}

// ---------------------------------------------------------------------------
// Área 5 — IA, estadísticas y PDF
// ---------------------------------------------------------------------------

void MainWindow::onAIExplanationReady(int /*moveIndex*/, const QString& explanation)
{
    analysisSidebar->setAIExplanation(explanation);
}

void MainWindow::onShowStatistics()
{
    // Cargar datos frescos del backend antes de mostrar la vista
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
    // Obtener la partida del backend y cargarla en el tablero.
    // matchHistoryService ya cargó las partidas; las buscamos en su último resultado.
    // Por simplicidad se solicita un fetch fresco y se conecta una vez.
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

        // Escribir PGN a un temp file y parsear
        QTemporaryFile tmp;
        tmp.setAutoRemove(true);
        if (tmp.open()) {
            tmp.write(pgn.toUtf8());
            tmp.flush();
            chess::PGNParser parser;
            chess::Game game = parser.parseFile(tmp.fileName());
            if (game.isValid()) {
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

void MainWindow::onAnalysisComplete(const QVector<MoveAnalysis>& analysis)
{
    // Área 4 (StockfishEngine / AnalysisService) llama a este slot cuando termina
    m_currentAnalysis = analysis;
}
