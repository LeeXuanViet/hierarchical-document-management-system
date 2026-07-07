#include "pdf_renderer.hpp"
#include "pdf_text_extractor.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTextStream>

namespace dms {

namespace {

// Check whether a given executable exists on PATH.
bool toolAvailable(const QString& name)
{
    QProcess p;
    p.start("which", QStringList{name});
    p.waitForFinished(3000);
    return p.exitCode() == 0 && p.readAllStandardOutput().trimmed().size() > 0;
}

} // namespace

PdfRenderResult renderPdf(const std::string& pdfBytes, int maxPages, int dpi)
{
    PdfRenderResult result;

    if (!toolAvailable("pdftoppm") && !toolAvailable("pdftotext")) {
        result.error = "poppler-utils not installed";
        return result;
    }

    // Write the PDF bytes to a temp file.
    QTemporaryFile pdfFile(QDir::tempPath() + "/dms_preview_XXXXXX.pdf");
    pdfFile.setAutoRemove(true);
    if (!pdfFile.open()) {
        result.error = "cannot create temp pdf file";
        return result;
    }
    pdfFile.write(pdfBytes.data(), static_cast<qint64>(pdfBytes.size()));
    pdfFile.flush();
    QString pdfPath = pdfFile.fileName();

    // Use a temp dir for rendered page images.
    QTemporaryDir tmpDir(QDir::tempPath() + "/dms_pdfpages_XXXXXX");
    tmpDir.setAutoRemove(true);
    QString outPrefix = tmpDir.filePath("page");

    // Render pages to PNG with pdftoppm.
    if (toolAvailable("pdftoppm")) {
        QProcess pp;
        QStringList args;
        args << "-png"
             << "-r" << QString::number(dpi)
             << "-f" << "1";
        // maxPages <= 0 means render ALL pages (omit -l).
        if (maxPages > 0) {
            args << "-l" << QString::number(maxPages);
        }
        args << pdfPath
             << outPrefix;
        pp.start("pdftoppm", args);
        // Allow more time for large multi-page PDFs.
        pp.waitForFinished(60000);
        if (pp.exitCode() == 0) {
            QDir dir(tmpDir.path());
            QStringList filters;
            filters << "page-*.png";
            QStringList pages = dir.entryList(filters, QDir::Files, QDir::Name);
            for (const QString& pageName : pages) {
                QPixmap pix(dir.filePath(pageName));
                if (!pix.isNull()) {
                    result.pageImages.push_back(pix);
                }
            }
        }
    }

    // Extract text with pdftotext (handles compressed streams).
    if (toolAvailable("pdftotext")) {
        QTemporaryFile txtFile(QDir::tempPath() + "/dms_preview_XXXXXX.txt");
        txtFile.setAutoRemove(true);
        if (txtFile.open()) {
            QString txtPath = txtFile.fileName();
            txtFile.close();
            QProcess pt;
            pt.start("pdftotext", QStringList{pdfPath, txtPath});
            pt.waitForFinished(15000);
            if (pt.exitCode() == 0) {
                QFile f(txtPath);
                if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream in(&f);
                    in.setCodec("UTF-8");
                    result.text = in.readAll().toStdString();
                    f.close();
                }
            }
        }
    }

    // Page count via pdfinfo if available, else from rendered images.
    if (toolAvailable("pdfinfo")) {
        QProcess pi;
        pi.start("pdfinfo", QStringList{pdfPath});
        pi.waitForFinished(10000);
        if (pi.exitCode() == 0) {
            QString out = QString::fromUtf8(pi.readAllStandardOutput());
            QRegularExpression re("Pages:\\s+(\\d+)");
            auto m = re.match(out);
            if (m.hasMatch()) {
                result.pageCount = m.captured(1).toInt();
            }
        }
    }
    if (result.pageCount == 0) {
        result.pageCount = static_cast<int>(result.pageImages.size());
    }

    result.ok = !result.pageImages.empty() || !result.text.empty();
    if (!result.ok && result.error.isEmpty()) {
        result.error = "pdftoppm/pdftotext produced no output";
    }
    return result;
}

std::string extractPdfText(const std::string& pdfBytes)
{
    if (!toolAvailable("pdftotext")) {
        // Fallback to the pure-C++ extractor (uncompressed text only).
        PdfInfo info = extractPdfInfo(pdfBytes, 65536);
        return info.text;
    }

    // Write the PDF bytes to a temp file.
    QTemporaryFile pdfFile(QDir::tempPath() + "/dms_text_XXXXXX.pdf");
    pdfFile.setAutoRemove(true);
    if (!pdfFile.open()) return {};
    pdfFile.write(pdfBytes.data(), static_cast<qint64>(pdfBytes.size()));
    pdfFile.flush();
    QString pdfPath = pdfFile.fileName();

    // Extract text with pdftotext (handles compressed streams).
    QTemporaryFile txtFile(QDir::tempPath() + "/dms_text_XXXXXX.txt");
    txtFile.setAutoRemove(true);
    if (!txtFile.open()) return {};
    QString txtPath = txtFile.fileName();
    txtFile.close();

    QProcess pt;
    pt.start("pdftotext", QStringList{pdfPath, txtPath});
    pt.waitForFinished(30000);
    if (pt.exitCode() != 0) return {};

    QFile f(txtPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return {};
    QTextStream in(&f);
    in.setCodec("UTF-8");
    std::string text = in.readAll().toStdString();
    f.close();
    return text;
}

} // namespace dms
