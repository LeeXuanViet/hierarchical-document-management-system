#include "document_detail_dialog.hpp"
#include "pdf_text_extractor.hpp"
#include "pdf_renderer.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QScrollArea>
#include <QPixmap>

#include <algorithm>

namespace dms {

DocumentDetailDialog::DocumentDetailDialog(AppContext& app, const QString& userId,
                                           const QString& docId, QWidget* parent)
    : QDialog(parent), app_(app), userId_(userId), docId_(docId)
{
    setWindowTitle("Document Details");
    setMinimumSize(720, 520);

    auto doc = app_.docRepo().findById(docId.toStdString());
    if (!doc) {
        QMessageBox::warning(this, "Error", "Document not found");
        reject();
        return;
    }

    // Check permission
    std::string reason = app_.perms().explainReadAccess(userId.toStdString(), docId.toStdString());
    if (reason == "DENIED") {
        QMessageBox::warning(this, "Access Denied", "You don't have permission to view this document");
        reject();
        return;
    }

    // Can edit if OWNER or has EDIT share permission or SYS_ADMIN
    canEdit_ = (reason == "OWNER" || reason == "SYS_ADMIN");
    if (!canEdit_) {
        // Check if shared with EDIT permission
        auto shares = app_.shareRepo().findByDocument(docId.toStdString());
        for (const auto& s : shares) {
            if (s.toUserId == userId.toStdString() && s.permission == SharePermission::Edit) {
                canEdit_ = true;
                break;
            }
        }
    }

    auto* layout = new QVBoxLayout(this);

    // Metadata
    auto owner = app_.userRepo().findById(doc->ownerUserId);
    QString ownerName = owner ? QString::fromStdString(owner->username) : "?";
    metaLabel_ = new QLabel(QString("Owner: %1 | Created: %2 | Size: %3 B | Access: %4")
        .arg(ownerName)
        .arg(QString::fromStdString(doc->createdAt))
        .arg(doc->size)
        .arg(QString::fromStdString(reason)));
    metaLabel_->setWordWrap(true);
    layout->addWidget(metaLabel_);

    // Form
    auto* form = new QFormLayout();

    titleEdit_ = new QLineEdit(QString::fromStdString(doc->title));
    titleEdit_->setReadOnly(!canEdit_);
    form->addRow("Title:", titleEdit_);

    visibilityCombo_ = new QComboBox();
    visibilityCombo_->addItem("Personal", static_cast<int>(VisibilityType::Personal));
    visibilityCombo_->addItem("UnitPublic", static_cast<int>(VisibilityType::UnitPublic));
    int idx = (doc->visibility == VisibilityType::UnitPublic) ? 1 : 0;
    visibilityCombo_->setCurrentIndex(idx);
    visibilityCombo_->setEnabled(canEdit_);
    form->addRow("Visibility:", visibilityCombo_);

    layout->addLayout(form);

    // Content
    contentEdit_ = new QTextEdit();
    contentEdit_->setStyleSheet(
        "QTextEdit { border: 1px solid #dee2e6; border-radius: 6px; "
        "padding: 10px; font-size: 13px; background: #f8f9fa; }");

    QString titleLower = QString::fromStdString(doc->title).toLower();
    bool isPdf = titleLower.endsWith(".pdf");
    bool isImage = titleLower.endsWith(".png") || titleLower.endsWith(".jpg") ||
                   titleLower.endsWith(".jpeg") || titleLower.endsWith(".gif") ||
                   titleLower.endsWith(".bmp");
    bool isOffice = titleLower.endsWith(".doc") || titleLower.endsWith(".docx") ||
                    titleLower.endsWith(".xls") || titleLower.endsWith(".xlsx") ||
                    titleLower.endsWith(".ppt") || titleLower.endsWith(".pptx");
    bool isArchive = titleLower.endsWith(".zip") || titleLower.endsWith(".rar") ||
                     titleLower.endsWith(".7z");
    bool knownBinary = isPdf || isImage || isOffice || isArchive ||
                       titleLower.endsWith(".svg") ||
                       titleLower.endsWith(".exe") || titleLower.endsWith(".bin");

    std::string content = app_.fileStore().read(doc->contentPath);
    bool hasNullByte = false;
    int checkLen = std::min(content.size(), static_cast<size_t>(1024));
    for (int i = 0; i < checkLen; ++i) {
        if (content[i] == '\0') {
            hasNullByte = true;
            break;
        }
    }

    if (isPdf) {
        // Real PDF preview via poppler-utils (pdftoppm + pdfinfo).
        // Renders ALL pages (maxPages=0). Text extraction is NOT shown on
        // screen — it is only used internally by the search index.
        canEdit_ = false;
        contentEdit_->setReadOnly(true);
        PdfRenderResult rendered = renderPdf(content, 0, 130);

        // Metadata header (from pure-C++ extractor as a reliable fallback).
        PdfInfo info = extractPdfInfo(content, 16384);
        int pageCount = rendered.pageCount > 0 ? rendered.pageCount : info.pageCount;

        QString html;
        html += "<div style='font-family:Segoe UI,Arial,sans-serif; padding:10px; color:#343a40;'>";
        html += "<h2 style='margin:0 0 6px 0; color:#495057;'>" +
                QString::fromStdString(doc->title).toHtmlEscaped() + "</h2>";
        html += "<table style='font-size:12px; color:#495057; margin:4px 0 8px 0;'>";
        auto row = [&](const QString& k, const QString& v) {
            html += "<tr><td style='padding:2px 8px 2px 0; color:#6c757d;'>" + k +
                    "</td><td style='padding:2px 0;'>" + v.toHtmlEscaped() + "</td></tr>";
        };
        row("Type", "PDF document");
        row("Size", QString::number(doc->size) + " bytes");
        if (pageCount > 0) row("Pages", QString::number(pageCount));
        if (!info.title.empty()) row("Title", QString::fromStdString(info.title));
        if (!info.author.empty()) row("Author", QString::fromStdString(info.author));
        if (!info.producer.empty()) row("Producer", QString::fromStdString(info.producer));
        if (info.encrypted) row("Encrypted", "yes");
        html += "</table>";

        if (info.encrypted) {
            html += "<p style='color:#856404; background:#fff3cd; border:1px solid #ffeeba; "
                    "border-radius:4px; padding:8px; font-size:12px;'>"
                    "This PDF is encrypted. Preview is not available.</p>";
        }
        html += "</div>";

        // Rendered page images (if poppler produced any).
        if (!rendered.pageImages.empty()) {
            contentEdit_->hide();
            auto* imgScroll = new QScrollArea();
            imgScroll->setWidgetResizable(true);
            imgScroll->setStyleSheet(
                "QScrollArea { border: 1px solid #dee2e6; border-radius: 6px; "
                "background: #f8f9fa; }");
            imgScroll->setAlignment(Qt::AlignHCenter);
            auto* pagesContainer = new QWidget();
            auto* pagesLayout = new QVBoxLayout(pagesContainer);
            pagesLayout->setAlignment(Qt::AlignHCenter);
            pagesLayout->setSpacing(14);
            pagesLayout->setContentsMargins(12, 12, 12, 12);
            int maxW = 620;
            for (size_t i = 0; i < rendered.pageImages.size(); ++i) {
                auto* pageLbl = new QLabel();
                pageLbl->setAlignment(Qt::AlignCenter);
                pageLbl->setStyleSheet(
                    "background: white; border: 1px solid #ced4da; "
                    "border-radius: 4px; padding: 6px;");
                QPixmap scaled = rendered.pageImages[i]
                    .scaledToWidth(maxW, Qt::SmoothTransformation);
                pageLbl->setPixmap(scaled);
                pagesLayout->addWidget(pageLbl);

                auto* capLbl = new QLabel("Page " + QString::number(i + 1) +
                                          " of " + QString::number(pageCount));
                capLbl->setAlignment(Qt::AlignCenter);
                capLbl->setStyleSheet("color:#6c757d; font-size:11px; padding:0 0 6px 0;");
                pagesLayout->addWidget(capLbl);
            }
            imgScroll->setWidget(pagesContainer);
            layout->addWidget(imgScroll);
            // Metadata-only text panel (no extracted text shown).
            contentEdit_->show();
            contentEdit_->setHtml(html);
            contentEdit_->setMaximumHeight(140);
        } else {
            // No images rendered: show metadata + fallback message.
            contentEdit_->show();
            html += "<p style='color:#6c757d; font-size:12px; margin-top:8px;'>"
                    "No rendered preview available. "
                    "Ensure poppler-utils is installed.</p>";
            contentEdit_->setHtml(html);
        }
    } else if (isImage) {
        // Real image thumbnail rendered from stored bytes.
        canEdit_ = false;
        contentEdit_->setReadOnly(true);
        QPixmap pix;
        pix.loadFromData(reinterpret_cast<const uchar*>(content.data()),
                         static_cast<uint>(content.size()));
        if (!pix.isNull()) {
            contentEdit_->hide();
            auto* imgScroll = new QScrollArea();
            imgScroll->setWidgetResizable(true);
            imgScroll->setStyleSheet(
                "QScrollArea { border: 1px solid #dee2e6; border-radius: 6px; "
                "background: #f8f9fa; }");
            imgScroll->setAlignment(Qt::AlignCenter);
            auto* imgLabel = new QLabel();
            imgLabel->setAlignment(Qt::AlignCenter);
            imgLabel->setStyleSheet("background: #f8f9fa;");
            int maxW = std::max(300, 600);
            int maxH = std::max(300, 500);
            QPixmap scaled = pix.scaled(maxW, maxH, Qt::KeepAspectRatio,
                                        Qt::SmoothTransformation);
            imgLabel->setPixmap(scaled);
            imgScroll->setWidget(imgLabel);
            layout->addWidget(imgScroll);
        } else {
            contentEdit_->setHtml(QString(
                "<div style='font-family:Segoe UI,Arial,sans-serif; padding:12px;'>"
                "<h2 style='color:#343a40; margin:0 0 10px 0;'>Image preview</h2>"
                "<p><b>Name:</b> %1</p>"
                "<p><b>Size:</b> %2 bytes</p>"
                "<p style='color:#6c757d;'>The image data could not be decoded.</p>"
                "</div>")
                .arg(QString::fromStdString(doc->title).toHtmlEscaped())
                .arg(doc->size));
        }
    } else if (knownBinary || hasNullByte) {
        canEdit_ = false;
        contentEdit_->setReadOnly(true);
        QString type = "Binary/non-text file";
        if (isOffice) type = "Office document";
        else if (isArchive) type = "Archive file";
        contentEdit_->setHtml(QString(
            "<div style='font-family:Segoe UI,Arial,sans-serif; padding:12px;'>"
            "<h2 style='color:#343a40; margin:0 0 10px 0;'>File preview</h2>"
            "<p><b>Name:</b> %1</p>"
            "<p><b>Size:</b> %2 bytes</p>"
            "<p><b>Type:</b> %3</p>"
            "<p style='color:#6c757d;'>Raw/binary content is hidden, so Office/archive files are not shown as broken text.</p>"
            "</div>")
            .arg(QString::fromStdString(doc->title).toHtmlEscaped())
            .arg(doc->size)
            .arg(type));
    } else {
        contentEdit_->setPlainText(QString::fromStdString(content));
        contentEdit_->setReadOnly(!canEdit_);
    }
    layout->addWidget(contentEdit_);

    // Buttons
    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    if (canEdit_) {
        auto* saveBtn = new QPushButton("Save");
        connect(saveBtn, &QPushButton::clicked, this, &DocumentDetailDialog::onSave);
        btnLayout->addWidget(saveBtn);
    }

    auto* closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(closeBtn);

    layout->addLayout(btnLayout);
}

void DocumentDetailDialog::onSave()
{
    std::string uid = userId_.toStdString();
    std::string did = docId_.toStdString();
    std::string newTitle = titleEdit_->text().toStdString();
    std::string newContent = contentEdit_->toPlainText().toStdString();

    try {
        app_.docs().edit(uid, did, newTitle, newContent);
        QMessageBox::information(this, "Saved", "Document updated successfully");
        accept();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", QString::fromStdString(e.what()));
    }
}

} // namespace dms
