#pragma once

// Renders PDF files to images/text using poppler-utils (pdftoppm, pdftotext,
// pdfinfo). Falls back gracefully when the tools are unavailable.

#include <QPixmap>
#include <QString>
#include <string>
#include <vector>

namespace dms {

struct PdfRenderResult {
    std::vector<QPixmap> pageImages;   // rendered pages (first N)
    std::string text;                  // extracted text (may be empty)
    int pageCount = 0;
    bool ok = false;
    QString error;                     // empty on success
};

// Render up to `maxPages` pages of a PDF (given as raw bytes) to QPixmap images
// at `dpi` resolution, and extract text via pdftotext. Returns pageImages,
// text, pageCount. If maxPages <= 0, renders ALL pages. If poppler-utils is
// missing, ok=false and error is set.
PdfRenderResult renderPdf(const std::string& pdfBytes,
                          int maxPages = 3, int dpi = 110);

// Extract text from a PDF using pdftotext (poppler-utils). Lightweight: no
// image rendering, no QPixmap. Returns empty string if pdftotext is unavailable
// or fails. Used by the search index to index real PDF text content.
std::string extractPdfText(const std::string& pdfBytes);

} // namespace dms
