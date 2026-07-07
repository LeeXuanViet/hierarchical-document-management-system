#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>

#include "api/app_context.hpp"

namespace dms {

class OrgTreeWidget;
class DocumentTableWidget;
class SearchPanel;
class AdminPanel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(AppContext& app, const QString& userId, QWidget* parent = nullptr);

    void refreshAll();

private slots:
    void showDocuments();
    void showSearch();
    void showAdmin();
    void onLogout();
    void onUnitSelected(const QString& unitId);

private:
    void setupToolbar();
    void setupStatusBar();
    void setupPages();
    void populateOrgTree();

    AppContext& app_;
    QString currentUserId_;

    QStackedWidget* stackedWidget_;
    QWidget* documentsPage_;
    SearchPanel* searchPanel_;
    AdminPanel* adminPanel_;

    OrgTreeWidget* orgTree_;
    DocumentTableWidget* docTable_;

    QAction* docsAction_;
    QAction* searchAction_;
    QAction* adminAction_;

    void updateActiveTab(int pageIndex);
};

} // namespace dms
