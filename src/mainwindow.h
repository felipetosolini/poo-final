#pragma once

#include <QMainWindow>
#include <QShortcut>
#include <QVector>
#include "gamemanager.h"
#include "boardwidget.h"
#include "movelistwidget.h"
#include "capturedpieceswidget.h"
#include "evaluationbarwidget.h"
#include "analysissidebar.h"
#include "loginwindow.h"
#include "registerwindow.h"
#include "userdata.h"
#include "sessionmanager.h"
#include "httpclient.h"
#include "authservice.h"
#include "matchhistoryservice.h"
#include "aiexplanationservice.h"
#include "statisticsservice.h"
#include "statisticsview.h"
#include "analysisdata.h"
#include "stockfishengine.h"
#include "analysisservice.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    // GameManager
    void onBoardUpdated(const chess::Board& board);
    void onMoveNavigated(int index);

    // BoardWidget
    void onMoveRequested(const chess::Move& move);

    // UI actions
    void onOpenPGN();
    void onExportPDF();
    void onPreviousMove();
    void onNextMove();
    void onStartGame();
    void onEndGame();
    void onPlayPause();

    // Auth
    void onLoginRequested(const QString& username, const QString& password);
    void onRegisterRequested();
    void onBackToLogin();
    void onLoginSuccess(const UserData& user);
    void onLoginFailed(const QString& error);
    void onRegisterSuccess();
    void onRegisterFailed(const QString& error);
    void onLogout();

    // Área 5 — IA y estadísticas
    void onAIExplanationReady(int moveIndex, const QString& explanation);
    void onShowStatistics();
    void onMatchSelectedFromStats(int matchId);

    // Stockfish / Análisis (Área 4)
    void onEngineReady();
    void onAnalysisProgress(int current, int total);
    void onMoveAnalyzed(int moveIndex, MoveAnalysis analysis);
    void onAnalysisComplete(QVector<MoveAnalysis> results, AccuracyScore accuracy);

private:
    Ui::MainWindow *ui;

    // Core widgets
    GameManager             *gameManager          = nullptr;
    BoardWidget             *boardWidget           = nullptr;
    MoveListWidget          *moveListWidget        = nullptr;
    CapturedPiecesWidget    *capturedPiecesWidget  = nullptr;
    EvaluationBarWidget     *evaluationBarWidget   = nullptr;
    AnalysisSidebarWidget   *analysisSidebar       = nullptr;
    LoginWindow             *loginWindow           = nullptr;
    RegisterWindow          *registerWindow        = nullptr;

    // Servicios de red y sesión (Área 3)
    SessionManager      *sessionManager      = nullptr;
    HttpClient          *httpClient          = nullptr;
    AuthService         *authService         = nullptr;
    MatchHistoryService *matchHistoryService = nullptr;

    // Área 5 — IA, estadísticas y PDF
    AIExplanationService *aiExplanationService = nullptr;
    StatisticsService    *statisticsService    = nullptr;
    StatisticsView       *statisticsView       = nullptr;

    // Stockfish + análisis (Área 4)
    StockfishEngine         *stockfishEngine  = nullptr;
    AnalysisService         *analysisService  = nullptr;
    QVector<MoveAnalysis>    m_currentAnalysis;
    std::vector<chess::Board> gameBoardStates;
    std::vector<chess::Move>  m_pendingMoves;   // análisis en espera si el motor no estaba listo

    // Shortcuts
    QShortcut *shortcutOpenPGN   = nullptr;
    QShortcut *shortcutExportPDF = nullptr;
    QShortcut *shortcutPrevMove  = nullptr;
    QShortcut *shortcutNextMove  = nullptr;

    // State
    bool isPlaying     = false;
    int  playbackSpeed = 1000;

    void setupUI();
    void setupConnections();
    void setupShortcuts();
    void applyStyles();
    void showLoginWindow();
    void showMainWindow();
    void showMoveAnalysis(int moveIndex);
    std::vector<chess::Board> buildBoardStates(const std::vector<chess::Move>& moves);
};
