#include "loginwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent, Qt::Window)
{
    setWindowTitle("Chess Insight AI");
    setFixedSize(380, 320);

    auto outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addStretch(1);

    // Contenedor centrado
    auto card = new QWidget();
    card->setFixedWidth(300);
    auto cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(10);

    // Título
    auto titleLabel = new QLabel("Chess Insight AI");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #d4d4d4;");
    cardLayout->addWidget(titleLabel);

    auto subtitleLabel = new QLabel("Iniciá sesión para continuar");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet("color: #7a7a7a; font-size: 11px;");
    cardLayout->addWidget(subtitleLabel);
    cardLayout->addSpacing(8);

    usernameEdit = new QLineEdit();
    usernameEdit->setPlaceholderText("Usuario");
    usernameEdit->setMinimumHeight(34);
    cardLayout->addWidget(usernameEdit);

    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("Contraseña");
    passwordEdit->setMinimumHeight(34);
    connect(passwordEdit, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);
    cardLayout->addWidget(passwordEdit);

    cardLayout->addSpacing(4);

    loginButton = new QPushButton("Iniciar sesión");
    loginButton->setMinimumHeight(36);
    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    cardLayout->addWidget(loginButton);

    registerButton = new QPushButton("Crear cuenta");
    registerButton->setObjectName("secondaryButton");
    registerButton->setMinimumHeight(34);
    connect(registerButton, &QPushButton::clicked, this, &LoginWindow::onRegisterClicked);
    cardLayout->addWidget(registerButton);

    // Centrar el card
    auto hCenter = new QHBoxLayout();
    hCenter->addStretch();
    hCenter->addWidget(card);
    hCenter->addStretch();
    outer->addLayout(hCenter);
    outer->addStretch(1);
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
