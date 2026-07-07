#include "main_window.hpp"
#include "org_tree_widget.hpp"
#include "document_table_widget.hpp"
#include "search_panel.hpp"
#include "admin_panel.hpp"
#include "login_dialog.hpp"

#include <QSplitter>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QApplication>
#include <QLabel>
#include <QFrame>
#include <QToolButton>

namespace dms {

MainWindow::MainWindow(AppContext& app, const QString& userId, QWidget* parent)
    : QMainWindow(parent), app_(app), currentUserId_(userId)
{
    setWindowTitle("Document Management System - Hierarchical Access Control");
    resize(1200, 750);

    // Global application style
    setStyleSheet(
        "QMainWindow { background: #ffffff; }"
        "QToolBar { background: #343a40; border: none; spacing: 4px; padding: 4px 8px; }"
        "QToolBar QToolButton { color: #f8f9fa; padding: 6px 12px; border-radius: 4px; "
        "  font-size: 12px; margin: 2px; }"
        "QToolBar QToolButton:hover { background: #495057; }"
        "QToolBar QToolButton:pressed { background: #212529; }"
        "QStatusBar { background: #f8f9fa; border-top: 1px solid #dee2e6; "
        "  color: #495057; font-size: 11px; padding: 4px; }");

    setupToolbar();
    setupPages();
    setupStatusBar();

    showDocuments();
    populateOrgTree();
}

void MainWindow::setupToolbar()
{
    auto* toolbar = addToolBar("Navigation");
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(20, 20));
    toolbar->setToolButtonStyle(Qt::ToolButtonTextOnly);

    // App title
    auto* titleLabel = new QLabel("  DMS  ");
    titleLabel->setStyleSheet("color: #ffffff; font-weight: bold; font-size: 14px; padding: 0 8px;");
    toolbar->addWidget(titleLabel);

    auto* sep = new QFrame();
    sep->setFrameShape(QFrame::VLine);
    sep->setStyleSheet("color: #6c757d; margin: 4px 8px;");
    toolbar->addWidget(sep);

    docsAction_ = toolbar->addAction("Documents");
    connect(docsAction_, &QAction::triggered, this, &MainWindow::showDocuments);

    searchAction_ = toolbar->addAction("Search");
    connect(searchAction_, &QAction::triggered, this, &MainWindow::showSearch);

    // Admin only visible for SYS_ADMIN
    auto user = app_.userRepo().findById(currentUserId_.toStdString());
    if (user && user->role == Role::SysAdmin) {
        adminAction_ = toolbar->addAction("Admin");
        connect(adminAction_, &QAction::triggered, this, &MainWindow::showAdmin);
    } else {
        adminAction_ = nullptr;
    }

    // Right-align logout
    auto* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolbar->addWidget(spacer);

    // User info in toolbar
    if (user) {
        QString userInfo = QString("%1 (%2)")
            .arg(QString::fromStdString(user->username))
            .arg(QString::fromStdString(roleToString(user->role)));
        auto* userLabel = new QLabel(userInfo);
        userLabel->setStyleSheet("color: #adb5bd; font-size: 11px; padding: 0 8px;");
        toolbar->addWidget(userLabel);
    }

    auto* logoutAction = toolbar->addAction("Logout");
    connect(logoutAction, &QAction::triggered, this, &MainWindow::onLogout);
}

void MainWindow::setupStatusBar()
{
    auto user = app_.userRepo().findById(currentUserId_.toStdString());
    if (user) {
        auto unit = app_.orgRepo().findById(user->unitId);
        QString unitName = unit ? QString::fromStdString(unit->name) : "?";

        auto formatBytes = [](uint64_t bytes) -> QString {
            if (bytes == 0) return "0";
            if (bytes < 1024) return QString::number(bytes) + " B";
            if (bytes < 1024 * 1024) return QString::number(bytes / 1024.0, 'f', 1) + " KB";
            return QString::number(bytes / (1024.0 * 1024.0), 'f', 1) + " MB";
        };

        QString status = QString("Unit: %1  |  Quota: %2 / %3  |  Role: %4")
            .arg(unitName)
            .arg(formatBytes(user->quotaUsed))
            .arg(formatBytes(user->quotaLimit))
            .arg(QString::fromStdString(roleToString(user->role)));
        statusBar()->showMessage(status);
    }
}

void MainWindow::setupPages()
{
    stackedWidget_ = new QStackedWidget(this);
    setCentralWidget(stackedWidget_);

    // Page 0: Documents (Splitter with OrgTree + DocTable)
    documentsPage_ = new QWidget();
    auto* docLayout = new QHBoxLayout(documentsPage_);
    docLayout->setContentsMargins(0, 0, 0, 0);
    docLayout->setSpacing(0);

    auto* splitter = new QSplitter(Qt::Horizontal);
    splitter->setHandleWidth(1);
    splitter->setStyleSheet("QSplitter::handle { background: #dee2e6; }");

    orgTree_ = new OrgTreeWidget(app_);
    orgTree_->setMinimumWidth(200);
    orgTree_->setMaximumWidth(300);
    splitter->addWidget(orgTree_);

    docTable_ = new DocumentTableWidget(app_, currentUserId_);
    splitter->addWidget(docTable_);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 4);

    docLayout->addWidget(splitter);
    stackedWidget_->addWidget(documentsPage_);

    // Page 1: Search
    searchPanel_ = new SearchPanel(app_, currentUserId_);
    stackedWidget_->addWidget(searchPanel_);

    // Page 2: Admin
    adminPanel_ = new AdminPanel(app_, currentUserId_);
    stackedWidget_->addWidget(adminPanel_);

    // Connect org tree selection to doc table
    connect(orgTree_, &OrgTreeWidget::unitSelected,
            this, &MainWindow::onUnitSelected);
}

void MainWindow::populateOrgTree()
{
    orgTree_->populate();

    auto user = app_.userRepo().findById(currentUserId_.toStdString());
    if (user) {
        onUnitSelected(QString::fromStdString(user->unitId));
    }
}

void MainWindow::showDocuments()
{
    stackedWidget_->setCurrentIndex(0);
    updateActiveTab(0);
}

void MainWindow::showSearch()
{
    stackedWidget_->setCurrentIndex(1);
    updateActiveTab(1);
}

void MainWindow::showAdmin()
{
    auto user = app_.userRepo().findById(currentUserId_.toStdString());
    if (!user || user->role != Role::SysAdmin) {
        QMessageBox::warning(this, "Access Denied", "Only SYS_ADMIN can access admin panel");
        return;
    }
    adminPanel_->refresh();
    stackedWidget_->setCurrentIndex(2);
    updateActiveTab(2);
}

void MainWindow::updateActiveTab(int pageIndex)
{
    const QString activeStyle =
        "color: #ffffff; background: #495057; padding: 6px 14px; "
        "border-radius: 4px; font-weight: bold; font-size: 12px;";
    const QString inactiveStyle =
        "color: #f8f9fa; padding: 6px 12px; border-radius: 4px; font-size: 12px;";

    // Helper to style a QAction's associated widget
    auto styleAction = [&](QAction* action, bool active) {
        if (!action) return;
        QList<QWidget*> widgets = action->associatedWidgets();
        for (QWidget* w : widgets) {
            if (auto* btn = qobject_cast<QToolButton*>(w)) {
                btn->setStyleSheet(active ? activeStyle : inactiveStyle);
            }
        }
    };

    styleAction(docsAction_, pageIndex == 0);
    styleAction(searchAction_, pageIndex == 1);
    styleAction(adminAction_, pageIndex == 2);
}

void MainWindow::onLogout()
{
    app_.session().logout();
    close();

    LoginDialog loginDlg(app_);
    if (loginDlg.exec() == QDialog::Accepted) {
        auto* newWindow = new MainWindow(app_, loginDlg.loggedInUserId());
        newWindow->show();
    } else {
        QApplication::quit();
    }
}

void MainWindow::onUnitSelected(const QString& unitId)
{
    docTable_->refreshForUnit(unitId);
}

void MainWindow::refreshAll()
{
    populateOrgTree();
    setupStatusBar();
    if (adminPanel_) adminPanel_->refresh();
}

} // namespace dms
