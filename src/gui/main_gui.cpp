#include <QApplication>
#include <QStyleFactory>

#include "api/app_context.hpp"
#include "api/seed_data.hpp"
#include "gui/login_dialog.hpp"
#include "gui/main_window.hpp"
#include "gui/pdf_renderer.hpp"

#include <cctype>
#include <string>

int main(int argc, char* argv[])
{
    QApplication qtApp(argc, argv);
    qtApp.setApplicationName("Hierarchical Document Manager");
    qtApp.setStyle(QStyleFactory::create("Fusion"));

    // Initialize backend with SQLite persistence
    dms::AppContext app("dms.db", "data/files");

    // Register a PDF text extractor so the search index can index real text
    // content from PDF files (via poppler-utils pdftotext). This must be set
    // BEFORE seeding so that rebuildIndex() uses it for any PDF documents.
    app.search().setTextExtractor(
        [](const std::string& title, const std::string& rawContent) -> std::string {
            // Case-insensitive check for .pdf extension.
            std::string lower;
            lower.reserve(title.size());
            for (char c : title) lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
            if (lower.size() >= 4 &&
                lower.compare(lower.size() - 4, 4, ".pdf") == 0) {
                return dms::extractPdfText(rawContent);
            }
            // Non-PDF binary: no extractable text.
            return {};
        });

    // Only seed demo data if database is freshly created (no existing data).
    if (app.isNewDatabase()) {
        dms::seedDemo(app);
    } else {
        // Rebuild in-memory search index from existing persisted data.
        app.search().rebuildIndex();
    }

    // Show login dialog
    dms::LoginDialog loginDlg(app);
    if (loginDlg.exec() != QDialog::Accepted) {
        return 0;
    }

    // Show main window
    dms::MainWindow mainWindow(app, loginDlg.loggedInUserId());
    mainWindow.show();

    return qtApp.exec();
}
