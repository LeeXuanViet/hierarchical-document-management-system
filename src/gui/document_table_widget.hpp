#pragma once

#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QSplitter>
#include <QScrollArea>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

#include "api/app_context.hpp"
#include "pdf_text_extractor.hpp"
#include "pdf_renderer.hpp"

namespace dms {

class DocumentTableWidget : public QWidget {
    Q_OBJECT
public:
    explicit DocumentTableWidget(AppContext& app, const QString& userId, QWidget* parent = nullptr);

    void refreshForUnit(const QString& unitId);
    void refreshMyDocs();
    void refreshSharedWithMe();
    void refreshFavorites();

signals:
    void documentSelected(const QString& docId);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void onFilterChanged(int index);
    void onUploadClicked();
    void onUploadFileClicked();
    void onTableClicked(int row, int column);
    void onTableDoubleClicked(int row, int column);
    void onContextMenu(const QPoint& pos);

private:
    void populateTable(const std::vector<Document>& docs);
    QString getPermissionBadge(const std::string& docId);
    void uploadFile(const QString& filePath);
    void setDropHighlight(bool active);
    void showPreview(const QString& docId);
    QString formatSize(uint64_t bytes);
    QString buildPdfPreviewHtml(const std::string& content, const QString& title, uint64_t size);
    QString buildImagePreviewHtml(const std::string& content, const QString& title, uint64_t size);
    QString buildBinaryPreviewHtml(const QString& title, uint64_t size, const QString& type);

    AppContext& app_;
    QString currentUserId_;
    QString currentUnitId_;

    QSplitter* mainSplitter_;
    QTableWidget* table_;
    QComboBox* filterCombo_;
    QPushButton* uploadBtn_;
    QPushButton* uploadFileBtn_;
    QWidget* dropOverlay_;
    QLabel* statsLabel_;

    // Preview panel
    QWidget* previewPanel_;
    QLabel* previewTitle_;
    QLabel* previewMeta_;
    QTextEdit* previewContent_;
    QLabel* previewImageLabel_;   // shown for image thumbnails
    QScrollArea* previewScroll_;  // wraps previewImageLabel_ for large images
};

} // namespace dms
