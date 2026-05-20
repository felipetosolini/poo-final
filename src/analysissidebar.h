#pragma once

#include <QWidget>
#include <QTabWidget>
#include <QTextEdit>

class AnalysisSidebarWidget : public QWidget {
    Q_OBJECT

public:
    explicit AnalysisSidebarWidget(QWidget *parent = nullptr);

    void setEngineAnalysis(const QString& analysis);
    void setAIExplanation(const QString& explanation);
    void clear();

private:
    QTabWidget *tabWidget;
    QTextEdit *engineAnalysisEdit;
    QTextEdit *aiExplanationEdit;
};
