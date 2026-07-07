#pragma once

#include <QDialog>
#include <QComboBox>
#include <QTableWidget>
#include <QPushButton>
#include <QRadioButton>

#include "api/app_context.hpp"

namespace dms {

class ShareDialog : public QDialog {
    Q_OBJECT
public:
    ShareDialog(AppContext& app, const QString& userId,
                const QString& docId, QWidget* parent = nullptr);

private slots:
    void onShare();
    void onRevoke();
    void refreshShareList();

private:
    AppContext& app_;
    QString userId_;
    QString docId_;

    QComboBox* userCombo_;
    QRadioButton* readRadio_;
    QRadioButton* editRadio_;
    QTableWidget* shareTable_;
};

} // namespace dms
