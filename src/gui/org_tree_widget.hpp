#pragma once

#include <QTreeWidget>
#include "api/app_context.hpp"

namespace dms {

class OrgTreeWidget : public QTreeWidget {
    Q_OBJECT
public:
    explicit OrgTreeWidget(AppContext& app, QWidget* parent = nullptr);

    void populate();

signals:
    void unitSelected(const QString& unitId);

private slots:
    void onItemClicked(QTreeWidgetItem* item, int column);

private:
    void addChildren(QTreeWidgetItem* parentItem, const std::string& parentUnitId);

    AppContext& app_;
};

} // namespace dms
