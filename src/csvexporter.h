#pragma once

#include "database.h"
#include <QWidget>

class CsvExporter
{
public:
    // Export filtered work entries to CSV (Excel-compatible)
    static bool exportWorkLog(const QList<WorkEntry>& entries,
                               int year, int month,
                               QWidget* parent = nullptr);

    // Export clients list
    static bool exportClients(const QList<Client>& clients,
                               QWidget* parent = nullptr);
};
