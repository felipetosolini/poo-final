#pragma once

#include <QWidget>

namespace Ui { class RegisterWindow; }

class RegisterWindow : public QWidget {
    Q_OBJECT

public:
    explicit RegisterWindow(QWidget *parent = nullptr);
    ~RegisterWindow();

signals:
    void registerRequested(const QString& username, const QString& email, const QString& password);
    void backToLogin();

private slots:
    void onRegisterClicked();
    void onBackClicked();
    bool validateInputs();

private:
    Ui::RegisterWindow *ui;
};
