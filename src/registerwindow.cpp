#include "registerwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QRegularExpression>

RegisterWindow::RegisterWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Chess Insight AI - Register");
    setMinimumSize(400, 450);
    
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Título
    auto titleLabel = new QLabel("Create New Account");
    auto titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);
    
    mainLayout->addSpacing(15);
    
    // Usuario
    auto userLabel = new QLabel("Username:");
    usernameEdit = new QLineEdit();
    usernameEdit->setPlaceholderText("Choose a username");
    mainLayout->addWidget(userLabel);
    mainLayout->addWidget(usernameEdit);
    
    // Email
    auto emailLabel = new QLabel("Email:");
    emailEdit = new QLineEdit();
    emailEdit->setPlaceholderText("your@email.com");
    mainLayout->addWidget(emailLabel);
    mainLayout->addWidget(emailEdit);
    
    // Contraseña
    auto passLabel = new QLabel("Password:");
    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("At least 8 characters");
    mainLayout->addWidget(passLabel);
    mainLayout->addWidget(passwordEdit);
    
    // Confirmar contraseña
    auto confirmLabel = new QLabel("Confirm Password:");
    confirmPasswordEdit = new QLineEdit();
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    mainLayout->addWidget(confirmLabel);
    mainLayout->addWidget(confirmPasswordEdit);
    
    mainLayout->addSpacing(20);
    
    // Botones
    auto buttonLayout = new QHBoxLayout();
    
    registerButton = new QPushButton("Register");
    registerButton->setMinimumHeight(40);
    connect(registerButton, &QPushButton::clicked, this, &RegisterWindow::onRegisterClicked);
    buttonLayout->addWidget(registerButton);
    
    backButton = new QPushButton("Back to Login");
    backButton->setMinimumHeight(40);
    connect(backButton, &QPushButton::clicked, this, &RegisterWindow::onBackClicked);
    buttonLayout->addWidget(backButton);
    
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch();
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
