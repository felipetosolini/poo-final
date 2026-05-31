#include "registerwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QRegularExpression>

RegisterWindow::RegisterWindow(QWidget *parent)
    : QWidget(parent, Qt::Window)
{
    setWindowTitle("Chess Insight AI");
    setFixedSize(380, 400);

    auto outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addStretch(1);

    auto card = new QWidget();
    card->setFixedWidth(300);
    auto cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(8);

    auto titleLabel = new QLabel("Crear cuenta");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #d4d4d4;");
    cardLayout->addWidget(titleLabel);
    cardLayout->addSpacing(6);

    usernameEdit = new QLineEdit();
    usernameEdit->setPlaceholderText("Usuario (mín. 3 caracteres)");
    usernameEdit->setMinimumHeight(34);
    cardLayout->addWidget(usernameEdit);

    emailEdit = new QLineEdit();
    emailEdit->setPlaceholderText("Email");
    emailEdit->setMinimumHeight(34);
    cardLayout->addWidget(emailEdit);

    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("Contraseña (mín. 8 caracteres)");
    passwordEdit->setMinimumHeight(34);
    cardLayout->addWidget(passwordEdit);

    confirmPasswordEdit = new QLineEdit();
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setPlaceholderText("Confirmar contraseña");
    confirmPasswordEdit->setMinimumHeight(34);
    connect(confirmPasswordEdit, &QLineEdit::returnPressed, this, &RegisterWindow::onRegisterClicked);
    cardLayout->addWidget(confirmPasswordEdit);

    cardLayout->addSpacing(4);

    registerButton = new QPushButton("Registrarse");
    registerButton->setMinimumHeight(36);
    connect(registerButton, &QPushButton::clicked, this, &RegisterWindow::onRegisterClicked);
    cardLayout->addWidget(registerButton);

    backButton = new QPushButton("Volver al login");
    backButton->setObjectName("secondaryButton");
    backButton->setMinimumHeight(34);
    connect(backButton, &QPushButton::clicked, this, &RegisterWindow::onBackClicked);
    cardLayout->addWidget(backButton);

    auto hCenter = new QHBoxLayout();
    hCenter->addStretch();
    hCenter->addWidget(card);
    hCenter->addStretch();
    outer->addLayout(hCenter);
    outer->addStretch(1);
}

bool RegisterWindow::validateInputs() {
    QString username = usernameEdit->text().trimmed();
    QString email = emailEdit->text().trimmed();
    QString password = passwordEdit->text();
    QString confirmPassword = confirmPasswordEdit->text();
    
    if (username.isEmpty() || email.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "All fields are required.");
        return false;
    }
    
    if (username.length() < 3) {
        QMessageBox::warning(this, "Validation Error", "Username must be at least 3 characters.");
        return false;
    }
    
    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    if (!emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "Validation Error", "Invalid email format.");
        return false;
    }
    
    if (password.length() < 8) {
        QMessageBox::warning(this, "Validation Error", "Password must be at least 8 characters.");
        return false;
    }
    
    if (password != confirmPassword) {
        QMessageBox::warning(this, "Validation Error", "Passwords do not match.");
        return false;
    }
    
    return true;
}

void RegisterWindow::onRegisterClicked() {
    if (!validateInputs()) {
        return;
    }
    
    emit registerRequested(usernameEdit->text().trimmed(),
                          emailEdit->text().trimmed(),
                          passwordEdit->text());
}

void RegisterWindow::onBackClicked() {
    emit backToLogin();
}
