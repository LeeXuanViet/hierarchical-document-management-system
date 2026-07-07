#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QRadioButton>
#include <QListWidget>
#include <QPushButton>

#include "api/app_context.hpp"

namespace dms {

class SearchPanel : public QWidget {
    Q_OBJECT
public:
    explicit SearchPanel(AppContext& app, const QString& userId, QWidget* parent = nullptr);

signals:
    void documentSelected(const QString& docId);

private slots:
    void onSearch();
    void onResultDoubleClicked(QListWidgetItem* item);

private:
    AppContext& app_;
    QString userId_;

    QLineEdit* searchEdit_;
    QRadioButton* titleRadio_;
    QRadioButton* contentRadio_;
    QListWidget* resultList_;
};

} // namespace dms
