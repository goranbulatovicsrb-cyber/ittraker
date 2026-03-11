#pragma once

#include "database.h"
#include <QString>
#include <QWidget>

class PdfExporter
{
public:
    // Single invoice for one WorkEntry
    static bool exportInvoice(const WorkEntry& entry, QWidget* parent = nullptr);

    // Annual report with charts data
    static bool exportAnnualReport(int year, QWidget* parent = nullptr);

private:
    static QString invoiceHtml(const WorkEntry& entry, const QString& invoiceNumber);
    static QString annualReportHtml(int year,
                                    const QMap<int,double>& monthly,
                                    const QMap<QString,double>& byType,
                                    const QList<WorkEntry>& entries);

    static QString formatCurrency(double val);
    static QString cssBase();
};
