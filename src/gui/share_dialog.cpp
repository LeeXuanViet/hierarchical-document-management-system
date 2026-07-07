#include "share_dialog.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>

namespace dms {

ShareDialog::ShareDialog(AppContext& app, const QString& userId,
                         const QString& docId, QWidget* parent)
    : QDialog(parent), app_(app), userId_(userId), docId_(docId)
{
    setWindowTitle("Share Document");
    setMinimumSize(450, 350);

    auto* layout = new QVBoxLayout(this);

    // Document info
    auto doc = app_.docRepo().findById(docId.toStdString());
    if (doc) {
        layout->addWidget(new QLabel(
            QString("Sharing: <b>%1</b>").arg(QString::fromStdString(doc->title))));
    }

    // Share form
    auto* formGroup = new QGroupBox("Share with user");
    auto* formLayout = new QHBoxLayout(formGroup);

    userCombo_ = new QComboBox();
    // Populate with all users except current
    auto allUsers = app_.userRepo().findAll();
    for (const auto& u : allUsers) {
        if (u.id != userId.toStdString()) {
            userCombo_->addItem(QString::fromStdString(u.username),
                               QString::fromStdString(u.id));
        }
    }
    formLayout->addWidget(userCombo_);

    readRadio_ = new QRadioButton("Read");
    readRadio_->setChecked(true);
    formLayout->addWidget(readRadio_);

    editRadio_ = new QRadioButton("Edit");
    formLayout->addWidget(editRadio_);

    auto* shareBtn = new QPushButton("Share");
    connect(shareBtn, &QPushButton::clicked, this, &ShareDialog::onShare);
    formLayout->addWidget(shareBtn);

    layout->addWidget(formGroup);

    // Current shares table
    layout->addWidget(new QLabel("Current shares:"));

    shareTable_ = new QTableWidget();
    shareTable_->setColumnCount(3);
    shareTable_->setHorizontalHeaderLabels({"User", "Permission", "Action"});
    shareTable_->horizontalHeader()->setStretchLastSection(true);
    shareTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    shareTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(shareTable_);

    // Close button
    auto* closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(closeBtn);

    refreshShareList();
}

void ShareDialog::onShare()
{
    if (userCombo_->count() == 0) {
        QMessageBox::warning(this, "Error", "No users available to share with");
        return;
    }

    std::string toUserId = userCombo_->currentData().toString().toStdString();
    SharePermission perm = editRadio_->isChecked() ?
        SharePermission::Edit : SharePermission::Read;

    try {
        std::string shareId = app_.shares().share(
            userId_.toStdString(), docId_.toStdString(), toUserId, perm);
        if (!shareId.empty()) {
            QMessageBox::information(this, "Success", "Document shared successfully");
            refreshShareList();
        }
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", QString::fromStdString(e.what()));
    }
}

void ShareDialog::onRevoke()
{
    int row = shareTable_->currentRow();
    if (row < 0) return;

    auto* userItem = shareTable_->item(row, 0);
    if (!userItem) return;

    // Get the share ID stored in user role data
    QString shareId = userItem->data(Qt::UserRole).toString();
    if (shareId.isEmpty()) return;

    try {
        app_.shares().revoke(userId_.toStdString(), shareId.toStdString());
        refreshShareList();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", QString::fromStdString(e.what()));
    }
}

void ShareDialog::refreshShareList()
{
    auto shares = app_.shareRepo().findByDocument(docId_.toStdString());
    shareTable_->setRowCount(0);
    shareTable_->setRowCount(static_cast<int>(shares.size()));

    for (int i = 0; i < static_cast<int>(shares.size()); ++i) {
        const auto& share = shares[i];
        auto user = app_.userRepo().findById(share.toUserId);
        QString username = user ? QString::fromStdString(user->username) : "?";

        auto* userItem = new QTableWidgetItem(username);
        userItem->setData(Qt::UserRole, QString::fromStdString(share.id));
        shareTable_->setItem(i, 0, userItem);

        shareTable_->setItem(i, 1, new QTableWidgetItem(
            QString::fromStdString(sharePermissionToString(share.permission))));

        auto* revokeBtn = new QPushButton("Revoke");
        connect(revokeBtn, &QPushButton::clicked, [this, i]() {
            shareTable_->setCurrentCell(i, 0);
            onRevoke();
        });
        shareTable_->setCellWidget(i, 2, revokeBtn);
    }
}

} // namespace dms
