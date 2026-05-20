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
    
    setupUI();
    setupConnections();
    setupShortcuts();
    applyStyles();
    
    // Mostrar login primero
    showLoginWindow();
}

MainWindow::~MainWindow()
{
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
}

void MainWindow::setupConnections() {
    // GameManager → UI
    connect(gameManager, &GameManager::boardUpdated, this, &MainWindow::onBoardUpdated);
    connect(gameManager, &GameManager::moveNavigated, this, &MainWindow::onMoveNavigated);
    
    // BoardWidget → GameManager
    connect(boardWidget, &BoardWidget::moveRequested, this, &MainWindow::onMoveRequested);
    
    // Auth
    connect(loginWindow, &LoginWindow::loginRequested, this, &MainWindow::onLoginRequested);
    connect(loginWindow, &LoginWindow::registerRequested, this, &MainWindow::onRegisterRequested);
    connect(registerWindow, &RegisterWindow::backToLogin, this, &MainWindow::onBackToLogin);
    connect(registerWindow, &RegisterWindow::registerRequested, [this](const QString& user, const QString& email, const QString& pwd) {
        QMessageBox::information(this, "Success", "Registration successful! Please log in.");
        onBackToLogin();
    });
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
    if (!fileName.isEmpty()) {
        // TODO: parsear PGN y cargar
        QMessageBox::information(this, "PGN", "File selected: " + fileName);
    }
}

void MainWindow::onExportPDF() {
    QMessageBox::information(this, "Export", "PDF export coming soon...");
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
    QMessageBox::information(this, "Login", "Login successful (demo)");
    showMainWindow();
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
