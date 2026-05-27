#pragma once

#include <QMainWindow>
#include <QShortcut>
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
    // GameManager signals
    void onBoardUpdated(const chess::Board& board);
    void onMoveNavigated(int index);
    
    // BoardWidget signals
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
    void onAnalysisComplete(const QVector<MoveAnalysis>& analysis);

private:
    Ui::MainWindow *ui;
    
    // Core components
    GameManager *gameManager;
    BoardWidget *boardWidget;
    MoveListWidget *moveListWidget;
    CapturedPiecesWidget *capturedPiecesWidget;
    EvaluationBarWidget *evaluationBarWidget;
    AnalysisSidebarWidget *analysisSidebar;
    LoginWindow *loginWindow;
    RegisterWindow *registerWindow;

    // Servicios de red y sesión (Área 3)
    SessionManager      *sessionManager;
    HttpClient          *httpClient;
    AuthService         *authService;
    MatchHistoryService *matchHistoryService;

    // Área 5 — IA, estadísticas y PDF
    AIExplanationService *aiExplanationService;
    StatisticsService    *statisticsService;
    StatisticsView       *statisticsView;

    // Datos de análisis de la partida activa (llenados por Área 4)
    QVector<MoveAnalysis> m_currentAnalysis;
    
    // Shortcuts
    QShortcut *shortcutOpenPGN;
    QShortcut *shortcutExportPDF;
    QShortcut *shortcutPrevMove;
    QShortcut *shortcutNextMove;
    
    // State
    bool isPlaying = false;
    int playbackSpeed = 1000;  // ms
    
    void setupUI();
    void setupConnections();
    void setupShortcuts();
    void applyStyles();
    void showLoginWindow();
    void showMainWindow();
};
