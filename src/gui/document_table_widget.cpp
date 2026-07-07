#include "document_table_widget.hpp"
#include "document_detail_dialog.hpp"
#include "share_dialog.hpp"

#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QFrame>

namespace dms {

DocumentTableWidget::DocumentTableWidget(AppContext& app, const QString& userId, QWidget* parent)
    : QWidget(parent), app_(app), currentUserId_(userId)
{
    setAcceptDrops(true);

    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(8, 8, 8, 8);

    // --- Toolbar row ---
    auto* toolbarFrame = new QFrame();
    toolbarFrame->setFrameShape(QFrame::StyledPanel);
    toolbarFrame->setStyleSheet(
        "QFrame { background: #f8f9fa; border: 1px solid #dee2e6; border-radius: 6px; padding: 4px; }");
    auto* toolbarLayout = new QHBoxLayout(toolbarFrame);
    toolbarLayout->setContentsMargins(8, 4, 8, 4);

    filterCombo_ = new QComboBox();
    filterCombo_->addItem("Unit Documents", 0);
    filterCombo_->addItem("My Documents", 1);
    filterCombo_->addItem("Shared With Me", 2);
    filterCombo_->addItem("Favorites", 3);
    filterCombo_->setMinimumWidth(160);
    filterCombo_->setStyleSheet(
        "QComboBox { padding: 4px 8px; border: 1px solid #ced4da; border-radius: 4px; }"
        "QComboBox:hover { border-color: #80bdff; }");
    toolbarLayout->addWidget(filterCombo_);

    toolbarLayout->addStretch();

    statsLabel_ = new QLabel("0 documents");
    statsLabel_->setStyleSheet("color: #6c757d; font-size: 11px;");
    toolbarLayout->addWidget(statsLabel_);

    toolbarLayout->addStretch();

    // Upload from file button
    uploadFileBtn_ = new QPushButton("Upload File...");
    uploadFileBtn_->setStyleSheet(
        "QPushButton { background: #28a745; color: white; padding: 6px 14px; "
        "border: none; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background: #218838; }"
        "QPushButton:pressed { background: #1e7e34; }");
    toolbarLayout->addWidget(uploadFileBtn_);

    // Upload text button
    uploadBtn_ = new QPushButton("Upload Text");
    uploadBtn_->setStyleSheet(
        "QPushButton { background: #007bff; color: white; padding: 6px 14px; "
        "border: none; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background: #0069d9; }"
        "QPushButton:pressed { background: #0062cc; }");
    toolbarLayout->addWidget(uploadBtn_);

    layout->addWidget(toolbarFrame);


    // --- Drop overlay (shown during drag) ---
    dropOverlay_ = new QWidget(this);
    dropOverlay_->setStyleSheet(
        "background: rgba(40, 167, 69, 0.12); border: 3px dashed #28a745; border-radius: 12px;");
    dropOverlay_->hide();

    // --- Main splitter: table on top, preview on bottom ---
    mainSplitter_ = new QSplitter(Qt::Vertical);
    mainSplitter_->setHandleWidth(3);
    mainSplitter_->setStyleSheet("QSplitter::handle { background: #dee2e6; }");

    // Table
    table_ = new QTableWidget();
    table_->setColumnCount(6);
    table_->setHorizontalHeaderLabels({"Title", "Owner", "Visibility", "Size", "Created", "Access"});
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setContextMenuPolicy(Qt::CustomContextMenu);
    table_->setAlternatingRowColors(true);
    table_->setShowGrid(false);
    table_->verticalHeader()->setVisible(false);
    table_->setStyleSheet(
        "QTableWidget { border: 1px solid #dee2e6; border-radius: 4px; }"
        "QTableWidget::item { padding: 4px 8px; }"
        "QTableWidget::item:selected { background: #cce5ff; color: #004085; }"
        "QHeaderView::section { background: #e9ecef; border: none; "
        "padding: 6px 8px; font-weight: bold; border-bottom: 2px solid #dee2e6; }");
    mainSplitter_->addWidget(table_);

    // Preview panel
    previewPanel_ = new QWidget();
    previewPanel_->setStyleSheet("background: #ffffff;");
    auto* previewLayout = new QVBoxLayout(previewPanel_);
    previewLayout->setContentsMargins(8, 6, 8, 6);
    previewLayout->setSpacing(4);

    // Preview header
    previewTitle_ = new QLabel("Preview");
    previewTitle_->setStyleSheet(
        "font-size: 13px; font-weight: bold; color: #343a40; padding: 4px 0; "
        "border-bottom: 1px solid #dee2e6;");
    previewLayout->addWidget(previewTitle_);

    previewMeta_ = new QLabel("");
    previewMeta_->setStyleSheet("font-size: 11px; color: #6c757d; padding: 2px 0;");
    previewMeta_->setWordWrap(true);
    previewLayout->addWidget(previewMeta_);

    previewContent_ = new QTextEdit();
    previewContent_->setReadOnly(true);
    previewContent_->setStyleSheet(
        "QTextEdit { border: 1px solid #e9ecef; border-radius: 4px; "
        "padding: 6px; font-family: monospace; font-size: 11px; background: #f8f9fa; }");
    previewContent_->setPlaceholderText("Select a document to preview its content...");
    previewLayout->addWidget(previewContent_);

    // Image preview (shown for image files; hidden otherwise)
    previewScroll_ = new QScrollArea();
    previewScroll_->setWidgetResizable(true);
    previewScroll_->setStyleSheet(
        "QScrollArea { border: 1px solid #e9ecef; border-radius: 4px; "
        "background: #f8f9fa; }");
    previewScroll_->setAlignment(Qt::AlignCenter);
    previewImageLabel_ = new QLabel();
    previewImageLabel_->setAlignment(Qt::AlignCenter);
    previewImageLabel_->setStyleSheet("background: #f8f9fa;");
    previewImageLabel_->setText("");
    previewScroll_->setWidget(previewImageLabel_);
    previewScroll_->hide();
    previewLayout->addWidget(previewScroll_);

    mainSplitter_->addWidget(previewPanel_);
    mainSplitter_->setStretchFactor(0, 3);
    mainSplitter_->setStretchFactor(1, 2);

    layout->addWidget(mainSplitter_);

    // Connections
    connect(filterCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DocumentTableWidget::onFilterChanged);
    connect(uploadBtn_, &QPushButton::clicked, this, &DocumentTableWidget::onUploadClicked);
    connect(uploadFileBtn_, &QPushButton::clicked, this, &DocumentTableWidget::onUploadFileClicked);
    connect(table_, &QTableWidget::cellClicked, this, &DocumentTableWidget::onTableClicked);
    connect(table_, &QTableWidget::cellDoubleClicked, this, &DocumentTableWidget::onTableDoubleClicked);
    connect(table_, &QTableWidget::customContextMenuRequested, this, &DocumentTableWidget::onContextMenu);
}

// --- Preview ---
void DocumentTableWidget::showPreview(const QString& docId)
{
    auto doc = app_.docRepo().findById(docId.toStdString());
    if (!doc) {
        previewTitle_->setText("Preview - Document not found");
        previewContent_->clear();
        previewMeta_->clear();
        return;
    }

    previewTitle_->setText("Preview: " + QString::fromStdString(doc->title));

    // Metadata line
    auto owner = app_.userRepo().findById(doc->ownerUserId);
    QString ownerName = owner ? QString::fromStdString(owner->username) : "?";
    QString access = getPermissionBadge(doc->id);
    previewMeta_->setText(
        QString("Owner: %1  |  Size: %2  |  Visibility: %3  |  Access: %4  |  Created: %5")
            .arg(ownerName)
            .arg(formatSize(doc->size))
            .arg(QString::fromStdString(visibilityToString(doc->visibility)))
            .arg(access)
            .arg(QString::fromStdString(doc->createdAt)));

    // Content preview
    std::string reason = app_.perms().explainReadAccess(currentUserId_.toStdString(), doc->id);
    if (reason == "DENIED") {
        previewContent_->setPlainText("[Access Denied - You don't have permission to view this content]");
        return;
    }

    QString title = QString::fromStdString(doc->title).toLower();
    bool isPdf = title.endsWith(".pdf");
    bool isImage = title.endsWith(".png") || title.endsWith(".jpg") ||
                   title.endsWith(".jpeg") || title.endsWith(".gif") ||
                   title.endsWith(".bmp");
    bool isSvg = title.endsWith(".svg");
    bool isOffice = title.endsWith(".doc") || title.endsWith(".docx") ||
                    title.endsWith(".xls") || title.endsWith(".xlsx") ||
                    title.endsWith(".ppt") || title.endsWith(".pptx");
    bool isArchive = title.endsWith(".zip") || title.endsWith(".rar") ||
                     title.endsWith(".7z");
    bool knownBinary = isPdf || isImage || isOffice || isArchive ||
                       title.endsWith(".exe") || title.endsWith(".bin");

    std::string content = app_.fileStore().read(doc->contentPath);

    // Check if content is likely binary. PDFs may not contain null bytes early,
    // so extension-based detection above is intentionally checked first.
    bool hasNullByte = false;
    int checkLen = std::min(content.size(), static_cast<size_t>(1024));
    for (int i = 0; i < checkLen; ++i) {
        if (content[i] == '\0') {
            hasNullByte = true;
            break;
        }
    }

    // Reset preview widgets: hide image view, show text view by default.
    previewScroll_->hide();
    previewImageLabel_->clear();
    previewContent_->show();

    if (isPdf) {
        // Try poppler-utils first for a real rendered preview.
        PdfRenderResult rendered = renderPdf(content, 2, 110);
        if (rendered.ok && !rendered.pageImages.empty()) {
            previewContent_->hide();
            // Build a vertical stack of page images inside the scroll area.
            int maxW = std::max(220, previewScroll_->width() - 24);
            QPixmap combined;
            int totalH = 0;
            for (const auto& pix : rendered.pageImages) {
                totalH += pix.height() + 12;
            }
            // Use a single QLabel with a stacked pixmap scaled to fit width.
            int maxH = std::max(220, previewScroll_->height() - 24);
            // Show first page scaled; remaining pages appended below via HTML.
            QPixmap first = rendered.pageImages.front()
                .scaled(maxW, maxH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            previewImageLabel_->setPixmap(first);
            previewImageLabel_->setText("");
            previewScroll_->show();

            // Caption panel: page count + size only (no extracted text on screen).
            QString html;
            html += "<div style='font-family:Segoe UI,Arial,sans-serif; padding:8px; color:#343a40;'>";
            html += "<b style='color:#495057;'>PDF preview</b> &mdash; ";
            html += QString::number(rendered.pageCount) + " page(s), ";
            html += formatSize(doc->size) + ". ";
            html += "Page 1 shown above";
            if (rendered.pageImages.size() > 1)
                html += " (+ " + QString::number(rendered.pageImages.size() - 1) + " more, double-click for all)";
            html += ".";
            html += "</div>";
            previewContent_->show();
            previewContent_->setHtml(html);
        } else {
            // Fallback to the pure-C++ extractor (metadata + uncompressed text).
            previewContent_->setHtml(buildPdfPreviewHtml(content,
                QString::fromStdString(doc->title), doc->size));
        }
    } else if (isImage) {
        // Render a real thumbnail from the stored bytes.
        QPixmap pix;
        pix.loadFromData(reinterpret_cast<const uchar*>(content.data()),
                         static_cast<uint>(content.size()));
        if (!pix.isNull()) {
            previewContent_->hide();
            // Scale to fit preview width while keeping aspect ratio.
            int maxW = std::max(200, previewScroll_->width() - 20);
            int maxH = std::max(200, previewScroll_->height() - 20);
            QPixmap scaled = pix.scaled(maxW, maxH, Qt::KeepAspectRatio,
                                        Qt::SmoothTransformation);
            previewImageLabel_->setPixmap(scaled);
            previewImageLabel_->setText("");
            previewScroll_->show();
        } else {
            previewContent_->setHtml(buildBinaryPreviewHtml(
                QString::fromStdString(doc->title), doc->size, "Image file"));
        }
    } else if (isSvg) {
        // SVG is XML text; show source as a lightweight preview.
        QString textContent = QString::fromStdString(content);
        if (textContent.size() > 4096) {
            textContent = textContent.left(4096) + "\n\n... [truncated]";
        }
        previewContent_->setPlainText(textContent);
    } else if (knownBinary || hasNullByte) {
        QString type = "Binary/non-text file";
        if (isOffice) type = "Office document";
        else if (isArchive) type = "Archive file";
        previewContent_->setHtml(buildBinaryPreviewHtml(
            QString::fromStdString(doc->title), doc->size, type));
    } else {
        // Show text content (limit to first 4KB for preview)
        QString textContent = QString::fromStdString(content);
        if (textContent.size() > 4096) {
            textContent = textContent.left(4096) + "\n\n... [truncated, double-click to view full content]";
        }
        previewContent_->setPlainText(textContent);
    }
}

// Build an HTML preview for a PDF: metadata + extracted text (if any).
QString DocumentTableWidget::buildPdfPreviewHtml(const std::string& content,
                                                 const QString& title, uint64_t size)
{
    PdfInfo info = extractPdfInfo(content, 8192);

    QString html;
    html += "<div style='font-family:Segoe UI,Arial,sans-serif; padding:10px; color:#343a40;'>";
    html += "<h2 style='margin:0 0 6px 0; color:#495057;'>" + title.toHtmlEscaped() + "</h2>";

    // Metadata table
    html += "<table style='font-size:12px; color:#495057; margin:4px 0 8px 0;'>";
    auto row = [&](const QString& k, const QString& v) {
        html += "<tr><td style='padding:2px 8px 2px 0; color:#6c757d;'>" + k +
                "</td><td style='padding:2px 0;'>" + v.toHtmlEscaped() + "</td></tr>";
    };
    row("Type", "PDF document");
    row("Size", formatSize(size));
    if (info.pageCount > 0) row("Pages", QString::number(info.pageCount));
    if (!info.title.empty()) row("Title", QString::fromStdString(info.title));
    if (!info.author.empty()) row("Author", QString::fromStdString(info.author));
    if (!info.subject.empty()) row("Subject", QString::fromStdString(info.subject));
    if (!info.producer.empty()) row("Producer", QString::fromStdString(info.producer));
    if (info.encrypted) row("Encrypted", "yes");
    html += "</table>";

    if (info.encrypted) {
        html += "<p style='color:#856404; background:#fff3cd; border:1px solid #ffeeba; "
                "border-radius:4px; padding:8px; font-size:12px;'>"
                "This PDF is encrypted. Preview is not available.</p>";
    } else {
        html += "<p style='color:#6c757d; font-size:12px; margin-top:8px;'>"
                "Install <code>poppler-utils</code> for full page rendering. "
                "Metadata is shown above. Double-click to open the detail view.</p>";
    }
    html += "</div>";
    return html;
}

// Build an HTML preview for a generic binary/non-text file.
QString DocumentTableWidget::buildBinaryPreviewHtml(const QString& title,
                                                    uint64_t size, const QString& type)
{
    return QString(
        "<div style='font-family:Segoe UI,Arial,sans-serif; padding:12px; color:#343a40;'>"
        "<h2 style='margin:0 0 8px 0; color:#495057;'>%1</h2>"
        "<p style='margin:6px 0;'><b>Type:</b> %2</p>"
        "<p style='margin:6px 0;'><b>Size:</b> %3</p>"
        "<p style='margin:12px 0 0 0; color:#6c757d;'>"
        "Raw/binary data is hidden to avoid showing broken bytes. "
        "Double-click to open the detail view with safe metadata preview."
        "</p>"
        "</div>")
        .arg(title.toHtmlEscaped())
        .arg(type)
        .arg(formatSize(size));
}

// --- Drag & Drop ---
void DocumentTableWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        setDropHighlight(true);
    }
}

void DocumentTableWidget::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void DocumentTableWidget::dragLeaveEvent(QDragLeaveEvent* /*event*/)
{
    setDropHighlight(false);
}

void DocumentTableWidget::dropEvent(QDropEvent* event)
{
    setDropHighlight(false);

    const QMimeData* mimeData = event->mimeData();
    if (!mimeData->hasUrls()) return;

    QList<QUrl> urls = mimeData->urls();
    int successCount = 0;
    int failCount = 0;

    for (const QUrl& url : urls) {
        if (!url.isLocalFile()) continue;
        QString filePath = url.toLocalFile();
        try {
            uploadFile(filePath);
            successCount++;
        } catch (...) {
            failCount++;
        }
    }

    if (successCount > 0) {
        QString msg = QString("Successfully uploaded %1 file(s)").arg(successCount);
        if (failCount > 0) msg += QString(", %1 failed").arg(failCount);
        QMessageBox::information(this, "Upload Complete", msg);
        onFilterChanged(filterCombo_->currentIndex());
    } else if (failCount > 0) {
        QMessageBox::warning(this, "Upload Failed", "Could not upload dropped files");
    }

    event->acceptProposedAction();
}

void DocumentTableWidget::setDropHighlight(bool active)
{
    if (active) {
        dropOverlay_->setGeometry(rect());
        dropOverlay_->show();
        dropOverlay_->raise();
    } else {
        dropOverlay_->hide();
    }
}

// --- Upload from file ---
void DocumentTableWidget::uploadFile(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    QString title = fileInfo.fileName();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error("Cannot open file: " + filePath.toStdString());
    }
    QByteArray data = file.readAll();
    file.close();

    std::string content(data.constData(), data.size());
    std::string userId = currentUserId_.toStdString();

    std::string docId = app_.docs().uploadPersonal(userId, title.toStdString(), content);
    // Update the search index so the new document is searchable immediately.
    app_.search().indexDocument(docId);
}

void DocumentTableWidget::onUploadFileClicked()
{
    QStringList files = QFileDialog::getOpenFileNames(this, "Select Files to Upload",
        QString(),
        "All Files (*);;Text Files (*.txt *.md *.csv *.json *.xml *.log)"
        ";;Documents (*.pdf *.doc *.docx *.odt *.rtf)"
        ";;Images (*.png *.jpg *.jpeg *.gif *.bmp *.svg)"
        ";;Spreadsheets (*.xls *.xlsx *.ods)");

    if (files.isEmpty()) return;

    bool ok = false;
    QStringList visOptions = {"Personal (Private)", "Unit Public"};
    QString visSel = QInputDialog::getItem(this, "Visibility",
        "Select visibility for uploaded files:", visOptions, 0, false, &ok);
    if (!ok) return;

    bool isPublic = (visSel == "Unit Public");
    int successCount = 0;
    QStringList errors;

    for (const QString& filePath : files) {
        QFileInfo fileInfo(filePath);
        QString title = fileInfo.fileName();

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            errors << "Cannot open: " + fileInfo.fileName();
            continue;
        }
        QByteArray data = file.readAll();
        file.close();

        std::string content(data.constData(), data.size());
        std::string userId = currentUserId_.toStdString();

        try {
            std::string docId;
            if (isPublic) {
                std::string unitId = currentUnitId_.isEmpty() ?
                    app_.userRepo().findById(userId)->unitId : currentUnitId_.toStdString();
                docId = app_.docs().uploadUnitPublic(userId, unitId, title.toStdString(), content);
            } else {
                docId = app_.docs().uploadPersonal(userId, title.toStdString(), content);
            }
            // Update the search index so the new document is searchable immediately.
            app_.search().indexDocument(docId);
            successCount++;
        } catch (const std::exception& e) {
            errors << fileInfo.fileName() + ": " + QString::fromStdString(e.what());
        }
    }

    if (successCount > 0) {
        QString msg = QString("Uploaded %1 of %2 files successfully.")
            .arg(successCount).arg(files.size());
        if (!errors.isEmpty()) {
            msg += "\n\nErrors:\n" + errors.join("\n");
        }
        QMessageBox::information(this, "Upload Complete", msg);
        onFilterChanged(filterCombo_->currentIndex());
    } else {
        QMessageBox::warning(this, "Upload Failed",
            "No files were uploaded.\n\n" + errors.join("\n"));
    }
}

// --- Data display ---
void DocumentTableWidget::refreshForUnit(const QString& unitId)
{
    currentUnitId_ = unitId;
    std::string uid = unitId.toStdString();
    std::string userId = currentUserId_.toStdString();

    // SYS_ADMIN sees all documents across the system, not just unit-scoped ones.
    bool isSysAdmin = app_.perms().isSysAdmin(userId);

    std::vector<Document> candidates;
    if (isSysAdmin) {
        candidates = app_.docRepo().findAll();
    } else {
        candidates = app_.docRepo().findByUnitScope(uid);
    }

    std::vector<Document> visible;
    for (const auto& doc : candidates) {
        if (doc.isDeleted) continue;
        auto reason = app_.perms().explainReadAccess(userId, doc.id);
        if (reason != "DENIED") {
            visible.push_back(doc);
        }
    }
    populateTable(visible);
}

void DocumentTableWidget::refreshMyDocs()
{
    auto docs = app_.docs().listMine(currentUserId_.toStdString());
    populateTable(docs);
}

void DocumentTableWidget::refreshSharedWithMe()
{
    auto shares = app_.shares().listSharedWithMe(currentUserId_.toStdString());
    std::vector<Document> docs;
    for (const auto& share : shares) {
        auto doc = app_.docRepo().findById(share.documentId);
        if (doc && !doc->isDeleted) {
            docs.push_back(*doc);
        }
    }
    populateTable(docs);
}

void DocumentTableWidget::refreshFavorites()
{
    auto docs = app_.docs().listFavorites(currentUserId_.toStdString());
    populateTable(docs);
}

QString DocumentTableWidget::formatSize(uint64_t bytes)
{
    if (bytes == 0) return "0 B";
    if (bytes < 1024) return QString::number(bytes) + " B";
    if (bytes < 1024 * 1024) return QString::number(bytes / 1024.0, 'f', 1) + " KB";
    if (bytes < 1024ULL * 1024 * 1024) return QString::number(bytes / (1024.0 * 1024.0), 'f', 1) + " MB";
    return QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
}

void DocumentTableWidget::populateTable(const std::vector<Document>& docs)
{
    table_->setRowCount(0);
    table_->setRowCount(static_cast<int>(docs.size()));

    for (int i = 0; i < static_cast<int>(docs.size()); ++i) {
        const auto& doc = docs[i];

        // Title
        QString title = QString::fromStdString(doc.title);
        auto* titleItem = new QTableWidgetItem(title);
        titleItem->setData(Qt::UserRole, QString::fromStdString(doc.id));
        titleItem->setToolTip(title);
        table_->setItem(i, 0, titleItem);

        // Owner name
        auto owner = app_.userRepo().findById(doc.ownerUserId);
        QString ownerName = owner ? QString::fromStdString(owner->username) : "?";
        table_->setItem(i, 1, new QTableWidgetItem(ownerName));

        // Visibility badge
        QString visStr = QString::fromStdString(visibilityToString(doc.visibility));
        auto* visItem = new QTableWidgetItem(visStr);
        if (doc.visibility == VisibilityType::UnitPublic) {
            visItem->setForeground(QColor("#28a745"));
        } else {
            visItem->setForeground(QColor("#6c757d"));
        }
        table_->setItem(i, 2, visItem);

        // Human-readable size
        table_->setItem(i, 3, new QTableWidgetItem(formatSize(doc.size)));

        // Date
        QString dateStr = QString::fromStdString(doc.createdAt);
        if (dateStr.length() > 10) dateStr = dateStr.left(10);
        table_->setItem(i, 4, new QTableWidgetItem(dateStr));

        // Permission badge with color
        QString badge = getPermissionBadge(doc.id);
        auto* permItem = new QTableWidgetItem(badge);
        if (badge == "OWNER") permItem->setForeground(QColor("#28a745"));
        else if (badge == "SHARED") permItem->setForeground(QColor("#007bff"));
        else if (badge == "UNIT_PUBLIC") permItem->setForeground(QColor("#17a2b8"));
        else if (badge == "SYS_ADMIN") permItem->setForeground(QColor("#6f42c1"));
        table_->setItem(i, 5, permItem);
    }

    statsLabel_->setText(QString("%1 document(s)").arg(docs.size()));

    // Clear preview when list changes
    previewTitle_->setText("Preview");
    previewMeta_->clear();
    previewContent_->clear();
}

QString DocumentTableWidget::getPermissionBadge(const std::string& docId)
{
    std::string reason = app_.perms().explainReadAccess(currentUserId_.toStdString(), docId);
    return QString::fromStdString(reason);
}

void DocumentTableWidget::onFilterChanged(int index)
{
    switch (index) {
        case 0:
            if (!currentUnitId_.isEmpty())
                refreshForUnit(currentUnitId_);
            break;
        case 1: refreshMyDocs(); break;
        case 2: refreshSharedWithMe(); break;
        case 3: refreshFavorites(); break;
    }
}

void DocumentTableWidget::onUploadClicked()
{
    bool ok = false;
    QString title = QInputDialog::getText(this, "Upload Text Document",
        "Document title:", QLineEdit::Normal, "", &ok);
    if (!ok || title.isEmpty()) return;

    QStringList visOptions = {"Personal (Private)", "Unit Public"};
    QString visSel = QInputDialog::getItem(this, "Visibility",
        "Select visibility:", visOptions, 0, false, &ok);
    if (!ok) return;

    QString content = QInputDialog::getMultiLineText(this, "Content",
        "Enter document content:", "", &ok);
    if (!ok) return;

    std::string userId = currentUserId_.toStdString();

    try {
        std::string docId;
        if (visSel == "Unit Public") {
            std::string unitId = currentUnitId_.isEmpty() ?
                app_.userRepo().findById(userId)->unitId : currentUnitId_.toStdString();
            docId = app_.docs().uploadUnitPublic(userId, unitId, title.toStdString(), content.toStdString());
        } else {
            docId = app_.docs().uploadPersonal(userId, title.toStdString(), content.toStdString());
        }
        // Update the search index so the new document is searchable immediately.
        app_.search().indexDocument(docId);
        QMessageBox::information(this, "Success", "Document uploaded successfully!");
        onFilterChanged(filterCombo_->currentIndex());
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", QString::fromStdString(e.what()));
    }
}

void DocumentTableWidget::onTableClicked(int row, int /*column*/)
{
    auto* item = table_->item(row, 0);
    if (!item) return;
    QString docId = item->data(Qt::UserRole).toString();
    showPreview(docId);
}

void DocumentTableWidget::onTableDoubleClicked(int row, int /*column*/)
{
    auto* item = table_->item(row, 0);
    if (!item) return;
    QString docId = item->data(Qt::UserRole).toString();

    DocumentDetailDialog dlg(app_, currentUserId_, docId, this);
    dlg.exec();
    onFilterChanged(filterCombo_->currentIndex());
}

void DocumentTableWidget::onContextMenu(const QPoint& pos)
{
    auto* item = table_->itemAt(pos);
    if (!item) return;
    int row = item->row();
    QString docId = table_->item(row, 0)->data(Qt::UserRole).toString();

    QMenu menu(this);
    menu.setStyleSheet(
        "QMenu { background: white; border: 1px solid #dee2e6; border-radius: 4px; padding: 4px; }"
        "QMenu::item { padding: 6px 16px; }"
        "QMenu::item:selected { background: #e9ecef; border-radius: 2px; }");

    menu.addAction("View / Edit", [this, docId]() {
        DocumentDetailDialog dlg(app_, currentUserId_, docId, this);
        dlg.exec();
        onFilterChanged(filterCombo_->currentIndex());
    });

    menu.addAction("Share...", [this, docId]() {
        ShareDialog dlg(app_, currentUserId_, docId, this);
        dlg.exec();
    });

    menu.addAction("Add to Favorites", [this, docId]() {
        try {
            app_.docs().addFavorite(currentUserId_.toStdString(), docId.toStdString());
            QMessageBox::information(this, "Favorites", "Added to favorites!");
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Error", QString::fromStdString(e.what()));
        }
    });

    menu.addSeparator();

    menu.addAction("Delete", [this, docId]() {
        auto reply = QMessageBox::question(this, "Delete Document",
            "Are you sure you want to delete this document?\nThis action cannot be undone.",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            try {
                app_.docs().remove(currentUserId_.toStdString(), docId.toStdString());
                onFilterChanged(filterCombo_->currentIndex());
            } catch (const std::exception& e) {
                QMessageBox::warning(this, "Error", QString::fromStdString(e.what()));
            }
        }
    });

    menu.exec(table_->viewport()->mapToGlobal(pos));
}

} // namespace dms
