#pragma once

#include <QWidget>
#include <QString>

class BackupManager
{
public:
    // Save a copy of the database to user-chosen location
    static bool backup(QWidget* parent = nullptr);

    // Restore database from a backup file (restarts DB connection)
    static bool restore(QWidget* parent = nullptr);

private:
    static QString dbPath();
};
