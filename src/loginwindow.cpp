#include "loginwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Chess Insight AI - Login");
    setMinimumSize(400, 300);
    
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Título
    auto titleLabel = new QLabel("Chess Insight AI");
    auto titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);
    
    mainLayout->addSpacing(20);
    
    // Usuario
    auto userLabel = new QLabel("Username:");
    usernameEdit = new QLineEdit();
    usernameEdit->setPlaceholderText("Enter your username");
    mainLayout->addWidget(userLabel);
    mainLayout->addWidget(usernameEdit);
    
    // Contraseña
    auto passLabel = new QLabel("Password:");
    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("Enter your password");
    mainLayout->addWidget(passLabel);
    mainLayout->addWidget(passwordEdit);
    
    mainLayout->addSpacing(20);
    
    // Botones
    auto buttonLayout = new QHBoxLayout();
    
    loginButton = new QPushButton("Login");
    loginButton->setMinimumHeight(40);
    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    buttonLayout->addWidget(loginButton);
    
    registerButton = new QPushButton("Register");
    registerButton->setMinimumHeight(40);
    connect(registerButton, &QPushButton::clicked, this, &LoginWindow::onRegisterClicked);
    buttonLayout->addWidget(registerButton);
    
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch();
}

void LoginWindow::onLoginClicked() {
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        return;  // TODO: mostrar error
    }
    
    emit loginRequested(username, password);
}

void LoginWindow::onRegisterClicked() {
    emit registerRequested();
}
