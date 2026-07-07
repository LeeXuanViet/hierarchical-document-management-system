#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

#include "api/app_context.hpp"

namespace dms {

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(AppContext& app, QWidget* parent = nullptr);

    QString loggedInUserId() const { return loggedInUserId_; }
    QString loggedInUsername() const { return loggedInUsername_; }

private slots:
    void onLogin();

private:
    AppContext& app_;
    QLineEdit* usernameEdit_;
    QLineEdit* passwordEdit_;
    QLabel* errorLabel_;
    QString loggedInUserId_;
    QString loggedInUsername_;
};

} // namespace dms
