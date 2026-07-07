#include "search_panel.hpp"
#include "document_detail_dialog.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

namespace dms {

SearchPanel::SearchPanel(AppContext& app, const QString& userId, QWidget* parent)
    : QWidget(parent), app_(app), userId_(userId)
{
    auto* layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel("Search Documents"));

    // Search bar
    auto* searchLayout = new QHBoxLayout();

    searchEdit_ = new QLineEdit();
    searchEdit_->setPlaceholderText("Enter search query...");
    searchLayout->addWidget(searchEdit_);

    titleRadio_ = new QRadioButton("By Title");
    titleRadio_->setChecked(true);
    searchLayout->addWidget(titleRadio_);

    contentRadio_ = new QRadioButton("By Content");
    searchLayout->addWidget(contentRadio_);

    auto* searchBtn = new QPushButton("Search");
    connect(searchBtn, &QPushButton::clicked, this, &SearchPanel::onSearch);
    connect(searchEdit_, &QLineEdit::returnPressed, this, &SearchPanel::onSearch);
    searchLayout->addWidget(searchBtn);

    layout->addLayout(searchLayout);

    // Results
    resultList_ = new QListWidget();
    connect(resultList_, &QListWidget::itemDoubleClicked, this, &SearchPanel::onResultDoubleClicked);
    layout->addWidget(resultList_);
}

void SearchPanel::onSearch()
{
    QString query = searchEdit_->text().trimmed();
    if (query.isEmpty()) return;

    resultList_->clear();

    std::string q = query.toStdString();
    std::string uid = userId_.toStdString();

    std::vector<Document> results;
    if (titleRadio_->isChecked()) {
        results = app_.search().searchByTitle(uid, q);
    } else {
        results = app_.search().searchByContent(uid, q);
    }

    if (results.empty()) {
        resultList_->addItem("No results found.");
        return;
    }

    for (const auto& doc : results) {
        auto owner = app_.userRepo().findById(doc.ownerUserId);
        QString ownerName = owner ? QString::fromStdString(owner->username) : "?";
        QString reason = QString::fromStdString(
            app_.perms().explainReadAccess(uid, doc.id));

        QString text = QString("[%1] %2 (by %3) [%4]")
            .arg(reason)
            .arg(QString::fromStdString(doc.title))
            .arg(ownerName)
            .arg(QString::fromStdString(visibilityToString(doc.visibility)));

        auto* item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, QString::fromStdString(doc.id));
        resultList_->addItem(item);
    }
}

void SearchPanel::onResultDoubleClicked(QListWidgetItem* item)
{
    QString docId = item->data(Qt::UserRole).toString();
    if (docId.isEmpty()) return;

    DocumentDetailDialog dlg(app_, userId_, docId, this);
    dlg.exec();
}

} // namespace dms
