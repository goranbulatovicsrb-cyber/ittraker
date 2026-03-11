#include "pdfexporter.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTextDocument>
#include <QPrinter>
#include <QDate>
#include <QStandardPaths>

static const QStringList MONTH_NAMES_FULL = {
    "", "Januar","Februar","Mart","April","Maj","Juni",
    "Juli","Avgust","Septembar","Oktobar","Novembar","Decembar"
};

// ─── Public API ───────────────────────────────────────────────────────────────

bool PdfExporter::exportInvoice(const WorkEntry& entry, QWidget* parent)
{
    QString invoiceNum = QString("IT-%1-%2")
                             .arg(QDate::currentDate().year())
                             .arg(entry.id, 4, 10, QChar('0'));

    QString defaultName = QString("%1/Faktura_%2_%3.pdf")
                              .arg(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation))
                              .arg(invoiceNum)
                              .arg(entry.clientName.simplified().replace(' ', '_'));

    QString path = QFileDialog::getSaveFileName(parent,
        "Sačuvaj fakturu kao PDF", defaultName, "PDF fajlovi (*.pdf)");
    if (path.isEmpty()) return false;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(path);
    printer.setPageSize(QPageSize::A4);
    printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

    QTextDocument doc;
    doc.setDefaultStyleSheet(cssBase());
    doc.setHtml(invoiceHtml(entry, invoiceNum));
    doc.print(&printer);

    QMessageBox::information(parent, "✅ Faktura sačuvana",
        QString("Faktura je sačuvana na:\n%1").arg(path));
    return true;
}

bool PdfExporter::exportAnnualReport(int year, QWidget* parent)
{
    QString defaultName = QString("%1/ITTracker_Godisnji_Izvjestaj_%2.pdf")
                              .arg(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation))
                              .arg(year);

    QString path = QFileDialog::getSaveFileName(parent,
        "Sačuvaj godišnji izvještaj kao PDF", defaultName, "PDF fajlovi (*.pdf)");
    if (path.isEmpty()) return false;

    auto& db     = Database::instance();
    auto monthly = db.earningsPerMonth(year);
    auto byType  = db.earningsByType(year);
    auto entries = db.getWorkEntries(year);

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(path);
    printer.setPageSize(QPageSize::A4);
    printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

    QTextDocument doc;
    doc.setDefaultStyleSheet(cssBase());
    doc.setHtml(annualReportHtml(year, monthly, byType, entries));
    doc.print(&printer);

    QMessageBox::information(parent, "✅ Izvještaj sačuvan",
        QString("Godišnji izvještaj %1 sačuvan na:\n%2").arg(year).arg(path));
    return true;
}

// ─── CSS ─────────────────────────────────────────────────────────────────────

QString PdfExporter::cssBase()
{
    return R"(
        body  { font-family: Arial, sans-serif; font-size: 11pt; color: #1a1a2e; margin: 0; }
        h1    { font-size: 22pt; color: #1f6feb; margin-bottom: 4px; }
        h2    { font-size: 14pt; color: #1f4080; border-bottom: 2px solid #1f6feb;
                padding-bottom: 4px; margin-top: 24px; }
        h3    { font-size: 11pt; color: #1f4080; margin-bottom: 6px; }
        table { width: 100%; border-collapse: collapse; margin-top: 10px; }
        th    { background: #1f6feb; color: #ffffff; padding: 8px 10px;
                text-align: left; font-size: 10pt; }
        td    { padding: 7px 10px; border-bottom: 1px solid #e0e0e0; font-size: 10pt; }
        tr:nth-child(even) td { background: #f5f8ff; }
        .right  { text-align: right; }
        .center { text-align: center; }
        .green  { color: #1a7a3a; font-weight: bold; }
        .red    { color: #c0392b; font-weight: bold; }
        .badge-paid   { background:#d4edda; color:#155724; padding:2px 8px;
                        border-radius:4px; font-size:9pt; }
        .badge-unpaid { background:#fff3cd; color:#856404; padding:2px 8px;
                        border-radius:4px; font-size:9pt; }
        .total-box { background:#1f6feb; color:#fff; padding:12px 18px;
                     border-radius:6px; font-size:14pt; font-weight:bold;
                     text-align:right; margin-top:16px; }
        .info-grid { width:100%; margin-bottom: 20px; }
        .info-grid td { border: none; padding: 3px 0; }
        .label { color: #666; font-size:10pt; }
        .footer { color:#888; font-size:9pt; text-align:center;
                  border-top:1px solid #ddd; margin-top:30px; padding-top:10px; }
        .stat-row td { border:none; padding: 4px 8px; }
        .stat-val { font-size:16pt; font-weight:bold; color:#1f6feb; }
    )";
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

QString PdfExporter::formatCurrency(double val)
{
    return QString("%1 €").arg(val, 0, 'f', 2);
}

// ─── Invoice HTML ─────────────────────────────────────────────────────────────

QString PdfExporter::invoiceHtml(const WorkEntry& entry, const QString& invoiceNumber)
{
    QString html;
    html += "<html><body>";

    // Header
    html += R"(
        <table style="width:100%; border:none; margin-bottom:20px;">
          <tr>
            <td style="border:none; vertical-align:top;">
              <h1>💻 IT Tracker</h1>
              <span class="label">Vaš IT servis</span>
            </td>
            <td style="border:none; text-align:right; vertical-align:top;">
              <h2 style="border:none; font-size:18pt; color:#1f6feb;">FAKTURA</h2>
              <b>Broj:</b> )";
    html += invoiceNumber;
    html += R"(<br><b>Datum:</b> )";
    html += QDate::currentDate().toString("dd.MM.yyyy");
    html += R"(
            </td>
          </tr>
        </table>
        <hr style="border:2px solid #1f6feb; margin-bottom:20px;">
    )";

    // Client info + job info
    html += "<table class='info-grid'><tr>";
    html += "<td style='width:50%; vertical-align:top; border:none;'>";
    html += "<h3>📋 Podaci o mušteriji</h3>";
    html += "<table style='border:none;'>";
    html += QString("<tr><td class='label' style='border:none;'>Ime:</td><td style='border:none;font-weight:bold;'>%1</td></tr>").arg(entry.clientName);
    html += "</table></td>";

    html += "<td style='width:50%; vertical-align:top; border:none;'>";
    html += "<h3>🔧 Podaci o poslu</h3>";
    html += "<table style='border:none;'>";
    html += QString("<tr><td class='label' style='border:none;'>Vrsta:</td><td style='border:none;'>%1</td></tr>").arg(entry.workType);
    html += QString("<tr><td class='label' style='border:none;'>Datum:</td><td style='border:none;'>%1</td></tr>").arg(entry.date.toString("dd.MM.yyyy"));
    if (!entry.description.isEmpty())
        html += QString("<tr><td class='label' style='border:none;'>Opis:</td><td style='border:none;'>%1</td></tr>").arg(entry.description);
    html += "</table></td></tr></table>";

    // Items table
    html += "<h3>📦 Stavke</h3>";
    html += "<table><tr><th>Opis usluge</th><th class='center'>Sati</th>"
            "<th class='right'>Cijena/h</th><th class='right'>Ukupno</th></tr>";

    QString desc = entry.description.isEmpty() ? entry.workType : entry.description;
    html += QString("<tr><td>%1</td><td class='center'>%2</td><td class='right'>%3</td>"
                    "<td class='right green'>%4</td></tr>")
                .arg(desc)
                .arg(entry.hours > 0 ? QString::number(entry.hours, 'f', 1) : "—")
                .arg(entry.pricePerHour > 0 ? formatCurrency(entry.pricePerHour) : "—")
                .arg(formatCurrency(entry.totalPrice));
    html += "</table>";

    // Total box
    html += QString("<div class='total-box'>UKUPNO ZA UPLATU: %1</div>")
                .arg(formatCurrency(entry.totalPrice));

    // Payment status
    html += "<br><p><b>Status plaćanja:</b> ";
    html += entry.isPaid
        ? "<span class='badge-paid'>✅ PLAĆENO</span>"
        : "<span class='badge-unpaid'>⏳ ČEKA UPLATU</span>";
    html += "</p>";

    // Footer
    html += QString("<div class='footer'>Faktura generisana: %1 | IT Tracker v1.0</div>")
                .arg(QDate::currentDate().toString("dd.MM.yyyy"));

    html += "</body></html>";
    return html;
}

// ─── Annual Report HTML ───────────────────────────────────────────────────────

QString PdfExporter::annualReportHtml(int year,
                                       const QMap<int,double>& monthly,
                                       const QMap<QString,double>& byType,
                                       const QList<WorkEntry>& entries)
{
    // Compute totals
    double totalYear  = 0.0;
    double totalPaid  = 0.0;
    double totalUnpaid= 0.0;
    int    totalJobs  = entries.size();

    for (auto& e : entries) {
        totalYear += e.totalPrice;
        if (e.isPaid) totalPaid   += e.totalPrice;
        else          totalUnpaid += e.totalPrice;
    }

    QString html;
    html += "<html><body>";

    // Title
    html += QString("<h1>💻 IT Tracker — Godišnji Izvještaj %1</h1>").arg(year);
    html += QString("<p class='label'>Generisano: %1</p>")
                .arg(QDate::currentDate().toString("dd.MM.yyyy"));
    html += "<hr style='border:2px solid #1f6feb;'>";

    // Summary stats
    html += "<h2>📊 Sažetak godine</h2>";
    html += "<table><tr>"
            "<th>Ukupna zarada</th><th>Plaćeno</th><th>Neplaćeno</th><th>Broj poslova</th>"
            "</tr><tr>"
            + QString("<td class='green stat-val'>%1</td>").arg(formatCurrency(totalYear))
            + QString("<td class='green'>%1</td>").arg(formatCurrency(totalPaid))
            + QString("<td class='red'>%1</td>").arg(formatCurrency(totalUnpaid))
            + QString("<td class='center'>%1</td>").arg(totalJobs)
            + "</tr></table>";

    // Monthly breakdown
    html += "<h2>📅 Zarada po mesecima</h2>";
    html += "<table><tr><th>Mesec</th><th class='right'>Zarada</th>"
            "<th class='right'>Udio (%)</th></tr>";
    for (int m = 1; m <= 12; m++) {
        double val  = monthly.value(m, 0.0);
        double pct  = totalYear > 0 ? (val / totalYear * 100.0) : 0.0;
        QString rowStyle = val > 0 ? "" : " style='color:#bbb;'";
        html += QString("<tr%5><td>%1</td><td class='right'>%2</td>"
                        "<td class='right'>%3%</td></tr>")
                    .arg(MONTH_NAMES_FULL[m])
                    .arg(formatCurrency(val))
                    .arg(pct, 0, 'f', 1)
                    .arg(rowStyle);
    }
    html += QString("<tr style='background:#1f6feb;color:#fff;font-weight:bold;'>"
                    "<td>UKUPNO</td><td class='right'>%1</td><td class='right'>100%</td></tr>")
                .arg(formatCurrency(totalYear));
    html += "</table>";

    // By type
    if (!byType.isEmpty()) {
        html += "<h2>🔧 Zarada po vrsti posla</h2>";
        html += "<table><tr><th>Vrsta posla</th><th class='right'>Zarada</th>"
                "<th class='right'>Udio (%)</th></tr>";
        for (auto it = byType.cbegin(); it != byType.cend(); ++it) {
            double pct = totalYear > 0 ? (it.value() / totalYear * 100.0) : 0.0;
            html += QString("<tr><td>%1</td><td class='right green'>%2</td>"
                            "<td class='right'>%3%</td></tr>")
                        .arg(it.key())
                        .arg(formatCurrency(it.value()))
                        .arg(pct, 0, 'f', 1);
        }
        html += "</table>";
    }

    // All jobs table
    html += "<h2>📋 Svi poslovi</h2>";
    html += "<table><tr><th>Datum</th><th>Mušterija</th><th>Vrsta</th>"
            "<th class='right'>Ukupno</th><th class='center'>Status</th></tr>";
    for (auto& e : entries) {
        html += QString("<tr><td>%1</td><td>%2</td><td>%3</td>"
                        "<td class='right green'>%4</td>"
                        "<td class='center'>%5</td></tr>")
                    .arg(e.date.toString("dd.MM.yyyy"))
                    .arg(e.clientName)
                    .arg(e.workType)
                    .arg(formatCurrency(e.totalPrice))
                    .arg(e.isPaid
                         ? "<span class='badge-paid'>Plaćeno</span>"
                         : "<span class='badge-unpaid'>Čeka</span>");
    }
    html += "</table>";

    html += QString("<div class='total-box'>UKUPNA ZARADA %1: %2</div>")
                .arg(year).arg(formatCurrency(totalYear));

    html += QString("<div class='footer'>IT Tracker v1.0 | Izvještaj za %1 | "
                    "Generisano %2</div>").arg(year)
                .arg(QDate::currentDate().toString("dd.MM.yyyy"));

    html += "</body></html>";
    return html;
}
