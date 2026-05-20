#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

class LoginWindow : public QWidget {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);

signals:
    void loginRequested(const QString& username, const QString& password);
    void registerRequested();

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    QPushButton *registerButton;
};
