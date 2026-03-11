#pragma once

#include <QObject>
#include <QtSql>
#include <QDate>
#include <QList>
#include <QMap>
#include <QString>

// ─── Data structures ──────────────────────────────────────────────────────────

struct Client {
    int     id      = -1;
    QString name;
    QString email;
    QString phone;
    QString address;
    QString notes;
};

struct WorkEntry {
    int     id           = -1;
    int     clientId     = -1;
    QString clientName;           // cached for display
    QString workType;             // Popravka, Sajt, Snimanje, Mreža, Instalacija, Ostalo
    QString description;
    QDate   date;
    double  hours        = 1.0;
    double  pricePerHour = 0.0;
    double  totalPrice   = 0.0;
    bool    isPaid       = false;
};

// ─── Database singleton ───────────────────────────────────────────────────────

class Database : public QObject
{
    Q_OBJECT
public:
    static Database& instance();

    bool initialize(const QString& path = "");

    // Clients
    int          addClient(Client& c);
    bool         updateClient(const Client& c);
    bool         deleteClient(int id);
    QList<Client> getAllClients();
    Client       getClientById(int id);

    // Work entries
    int              addWork(WorkEntry& w);
    bool             updateWork(const WorkEntry& w);
    bool             deleteWork(int id);
    bool             setWorkPaid(int id, bool paid);
    QList<WorkEntry> getWorkEntries(int year = 0, int month = 0,
                                    int clientId = 0,
                                    const QString& type = "");

    // Stats
    double              monthlyEarnings(int year, int month);
    double              unpaidTotal();
    int                 workCountThisMonth();
    int                 totalClients();
    QMap<int,double>    earningsPerMonth(int year);        // key = month 1-12
    QMap<QString,double> earningsByType(int year);

private:
    Database() = default;
    Database(const Database&) = delete;
    void createTables();

    QSqlDatabase m_db;
};
