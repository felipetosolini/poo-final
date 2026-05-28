#include "analysissidebar.h"
#include <QVBoxLayout>

AnalysisSidebarWidget::AnalysisSidebarWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumWidth(300);
    
    auto mainLayout = new QVBoxLayout(this);
    
    tabWidget = new QTabWidget();
    
    // Pestaña 1: Motor
    engineAnalysisEdit = new QTextEdit();
    engineAnalysisEdit->setReadOnly(true);
    tabWidget->addTab(engineAnalysisEdit, "Motor");

    // Pestaña 2: IA
    aiExplanationEdit = new QTextEdit();
    aiExplanationEdit->setReadOnly(true);
    tabWidget->addTab(aiExplanationEdit, "Explicación IA");
    
    mainLayout->addWidget(tabWidget);
}

void AnalysisSidebarWidget::setEngineAnalysis(const QString& analysis) {
    engineAnalysisEdit->setPlainText(analysis);
}

void AnalysisSidebarWidget::setAIExplanation(const QString& explanation) {
    aiExplanationEdit->setPlainText(explanation);
}

void AnalysisSidebarWidget::clear() {
    engineAnalysisEdit->clear();
    aiExplanationEdit->clear();
}
