#include "csvexporter.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>

static const QStringList MONTH_SHORT = {
    "","Jan","Feb","Mar","Apr","Maj","Jun",
    "Jul","Avg","Sep","Okt","Nov","Dec"
};

// Escape a CSV field (wrap in quotes if contains comma/newline/quote)
static QString csvField(const QString& s)
{
    QString escaped = s;
    escaped.replace("\"", "\"\"");
    if (escaped.contains(',') || escaped.contains('\n') || escaped.contains('"'))
        return "\"" + escaped + "\"";
    return escaped;
}

bool CsvExporter::exportWorkLog(const QList<WorkEntry>& entries,
                                  int year, int month, QWidget* parent)
{
    QString suffix;
    if (year > 0 && month > 0)
        suffix = QString("_%1_%2").arg(MONTH_SHORT.value(month)).arg(year);
    else if (year > 0)
        suffix = QString("_%1").arg(year);

    QString defaultName = QString("%1/ITTracker_Poslovi%2.csv")
                              .arg(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation))
                              .arg(suffix);

    QString path = QFileDialog::getSaveFileName(parent,
        "Izvezi poslove u CSV/Excel", defaultName,
        "CSV fajlovi (*.csv);;Svi fajlovi (*.*)");
    if (path.isEmpty()) return false;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(parent, "Greška", "Nije moguće sačuvati fajl:\n" + path);
        return false;
    }

    QTextStream out(&file);
    // UTF-8 BOM so Excel opens it correctly with ć,š,ž etc.
    out.setEncoding(QStringConverter::Utf8);
    out << "\xEF\xBB\xBF";

    // Header row
    out << "ID,Datum,Musterija,Vrsta posla,Opis,Sati,Cijena/h (EUR),"
           "Ukupno (EUR),Status\n";

    double total = 0.0;
    for (auto& e : entries) {
        out << e.id << ","
            << csvField(e.date.toString("dd.MM.yyyy")) << ","
            << csvField(e.clientName) << ","
            << csvField(e.workType) << ","
            << csvField(e.description) << ","
            << QString::number(e.hours, 'f', 2) << ","
            << QString::number(e.pricePerHour, 'f', 2) << ","
            << QString::number(e.totalPrice, 'f', 2) << ","
            << (e.isPaid ? "Placeno" : "Ceka uplatu")
            << "\n";
        total += e.totalPrice;
    }

    // Summary rows
    out << "\n";
    out << ",,,,,,,Ukupno:," << QString::number(total, 'f', 2) << "\n";
    out << ",,,,,,,Broj poslova:," << entries.size() << "\n";
    out << ",,,,,,Exportovano:," << QDate::currentDate().toString("dd.MM.yyyy") << "\n";

    file.close();

    auto ans = QMessageBox::information(parent, "✅ CSV Sačuvan",
        QString("Izvezeno %1 poslova u:\n%2\n\nOtvoriti u Excelu?")
            .arg(entries.size()).arg(path),
        QMessageBox::Yes | QMessageBox::No);

    if (ans == QMessageBox::Yes)
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));

    return true;
}

bool CsvExporter::exportClients(const QList<Client>& clients, QWidget* parent)
{
    QString defaultName = QString("%1/ITTracker_Musterije_%2.csv")
                              .arg(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation))
                              .arg(QDate::currentDate().toString("yyyy"));

    QString path = QFileDialog::getSaveFileName(parent,
        "Izvezi mušterije u CSV/Excel", defaultName,
        "CSV fajlovi (*.csv);;Svi fajlovi (*.*)");
    if (path.isEmpty()) return false;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(parent, "Greška", "Nije moguće sačuvati fajl:\n" + path);
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << "\xEF\xBB\xBF";

    out << "ID,Ime,Email,Telefon,Adresa,Napomene\n";
    for (auto& c : clients) {
        out << c.id << ","
            << csvField(c.name) << ","
            << csvField(c.email) << ","
            << csvField(c.phone) << ","
            << csvField(c.address) << ","
            << csvField(c.notes) << "\n";
    }

    file.close();

    auto ans = QMessageBox::information(parent, "✅ CSV Sačuvan",
        QString("Izvezeno %1 mušterija u:\n%2\n\nOtvoriti u Excelu?")
            .arg(clients.size()).arg(path),
        QMessageBox::Yes | QMessageBox::No);

    if (ans == QMessageBox::Yes)
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));

    return true;
}
