#pragma once

// Self-contained, dependency-free PDF text/metadata extractor.
// Works on uncompressed, unencrypted PDFs by parsing content streams for
// text-showing operators (Tj, TJ, ', "). For compressed/encrypted PDFs it
// still extracts document metadata and page count when available.

#include <string>
#include <vector>

namespace dms {

struct PdfInfo {
    std::string title;
    std::string author;
    std::string subject;
    std::string producer;
    std::string creator;
    std::string creationDate;   // raw PDF date string, e.g. D:20240101120000
    int pageCount = 0;
    bool encrypted = false;
    std::string text;           // extracted plain text (may be empty if compressed)
};

// Parse a PDF stored in `data` and return metadata + extracted text.
// `maxTextChars` caps the amount of extracted text to keep previews light.
PdfInfo extractPdfInfo(const std::string& data, size_t maxTextChars = 8192);

} // namespace dms
