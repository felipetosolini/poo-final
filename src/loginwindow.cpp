#include "loginwindow.h"
#include "ui_loginwindow.h"

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent, Qt::Window)
    , ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
    connect(ui->loginButton,   &QPushButton::clicked,        this, &LoginWindow::onLoginClicked);
    connect(ui->registerButton,&QPushButton::clicked,        this, &LoginWindow::onRegisterClicked);
    connect(ui->passwordEdit,  &QLineEdit::returnPressed,    this, &LoginWindow::onLoginClicked);
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::onLoginClicked()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();
    if (username.isEmpty() || password.isEmpty())
        return;
    emit loginRequested(username, password);
}

void LoginWindow::onRegisterClicked()
{
    emit registerRequested();
}
