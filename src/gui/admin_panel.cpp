#include "admin_panel.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QLabel>

namespace dms {

AdminPanel::AdminPanel(AppContext& app, const QString& userId, QWidget* parent)
    : QWidget(parent), app_(app), userId_(userId)
{
    auto* layout = new QVBoxLayout(this);

    tabs_ = new QTabWidget();

    // === User Management Tab ===
    auto* userTab = new QWidget();
    auto* userLayout = new QVBoxLayout(userTab);

    userTable_ = new QTableWidget();
    userTable_->setColumnCount(5);
    userTable_->setHorizontalHeaderLabels({"Username", "Role", "Unit", "Quota Used", "Quota Limit"});
    userTable_->horizontalHeader()->setStretchLastSection(true);
    userTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    userTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    userLayout->addWidget(userTable_);

    auto* userBtnLayout = new QHBoxLayout();
    auto* createUserBtn = new QPushButton("Create User");
    auto* changeRoleBtn = new QPushButton("Change Role");
    auto* setQuotaBtn = new QPushButton("Set Quota");
    connect(createUserBtn, &QPushButton::clicked, this, &AdminPanel::onCreateUser);
    connect(changeRoleBtn, &QPushButton::clicked, this, &AdminPanel::onChangeRole);
    connect(setQuotaBtn, &QPushButton::clicked, this, &AdminPanel::onSetQuota);
    userBtnLayout->addWidget(createUserBtn);
    userBtnLayout->addWidget(changeRoleBtn);
    userBtnLayout->addWidget(setQuotaBtn);
    userBtnLayout->addStretch();
    userLayout->addLayout(userBtnLayout);

    tabs_->addTab(userTab, "User Management");

    // === Unit Management Tab ===
    auto* unitTab = new QWidget();
    auto* unitLayout = new QVBoxLayout(unitTab);

    unitTree_ = new QTreeWidget();
    unitTree_->setHeaderLabel("Organization Units");
    unitLayout->addWidget(unitTree_);

    auto* unitBtnLayout = new QHBoxLayout();
    auto* createUnitBtn = new QPushButton("Create Unit");
    connect(createUnitBtn, &QPushButton::clicked, this, &AdminPanel::onCreateUnit);
    unitBtnLayout->addWidget(createUnitBtn);
    unitBtnLayout->addStretch();
    unitLayout->addLayout(unitBtnLayout);

    tabs_->addTab(unitTab, "Unit Management");

    layout->addWidget(tabs_);

    refresh();
}

void AdminPanel::refresh()
{
    refreshUserTable();
    refreshUnitTree();
}

void AdminPanel::refreshUserTable()
{
    auto users = app_.userRepo().findAll();
    userTable_->setRowCount(static_cast<int>(users.size()));

    for (int i = 0; i < static_cast<int>(users.size()); ++i) {
        const auto& user = users[i];

        auto* nameItem = new QTableWidgetItem(QString::fromStdString(user.username));
        nameItem->setData(Qt::UserRole, QString::fromStdString(user.id));
        userTable_->setItem(i, 0, nameItem);

        userTable_->setItem(i, 1, new QTableWidgetItem(
            QString::fromStdString(roleToString(user.role))));

        auto unit = app_.orgRepo().findById(user.unitId);
        QString unitName = unit ? QString::fromStdString(unit->name) : "?";
        userTable_->setItem(i, 2, new QTableWidgetItem(unitName));

        userTable_->setItem(i, 3, new QTableWidgetItem(
            QString::number(user.quotaUsed) + " B"));
        userTable_->setItem(i, 4, new QTableWidgetItem(
            QString::number(user.quotaLimit) + " B"));
    }
}

void AdminPanel::refreshUnitTree()
{
    unitTree_->clear();
    auto allUnits = app_.orgRepo().findAll();

    // Build tree from root units
    for (const auto& unit : allUnits) {
        if (unit.parentUnitId.empty()) {
            auto* item = new QTreeWidgetItem(unitTree_);
            item->setText(0, QString::fromStdString(unit.name));
            item->setData(0, Qt::UserRole, QString::fromStdString(unit.id));

            // Add children recursively
            std::function<void(QTreeWidgetItem*, const std::string&)> addChildren =
                [&](QTreeWidgetItem* parent, const std::string& parentId) {
                    auto u = app_.orgRepo().findById(parentId);
                    if (!u) return;
                    for (const auto& childId : u->childrenIds) {
                        auto child = app_.orgRepo().findById(childId);
                        if (!child) continue;
                        auto* childItem = new QTreeWidgetItem(parent);
                        childItem->setText(0, QString::fromStdString(child->name));
                        childItem->setData(0, Qt::UserRole, QString::fromStdString(child->id));
                        addChildren(childItem, child->id);
                    }
                };
            addChildren(item, unit.id);
            item->setExpanded(true);
        }
    }
}

void AdminPanel::onCreateUser()
{
    bool ok = false;
    QString username = QInputDialog::getText(this, "Create User",
        "Username:", QLineEdit::Normal, "", &ok);
    if (!ok || username.isEmpty()) return;

    QString password = QInputDialog::getText(this, "Create User",
        "Password:", QLineEdit::Password, "", &ok);
    if (!ok || password.isEmpty()) return;

    // Select role
    QStringList roles = {"USER", "UNIT_ADMIN", "SYS_ADMIN"};
    QString roleSel = QInputDialog::getItem(this, "Create User",
        "Role:", roles, 0, false, &ok);
    if (!ok) return;

    // Select unit
    auto allUnits = app_.orgRepo().findAll();
    QStringList unitNames;
    QStringList unitIds;
    for (const auto& u : allUnits) {
        unitNames << QString::fromStdString(u.name);
        unitIds << QString::fromStdString(u.id);
    }
    QString unitSel = QInputDialog::getItem(this, "Create User",
        "Unit:", unitNames, 0, false, &ok);
    if (!ok) return;

    int unitIdx = unitNames.indexOf(unitSel);
    std::string unitId = unitIds[unitIdx].toStdString();

    // Quota limit (default 10 MB = 10485760 bytes)
    constexpr long long DEFAULT_QUOTA = 10LL * 1024 * 1024;
    QString quotaStr = QInputDialog::getText(this, "Create User",
        "Quota limit (bytes, default 10 MB):", QLineEdit::Normal,
        QString::number(DEFAULT_QUOTA), &ok);
    if (!ok) return;
    long long quotaLimit = DEFAULT_QUOTA;
    if (!quotaStr.isEmpty()) {
        bool convOk = false;
        long long val = quotaStr.toLongLong(&convOk);
        if (convOk && val >= 0) quotaLimit = val;
    }

    Role role = stringToRole(roleSel.toStdString());

    std::string newUserId = app_.users().createUser(
        username.toStdString(), password.toStdString(), role, unitId, quotaLimit);

    if (!newUserId.empty()) {
        QMessageBox::information(this, "Success",
            "User created: " + QString::fromStdString(newUserId));
        refreshUserTable();
    } else {
        QMessageBox::warning(this, "Error", "Failed to create user");
    }
}

void AdminPanel::onChangeRole()
{
    int row = userTable_->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Error", "Select a user first");
        return;
    }

    QString targetUserId = userTable_->item(row, 0)->data(Qt::UserRole).toString();
    QStringList roles = {"USER", "UNIT_ADMIN", "SYS_ADMIN"};
    bool ok = false;
    QString roleSel = QInputDialog::getItem(this, "Change Role",
        "New role:", roles, 0, false, &ok);
    if (!ok) return;

    Role newRole = stringToRole(roleSel.toStdString());

    try {
        app_.users().changeRole(targetUserId.toStdString(), newRole);
        QMessageBox::information(this, "Success", "Role updated");
        refreshUserTable();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", QString::fromStdString(e.what()));
    }
}

void AdminPanel::onSetQuota()
{
    int row = userTable_->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Error", "Select a user first");
        return;
    }

    QString targetUserId = userTable_->item(row, 0)->data(Qt::UserRole).toString();
    bool ok = false;
    int quota = QInputDialog::getInt(this, "Set Quota",
        "Quota limit (bytes):", 1048576, 0, 1073741824, 1024, &ok);
    if (!ok) return;

    try {
        app_.users().setQuotaLimit(targetUserId.toStdString(), static_cast<long long>(quota));
        QMessageBox::information(this, "Success", "Quota updated");
        refreshUserTable();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", QString::fromStdString(e.what()));
    }
}

void AdminPanel::onCreateUnit()
{
    bool ok = false;
    QString unitName = QInputDialog::getText(this, "Create Unit",
        "Unit name:", QLineEdit::Normal, "", &ok);
    if (!ok || unitName.isEmpty()) return;

    // Select parent
    auto allUnits = app_.orgRepo().findAll();
    QStringList unitNames;
    QStringList unitIds;
    unitNames << "(Root - no parent)";
    unitIds << "";
    for (const auto& u : allUnits) {
        unitNames << QString::fromStdString(u.name);
        unitIds << QString::fromStdString(u.id);
    }

    QString parentSel = QInputDialog::getItem(this, "Create Unit",
        "Parent unit:", unitNames, 0, false, &ok);
    if (!ok) return;

    int parentIdx = unitNames.indexOf(parentSel);
    std::string parentId = unitIds[parentIdx].toStdString();

    std::string newId = app_.orgTree().createUnit(unitName.toStdString(), parentId);
    if (!newId.empty()) {
        QMessageBox::information(this, "Success",
            "Unit created: " + QString::fromStdString(newId));
        refreshUnitTree();
    } else {
        QMessageBox::warning(this, "Error", "Failed to create unit");
    }
}

} // namespace dms
