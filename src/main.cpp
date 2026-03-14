#include "mainwindow.h"
#include "loginscreen.h"
#include "database.h"
#include "session.h"
#include "style.h"

#include <QApplication>
#include <QMessageBox>
#include <QFont>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("ITTracker");
    app.setOrganizationName("ITTracker");
    app.setApplicationVersion("1.0.0");
    app.setStyleSheet(Style::appStyleSheet());

    QFont font("Segoe UI", 10);
    app.setFont(font);

    // Init DB first
    if (!Database::instance().initialize()) {
        QMessageBox::critical(nullptr, "Greška",
            "Nije moguće otvoriti bazu podataka!\nProgram će biti zatvoren.");
        return 1;
    }

    // Show login screen
    LoginScreen login;
    if (login.exec() != QDialog::Accepted)
        return 0;   // user closed login — exit

    // Launch main window (session is now set)
    MainWindow win;
    win.show();

    return app.exec();
}
