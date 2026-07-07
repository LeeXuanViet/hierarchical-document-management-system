#include "org_tree_widget.hpp"

#include <QHeaderView>

namespace dms {

OrgTreeWidget::OrgTreeWidget(AppContext& app, QWidget* parent)
    : QTreeWidget(parent), app_(app)
{
    setHeaderLabel("Organization Structure");
    setMinimumWidth(220);
    setAnimated(true);
    setIndentation(16);

    setStyleSheet(
        "QTreeWidget { border: none; background: #f8f9fa; font-size: 12px; }"
        "QTreeWidget::item { padding: 4px 2px; border-radius: 3px; }"
        "QTreeWidget::item:selected { background: #cce5ff; color: #004085; }"
        "QTreeWidget::item:hover { background: #e2e6ea; }"
        "QHeaderView::section { background: #e9ecef; border: none; "
        "  padding: 8px; font-weight: bold; font-size: 12px; "
        "  border-bottom: 2px solid #dee2e6; }");

    connect(this, &QTreeWidget::itemClicked, this, &OrgTreeWidget::onItemClicked);
}

void OrgTreeWidget::populate()
{
    clear();

    auto allUnits = app_.orgRepo().findAll();
    for (const auto& unit : allUnits) {
        if (unit.parentUnitId.empty()) {
            auto* item = new QTreeWidgetItem(this);
            item->setText(0, QString::fromStdString(unit.name));
            item->setData(0, Qt::UserRole, QString::fromStdString(unit.id));
            item->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
            addChildren(item, unit.id);
            item->setExpanded(true);
        }
    }
}

void OrgTreeWidget::addChildren(QTreeWidgetItem* parentItem, const std::string& parentUnitId)
{
    auto unit = app_.orgRepo().findById(parentUnitId);
    if (!unit) return;

    for (const auto& childId : unit->childrenIds) {
        auto child = app_.orgRepo().findById(childId);
        if (!child) continue;

        auto* childItem = new QTreeWidgetItem(parentItem);
        childItem->setText(0, QString::fromStdString(child->name));
        childItem->setData(0, Qt::UserRole, QString::fromStdString(child->id));
        bool hasChildren = !child->childrenIds.empty();
        childItem->setIcon(0, style()->standardIcon(
            hasChildren ? QStyle::SP_DirOpenIcon : QStyle::SP_DirClosedIcon));
        addChildren(childItem, child->id);
        childItem->setExpanded(true);
    }
}

void OrgTreeWidget::onItemClicked(QTreeWidgetItem* item, int /*column*/)
{
    QString unitId = item->data(0, Qt::UserRole).toString();
    if (!unitId.isEmpty()) {
        emit unitSelected(unitId);
    }
}

} // namespace dms
