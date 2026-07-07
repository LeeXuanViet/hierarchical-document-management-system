#include "login_dialog.hpp"

#include <QMessageBox>
#include <QFrame>

namespace dms {

LoginDialog::LoginDialog(AppContext& app, QWidget* parent)
    : QDialog(parent), app_(app)
{
    setWindowTitle("DMS - Login");
    setFixedSize(560, 460);
    setStyleSheet(
        "QDialog { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #667eea, stop:1 #764ba2); }");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(55, 45, 55, 45);

    // Card container
    auto* card = new QFrame();
    card->setStyleSheet(
        "QFrame { background: white; border-radius: 14px; padding: 26px; }");
    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(16);

    // Title
    auto* titleLabel = new QLabel("Document Management System");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setWordWrap(true);
    titleLabel->setMinimumHeight(42);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #343a40; padding: 8px 0;");
    cardLayout->addWidget(titleLabel);

    auto* subtitleLabel = new QLabel("Hierarchical Access Control");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet("font-size: 13px; color: #6c757d; margin-bottom: 8px;");
    cardLayout->addWidget(subtitleLabel);

    // Input fields
    usernameEdit_ = new QLineEdit();
    usernameEdit_->setPlaceholderText("Username");
    usernameEdit_->setStyleSheet(
        "QLineEdit { padding: 12px 14px; border: 1px solid #ced4da; border-radius: 6px; "
        "font-size: 14px; background: #f8f9fa; min-height: 22px; }"
        "QLineEdit:focus { border-color: #667eea; background: white; }");
    cardLayout->addWidget(usernameEdit_);

    passwordEdit_ = new QLineEdit();
    passwordEdit_->setPlaceholderText("Password");
    passwordEdit_->setEchoMode(QLineEdit::Password);
    passwordEdit_->setStyleSheet(
        "QLineEdit { padding: 12px 14px; border: 1px solid #ced4da; border-radius: 6px; "
        "font-size: 14px; background: #f8f9fa; min-height: 22px; }"
        "QLineEdit:focus { border-color: #667eea; background: white; }");
    cardLayout->addWidget(passwordEdit_);

    // Error label
    errorLabel_ = new QLabel();
    errorLabel_->setStyleSheet("color: #dc3545; font-size: 11px; padding: 2px 4px;");
    errorLabel_->setVisible(false);
    cardLayout->addWidget(errorLabel_);

    // Login button
    auto* loginBtn = new QPushButton("Login");
    loginBtn->setDefault(true);
    loginBtn->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "stop:0 #667eea, stop:1 #764ba2); color: white; padding: 12px; "
        "border: none; border-radius: 6px; font-size: 15px; font-weight: bold; min-height: 22px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "stop:0 #5a6fd6, stop:1 #6a4190); }"
        "QPushButton:pressed { background: #4a5cc8; }");
    cardLayout->addWidget(loginBtn);

    // Demo hint
    auto* hintLabel = new QLabel("Demo: admin/admin123, alice/alice123, bob/bob123");
    hintLabel->setAlignment(Qt::AlignCenter);
    hintLabel->setStyleSheet("color: #adb5bd; font-size: 12px; margin-top: 8px;");
    cardLayout->addWidget(hintLabel);

    layout->addWidget(card);

    connect(loginBtn, &QPushButton::clicked, this, &LoginDialog::onLogin);
    connect(passwordEdit_, &QLineEdit::returnPressed, this, &LoginDialog::onLogin);
    connect(usernameEdit_, &QLineEdit::returnPressed, [this]() { passwordEdit_->setFocus(); });

    usernameEdit_->setFocus();
}

void LoginDialog::onLogin()
{
    std::string username = usernameEdit_->text().toStdString();
    std::string password = passwordEdit_->text().toStdString();

    if (username.empty() || password.empty()) {
        errorLabel_->setText("Please enter username and password");
        errorLabel_->setVisible(true);
        return;
    }

    auto result = app_.session().login(username, password);
    if (result) {
        loggedInUserId_ = QString::fromStdString(*result);
        loggedInUsername_ = QString::fromStdString(username);
        accept();
    } else {
        errorLabel_->setText("Invalid username or password");
        errorLabel_->setVisible(true);
        passwordEdit_->clear();
        passwordEdit_->setFocus();
    }
}

} // namespace dms
