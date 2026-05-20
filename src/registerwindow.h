#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

class RegisterWindow : public QWidget {
    Q_OBJECT

public:
    explicit RegisterWindow(QWidget *parent = nullptr);

signals:
    void registerRequested(const QString& username, const QString& email, const QString& password);
    void backToLogin();

private slots:
    void onRegisterClicked();
    void onBackClicked();
    bool validateInputs();

private:
    QLineEdit *usernameEdit;
    QLineEdit *emailEdit;
    QLineEdit *passwordEdit;
    QLineEdit *confirmPasswordEdit;
    QPushButton *registerButton;
    QPushButton *backButton;
};
