#include "mainwindow.h"
#include "database.h"
#include "style.h"

#include <QApplication>
#include <QMessageBox>
#include <QSplashScreen>
#include <QTimer>
#include <QFont>
#include <QPixmap>
#include <QPainter>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // App metadata
    app.setApplicationName("ITTracker");
    app.setOrganizationName("ITTracker");
    app.setApplicationVersion("1.0.0");

    // Apply global stylesheet
    app.setStyleSheet(Style::appStyleSheet());

    // Default font
    QFont font("Segoe UI", 10);
    app.setFont(font);

    // Init database
    if (!Database::instance().initialize()) {
        QMessageBox::critical(nullptr, "Greška",
            "Nije moguće otvoriti bazu podataka!\nProgram će biti zatvoren.");
        return 1;
    }

    // Main window
    MainWindow win;
    win.show();

    return app.exec();
}
