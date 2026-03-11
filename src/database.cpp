#include "database.h"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

// ─── Singleton ────────────────────────────────────────────────────────────────

Database& Database::instance()
{
    static Database inst;
    return inst;
}

// ─── Init ─────────────────────────────────────────────────────────────────────

bool Database::initialize(const QString& path)
{
    QString dbPath = path;
    if (dbPath.isEmpty()) {
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataDir);
        dbPath = dataDir + "/ittracker.db";
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qCritical() << "Cannot open database:" << m_db.lastError().text();
        return false;
    }

    createTables();
    return true;
}

void Database::createTables()
{
    QSqlQuery q(m_db);

    q.exec(R"(
        CREATE TABLE IF NOT EXISTS clients (
            id      INTEGER PRIMARY KEY AUTOINCREMENT,
            name    TEXT NOT NULL,
            email   TEXT,
            phone   TEXT,
            address TEXT,
            notes   TEXT
        )
    )");

    q.exec(R"(
        CREATE TABLE IF NOT EXISTS work_entries (
            id             INTEGER PRIMARY KEY AUTOINCREMENT,
            client_id      INTEGER NOT NULL,
            work_type      TEXT NOT NULL,
            description    TEXT,
            date           TEXT NOT NULL,
            hours          REAL DEFAULT 1.0,
            price_per_hour REAL DEFAULT 0.0,
            total_price    REAL DEFAULT 0.0,
            is_paid        INTEGER DEFAULT 0,
            FOREIGN KEY(client_id) REFERENCES clients(id) ON DELETE CASCADE
        )
    )");
}

// ─── Clients ──────────────────────────────────────────────────────────────────

int Database::addClient(Client& c)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO clients (name,email,phone,address,notes) VALUES(?,?,?,?,?)");
    q.addBindValue(c.name);
    q.addBindValue(c.email);
    q.addBindValue(c.phone);
    q.addBindValue(c.address);
    q.addBindValue(c.notes);
    if (!q.exec()) {
        qWarning() << "addClient:" << q.lastError().text();
        return -1;
    }
    c.id = q.lastInsertId().toInt();
    return c.id;
}

bool Database::updateClient(const Client& c)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE clients SET name=?,email=?,phone=?,address=?,notes=? WHERE id=?");
    q.addBindValue(c.name);
    q.addBindValue(c.email);
    q.addBindValue(c.phone);
    q.addBindValue(c.address);
    q.addBindValue(c.notes);
    q.addBindValue(c.id);
    return q.exec();
}

bool Database::deleteClient(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM clients WHERE id=?");
    q.addBindValue(id);
    return q.exec();
}

QList<Client> Database::getAllClients()
{
    QList<Client> list;
    QSqlQuery q("SELECT id,name,email,phone,address,notes FROM clients ORDER BY name", m_db);
    while (q.next()) {
        Client c;
        c.id      = q.value(0).toInt();
        c.name    = q.value(1).toString();
        c.email   = q.value(2).toString();
        c.phone   = q.value(3).toString();
        c.address = q.value(4).toString();
        c.notes   = q.value(5).toString();
        list.append(c);
    }
    return list;
}

Client Database::getClientById(int id)
{
    Client c;
    QSqlQuery q(m_db);
    q.prepare("SELECT id,name,email,phone,address,notes FROM clients WHERE id=?");
    q.addBindValue(id);
    if (q.exec() && q.next()) {
        c.id      = q.value(0).toInt();
        c.name    = q.value(1).toString();
        c.email   = q.value(2).toString();
        c.phone   = q.value(3).toString();
        c.address = q.value(4).toString();
        c.notes   = q.value(5).toString();
    }
    return c;
}

// ─── Work entries ─────────────────────────────────────────────────────────────

int Database::addWork(WorkEntry& w)
{
    QSqlQuery q(m_db);
    q.prepare(R"(INSERT INTO work_entries
                 (client_id,work_type,description,date,hours,price_per_hour,total_price,is_paid)
                 VALUES(?,?,?,?,?,?,?,?))");
    q.addBindValue(w.clientId);
    q.addBindValue(w.workType);
    q.addBindValue(w.description);
    q.addBindValue(w.date.toString("yyyy-MM-dd"));
    q.addBindValue(w.hours);
    q.addBindValue(w.pricePerHour);
    q.addBindValue(w.totalPrice);
    q.addBindValue(w.isPaid ? 1 : 0);
    if (!q.exec()) {
        qWarning() << "addWork:" << q.lastError().text();
        return -1;
    }
    w.id = q.lastInsertId().toInt();
    return w.id;
}

bool Database::updateWork(const WorkEntry& w)
{
    QSqlQuery q(m_db);
    q.prepare(R"(UPDATE work_entries SET
                 client_id=?,work_type=?,description=?,date=?,
                 hours=?,price_per_hour=?,total_price=?,is_paid=?
                 WHERE id=?)");
    q.addBindValue(w.clientId);
    q.addBindValue(w.workType);
    q.addBindValue(w.description);
    q.addBindValue(w.date.toString("yyyy-MM-dd"));
    q.addBindValue(w.hours);
    q.addBindValue(w.pricePerHour);
    q.addBindValue(w.totalPrice);
    q.addBindValue(w.isPaid ? 1 : 0);
    q.addBindValue(w.id);
    return q.exec();
}

bool Database::deleteWork(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM work_entries WHERE id=?");
    q.addBindValue(id);
    return q.exec();
}

bool Database::setWorkPaid(int id, bool paid)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE work_entries SET is_paid=? WHERE id=?");
    q.addBindValue(paid ? 1 : 0);
    q.addBindValue(id);
    return q.exec();
}

QList<WorkEntry> Database::getWorkEntries(int year, int month, int clientId, const QString& type)
{
    QList<WorkEntry> list;

    QString sql = R"(
        SELECT w.id, w.client_id, c.name, w.work_type, w.description,
               w.date, w.hours, w.price_per_hour, w.total_price, w.is_paid
        FROM work_entries w
        JOIN clients c ON c.id = w.client_id
        WHERE 1=1
    )";

    if (year  > 0) sql += QString(" AND strftime('%Y',w.date)='%1'").arg(year);
    if (month > 0) sql += QString(" AND strftime('%m',w.date)='%1'").arg(month, 2, 10, QChar('0'));
    if (clientId > 0) sql += QString(" AND w.client_id=%1").arg(clientId);
    if (!type.isEmpty()) sql += QString(" AND w.work_type='%1'").arg(type);

    sql += " ORDER BY w.date DESC, w.id DESC";

    QSqlQuery q(sql, m_db);
    while (q.next()) {
        WorkEntry w;
        w.id           = q.value(0).toInt();
        w.clientId     = q.value(1).toInt();
        w.clientName   = q.value(2).toString();
        w.workType     = q.value(3).toString();
        w.description  = q.value(4).toString();
        w.date         = QDate::fromString(q.value(5).toString(), "yyyy-MM-dd");
        w.hours        = q.value(6).toDouble();
        w.pricePerHour = q.value(7).toDouble();
        w.totalPrice   = q.value(8).toDouble();
        w.isPaid       = q.value(9).toInt() == 1;
        list.append(w);
    }
    return list;
}

// ─── Statistics ───────────────────────────────────────────────────────────────

double Database::monthlyEarnings(int year, int month)
{
    QSqlQuery q(m_db);
    q.prepare(R"(SELECT COALESCE(SUM(total_price),0) FROM work_entries
                 WHERE strftime('%Y',date)=? AND strftime('%m',date)=?)");
    q.addBindValue(QString::number(year));
    q.addBindValue(QString("%1").arg(month, 2, 10, QChar('0')));
    if (q.exec() && q.next()) return q.value(0).toDouble();
    return 0.0;
}

double Database::unpaidTotal()
{
    QSqlQuery q("SELECT COALESCE(SUM(total_price),0) FROM work_entries WHERE is_paid=0", m_db);
    if (q.exec() && q.next()) return q.value(0).toDouble();
    return 0.0;
}

int Database::workCountThisMonth()
{
    QDate today = QDate::currentDate();
    QSqlQuery q(m_db);
    q.prepare("SELECT COUNT(*) FROM work_entries WHERE strftime('%Y',date)=? AND strftime('%m',date)=?");
    q.addBindValue(QString::number(today.year()));
    q.addBindValue(QString("%1").arg(today.month(), 2, 10, QChar('0')));
    if (q.exec() && q.next()) return q.value(0).toInt();
    return 0;
}

int Database::totalClients()
{
    QSqlQuery q("SELECT COUNT(*) FROM clients", m_db);
    if (q.exec() && q.next()) return q.value(0).toInt();
    return 0;
}

QMap<int,double> Database::earningsPerMonth(int year)
{
    QMap<int,double> map;
    for (int i = 1; i <= 12; i++) map[i] = 0.0;

    QSqlQuery q(m_db);
    q.prepare(R"(SELECT CAST(strftime('%m',date) AS INTEGER) AS m,
                        COALESCE(SUM(total_price),0)
                 FROM work_entries
                 WHERE strftime('%Y',date)=?
                 GROUP BY m)");
    q.addBindValue(QString::number(year));
    if (q.exec()) {
        while (q.next()) {
            map[q.value(0).toInt()] = q.value(1).toDouble();
        }
    }
    return map;
}

QMap<QString,double> Database::earningsByType(int year)
{
    QMap<QString,double> map;
    QSqlQuery q(m_db);
    q.prepare(R"(SELECT work_type, COALESCE(SUM(total_price),0)
                 FROM work_entries
                 WHERE strftime('%Y',date)=?
                 GROUP BY work_type)");
    q.addBindValue(QString::number(year));
    if (q.exec()) {
        while (q.next())
            map[q.value(0).toString()] = q.value(1).toDouble();
    }
    return map;
}
