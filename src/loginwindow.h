#pragma once

#include <QWidget>

namespace Ui { class LoginWindow; }

class LoginWindow : public QWidget {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

signals:
    void loginRequested(const QString& username, const QString& password);
    void registerRequested();

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    Ui::LoginWindow *ui;
};
