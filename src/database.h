#pragma once

#include <QObject>
#include <QtSql>
#include <QDate>
#include <QList>
#include <QMap>
#include <QString>

// ─── Data structures ─────────────────────────────────────────────────────────

struct AppUser {
    int     id       = -1;
    QString username;
    QString displayName;
    bool    isAdmin  = false;
};

struct Client {
    int     id      = -1;
    int     userId  = -1;   // owner
    QString name;
    QString email;
    QString phone;
    QString address;
    QString notes;
};

struct WorkEntry {
    int     id           = -1;
    int     userId       = -1;  // owner
    int     clientId     = -1;
    QString clientName;
    QString workType;
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

    // ── Users ────────────────────────────────────────────────────────────────
    int           addUser(AppUser& u, const QString& password);
    bool          deleteUser(int id);
    bool          updateUserPassword(int id, const QString& newPassword);
    QList<AppUser> getAllUsers();
    // Returns user if credentials match, else id==-1
    AppUser       authenticate(const QString& username, const QString& password);

    // ── Clients (scoped to current user via Session) ──────────────────────
    int           addClient(Client& c);
    bool          updateClient(const Client& c);
    bool          deleteClient(int id);
    QList<Client> getAllClients();          // filtered by Session::userId
    Client        getClientById(int id);

    // ── Work entries (scoped to current user via Session) ─────────────────
    int              addWork(WorkEntry& w);
    bool             updateWork(const WorkEntry& w);
    bool             deleteWork(int id);
    bool             setWorkPaid(int id, bool paid);
    QList<WorkEntry> getWorkEntries(int year = 0, int month = 0,
                                    int clientId = 0,
                                    const QString& type = "");

    // ── Stats (scoped to current user) ───────────────────────────────────
    double               monthlyEarnings(int year, int month);
    double               unpaidTotal();
    int                  workCountThisMonth();
    int                  totalClients();
    QMap<int,double>     earningsPerMonth(int year);
    QMap<QString,double> earningsByType(int year);

private:
    Database() = default;
    Database(const Database&) = delete;
    void createTables();
    void migrateExistingData();
    QString hashPassword(const QString& pw);
    int currentUserId() const;

    QSqlDatabase m_db;
};
