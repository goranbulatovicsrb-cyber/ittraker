#include "backupmanager.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QCoreApplication>
#include <QDate>
#include <QSqlDatabase>
#include <QStandardPaths>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>

QString BackupManager::dbPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
           + "/ittracker.db";
}

bool BackupManager::backup(QWidget* parent)
{
    QString src = dbPath();
    if (!QFile::exists(src)) {
        QMessageBox::warning(parent, "Greška", "Baza podataka nije pronađena!");
        return false;
    }

    QString defaultDest = QString("%1/ITTracker_Backup_%2.db")
                              .arg(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation))
                              .arg(QDate::currentDate().toString("yyyy-MM-dd"));

    QString dest = QFileDialog::getSaveFileName(parent,
        "Sačuvaj backup baze", defaultDest,
        "Database fajlovi (*.db);;Svi fajlovi (*.*)");
    if (dest.isEmpty()) return false;

    if (QFile::exists(dest)) QFile::remove(dest);

    if (!QFile::copy(src, dest)) {
        QMessageBox::critical(parent, "Greška", "Backup nije uspio!\nProvjerite dozvole fajla.");
        return false;
    }

    auto ans = QMessageBox::information(parent, "✅ Backup sačuvan",
        QString("Backup baze je sačuvan na:\n%1\n\nOtvoriti folder?").arg(dest),
        QMessageBox::Yes | QMessageBox::No);

    if (ans == QMessageBox::Yes)
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(dest).absolutePath()));

    return true;
}

bool BackupManager::restore(QWidget* parent)
{
    auto confirm = QMessageBox::warning(parent,
        "⚠️ Obnova backup-a",
        "Ovo će ZAMIJENITI sve trenutne podatke sa podacima iz backup fajla!\n\n"
        "Preporučuje se da prvo napravite backup trenutnih podataka.\n\n"
        "Da li ste sigurni da želite nastaviti?",
        QMessageBox::Yes | QMessageBox::Cancel);

    if (confirm != QMessageBox::Yes) return false;

    QString src = QFileDialog::getOpenFileName(parent,
        "Odaberi backup fajl", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
        "Database fajlovi (*.db);;Svi fajlovi (*.*)");
    if (src.isEmpty()) return false;

    // Verify it looks like a valid SQLite file
    QFile check(src);
    if (!check.open(QIODevice::ReadOnly) || check.size() < 16) {
        QMessageBox::critical(parent, "Greška", "Odabrani fajl nije validan backup!");
        return false;
    }
    QByteArray magic = check.read(16);
    check.close();
    if (!magic.startsWith("SQLite format 3")) {
        QMessageBox::critical(parent, "Greška",
            "Odabrani fajl nije SQLite baza podataka!\nMolimo odaberite validan .db backup fajl.");
        return false;
    }

    QString dest = dbPath();

    // Close Qt's DB connection before replacing the file
    {
        QSqlDatabase db = QSqlDatabase::database();
        if (db.isOpen()) db.close();
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    }

    if (QFile::exists(dest)) QFile::remove(dest);

    if (!QFile::copy(src, dest)) {
        QMessageBox::critical(parent, "Greška", "Obnova nije uspjela!\nProvjerite dozvole fajla.");
        return false;
    }

    QMessageBox::information(parent, "✅ Obnova uspješna",
        "Backup je uspješno obnovljen!\n\nProgram će se ponovo pokrenuti da učita podatke.");

    // Restart app
    qApp->quit();
    QProcess::startDetached(qApp->applicationFilePath(), qApp->arguments());

    return true;
}
