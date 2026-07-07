#pragma once

#include <QWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QTreeWidget>

#include "api/app_context.hpp"

namespace dms {

class AdminPanel : public QWidget {
    Q_OBJECT
public:
    explicit AdminPanel(AppContext& app, const QString& userId, QWidget* parent = nullptr);

    void refresh();

private slots:
    void onCreateUser();
    void onChangeRole();
    void onSetQuota();
    void onDeleteUser();
    void onCreateUnit();
    void onDeleteUnit();

private:
    void refreshUserTable();
    void refreshUnitTree();

    AppContext& app_;
    QString userId_;

    QTabWidget* tabs_;
    QTableWidget* userTable_;
    QTreeWidget* unitTree_;
};

} // namespace dms
