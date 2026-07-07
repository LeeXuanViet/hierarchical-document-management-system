#pragma once

#include <QDialog>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

#include "api/app_context.hpp"

namespace dms {

class DocumentDetailDialog : public QDialog {
    Q_OBJECT
public:
    DocumentDetailDialog(AppContext& app, const QString& userId,
                         const QString& docId, QWidget* parent = nullptr);

private slots:
    void onSave();

private:
    AppContext& app_;
    QString userId_;
    QString docId_;

    QLineEdit* titleEdit_;
    QComboBox* visibilityCombo_;
    QTextEdit* contentEdit_;
    QLabel* metaLabel_;

    bool canEdit_;
};

} // namespace dms
