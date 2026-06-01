#include "registerwindow.h"
#include "ui_registerwindow.h"
#include <QMessageBox>
#include <QRegularExpression>

RegisterWindow::RegisterWindow(QWidget *parent)
    : QWidget(parent, Qt::Window)
    , ui(new Ui::RegisterWindow)
{
    ui->setupUi(this);
    connect(ui->registerButton,     &QPushButton::clicked,     this, &RegisterWindow::onRegisterClicked);
    connect(ui->backButton,         &QPushButton::clicked,     this, &RegisterWindow::onBackClicked);
    connect(ui->confirmPasswordEdit,&QLineEdit::returnPressed, this, &RegisterWindow::onRegisterClicked);
}

RegisterWindow::~RegisterWindow()
{
    delete ui;
}

bool RegisterWindow::validateInputs()
{
    QString username        = ui->usernameEdit->text().trimmed();
    QString email           = ui->emailEdit->text().trimmed();
    QString password        = ui->passwordEdit->text();
    QString confirmPassword = ui->confirmPasswordEdit->text();

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

void RegisterWindow::onRegisterClicked()
{
    if (!validateInputs())
        return;
    emit registerRequested(ui->usernameEdit->text().trimmed(),
                           ui->emailEdit->text().trimmed(),
                           ui->passwordEdit->text());
}

void RegisterWindow::onBackClicked()
{
    emit backToLogin();
}
