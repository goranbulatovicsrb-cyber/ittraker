#include "database.h"
#include "session.h"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QCryptographicHash>

// ─── Singleton ────────────────────────────────────────────────────────────────

Database& Database::instance()
{
    static Database inst;
    return inst;
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

QString Database::hashPassword(const QString& pw)
{
    return QString(QCryptographicHash::hash(pw.toUtf8(), QCryptographicHash::Sha256).toHex());
}

int Database::currentUserId() const
{
    return Session::instance().userId();
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

    // Enable foreign keys
    QSqlQuery q(m_db);
    q.exec("PRAGMA foreign_keys = ON");

    createTables();
    migrateExistingData();
    return true;
}

void Database::createTables()
{
    QSqlQuery q(m_db);

    // Users table
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS app_users (
            id           INTEGER PRIMARY KEY AUTOINCREMENT,
            username     TEXT NOT NULL UNIQUE,
            display_name TEXT NOT NULL,
            password_hash TEXT NOT NULL,
            is_admin     INTEGER DEFAULT 0
        )
    )");

    // Clients table with user_id
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS clients (
            id      INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL DEFAULT 1,
            name    TEXT NOT NULL,
            email   TEXT,
            phone   TEXT,
            address TEXT,
            notes   TEXT,
            FOREIGN KEY(user_id) REFERENCES app_users(id) ON DELETE CASCADE
        )
    )");

    // Work entries with user_id
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS work_entries (
            id             INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id        INTEGER NOT NULL DEFAULT 1,
            client_id      INTEGER NOT NULL,
            work_type      TEXT NOT NULL,
            description    TEXT,
            date           TEXT NOT NULL,
            hours          REAL DEFAULT 1.0,
            price_per_hour REAL DEFAULT 0.0,
            total_price    REAL DEFAULT 0.0,
            is_paid        INTEGER DEFAULT 0,
            FOREIGN KEY(user_id)   REFERENCES app_users(id) ON DELETE CASCADE,
            FOREIGN KEY(client_id) REFERENCES clients(id)   ON DELETE CASCADE
        )
    )");
}

void Database::migrateExistingData()
{
    QSqlQuery q(m_db);

    // Add user_id columns if they don't exist yet (migration for old DBs)
    q.exec("ALTER TABLE clients      ADD COLUMN user_id INTEGER NOT NULL DEFAULT 1");
    q.exec("ALTER TABLE work_entries ADD COLUMN user_id INTEGER NOT NULL DEFAULT 1");

    // If no users exist, create default admin
    q.exec("SELECT COUNT(*) FROM app_users");
    if (q.next() && q.value(0).toInt() == 0) {
        q.prepare("INSERT INTO app_users (username, display_name, password_hash, is_admin) VALUES(?,?,?,?)");
        q.addBindValue("admin");
        q.addBindValue("Administrator");
        q.addBindValue(hashPassword("admin"));
        q.addBindValue(1);
        q.exec();
        qDebug() << "Created default admin user (admin/admin)";
    }
}

// ─── Users ────────────────────────────────────────────────────────────────────

int Database::addUser(AppUser& u, const QString& password)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO app_users (username, display_name, password_hash, is_admin) VALUES(?,?,?,?)");
    q.addBindValue(u.username.trimmed().toLower());
    q.addBindValue(u.displayName);
    q.addBindValue(hashPassword(password));
    q.addBindValue(u.isAdmin ? 1 : 0);
    if (!q.exec()) {
        qWarning() << "addUser:" << q.lastError().text();
        return -1;
    }
    u.id = q.lastInsertId().toInt();
    return u.id;
}

bool Database::deleteUser(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM app_users WHERE id=? AND is_admin=0");
    q.addBindValue(id);
    return q.exec() && q.numRowsAffected() > 0;
}

bool Database::updateUserPassword(int id, const QString& newPassword)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE app_users SET password_hash=? WHERE id=?");
    q.addBindValue(hashPassword(newPassword));
    q.addBindValue(id);
    return q.exec();
}

QList<AppUser> Database::getAllUsers()
{
    QList<AppUser> list;
    QSqlQuery q("SELECT id, username, display_name, is_admin FROM app_users ORDER BY display_name", m_db);
    while (q.next()) {
        AppUser u;
        u.id          = q.value(0).toInt();
        u.username    = q.value(1).toString();
        u.displayName = q.value(2).toString();
        u.isAdmin     = q.value(3).toInt() == 1;
        list.append(u);
    }
    return list;
}

AppUser Database::authenticate(const QString& username, const QString& password)
{
    AppUser u;
    QSqlQuery q(m_db);
    q.prepare("SELECT id, username, display_name, is_admin FROM app_users "
              "WHERE username=? AND password_hash=?");
    q.addBindValue(username.trimmed().toLower());
    q.addBindValue(hashPassword(password));
    if (q.exec() && q.next()) {
        u.id          = q.value(0).toInt();
        u.username    = q.value(1).toString();
        u.displayName = q.value(2).toString();
        u.isAdmin     = q.value(3).toInt() == 1;
    }
    return u;
}

// ─── Clients ──────────────────────────────────────────────────────────────────

int Database::addClient(Client& c)
{
    c.userId = currentUserId();
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO clients (user_id,name,email,phone,address,notes) VALUES(?,?,?,?,?,?)");
    q.addBindValue(c.userId);
    q.addBindValue(c.name);
    q.addBindValue(c.email);
    q.addBindValue(c.phone);
    q.addBindValue(c.address);
    q.addBindValue(c.notes);
    if (!q.exec()) { qWarning() << "addClient:" << q.lastError().text(); return -1; }
    c.id = q.lastInsertId().toInt();
    return c.id;
}

bool Database::updateClient(const Client& c)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE clients SET name=?,email=?,phone=?,address=?,notes=? WHERE id=? AND user_id=?");
    q.addBindValue(c.name);   q.addBindValue(c.email);
    q.addBindValue(c.phone);  q.addBindValue(c.address);
    q.addBindValue(c.notes);  q.addBindValue(c.id);
    q.addBindValue(currentUserId());
    return q.exec();
}

bool Database::deleteClient(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM clients WHERE id=? AND user_id=?");
    q.addBindValue(id); q.addBindValue(currentUserId());
    return q.exec();
}

QList<Client> Database::getAllClients()
{
    QList<Client> list;
    QSqlQuery q(m_db);
    q.prepare("SELECT id,user_id,name,email,phone,address,notes FROM clients WHERE user_id=? ORDER BY name");
    q.addBindValue(currentUserId());
    q.exec();
    while (q.next()) {
        Client c;
        c.id      = q.value(0).toInt();
        c.userId  = q.value(1).toInt();
        c.name    = q.value(2).toString();
        c.email   = q.value(3).toString();
        c.phone   = q.value(4).toString();
        c.address = q.value(5).toString();
        c.notes   = q.value(6).toString();
        list.append(c);
    }
    return list;
}

Client Database::getClientById(int id)
{
    Client c;
    QSqlQuery q(m_db);
    q.prepare("SELECT id,user_id,name,email,phone,address,notes FROM clients WHERE id=? AND user_id=?");
    q.addBindValue(id); q.addBindValue(currentUserId());
    if (q.exec() && q.next()) {
        c.id=q.value(0).toInt(); c.userId=q.value(1).toInt();
        c.name=q.value(2).toString(); c.email=q.value(3).toString();
        c.phone=q.value(4).toString(); c.address=q.value(5).toString();
        c.notes=q.value(6).toString();
    }
    return c;
}

// ─── Work entries ─────────────────────────────────────────────────────────────

int Database::addWork(WorkEntry& w)
{
    w.userId = currentUserId();
    QSqlQuery q(m_db);
    q.prepare(R"(INSERT INTO work_entries
                 (user_id,client_id,work_type,description,date,hours,price_per_hour,total_price,is_paid)
                 VALUES(?,?,?,?,?,?,?,?,?))");
    q.addBindValue(w.userId);     q.addBindValue(w.clientId);
    q.addBindValue(w.workType);   q.addBindValue(w.description);
    q.addBindValue(w.date.toString("yyyy-MM-dd"));
    q.addBindValue(w.hours);      q.addBindValue(w.pricePerHour);
    q.addBindValue(w.totalPrice); q.addBindValue(w.isPaid ? 1 : 0);
    if (!q.exec()) { qWarning() << "addWork:" << q.lastError().text(); return -1; }
    w.id = q.lastInsertId().toInt();
    return w.id;
}

bool Database::updateWork(const WorkEntry& w)
{
    QSqlQuery q(m_db);
    q.prepare(R"(UPDATE work_entries SET
                 client_id=?,work_type=?,description=?,date=?,
                 hours=?,price_per_hour=?,total_price=?,is_paid=?
                 WHERE id=? AND user_id=?)");
    q.addBindValue(w.clientId);    q.addBindValue(w.workType);
    q.addBindValue(w.description); q.addBindValue(w.date.toString("yyyy-MM-dd"));
    q.addBindValue(w.hours);       q.addBindValue(w.pricePerHour);
    q.addBindValue(w.totalPrice);  q.addBindValue(w.isPaid ? 1 : 0);
    q.addBindValue(w.id);          q.addBindValue(currentUserId());
    return q.exec();
}

bool Database::deleteWork(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM work_entries WHERE id=? AND user_id=?");
    q.addBindValue(id); q.addBindValue(currentUserId());
    return q.exec();
}

bool Database::setWorkPaid(int id, bool paid)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE work_entries SET is_paid=? WHERE id=? AND user_id=?");
    q.addBindValue(paid?1:0); q.addBindValue(id); q.addBindValue(currentUserId());
    return q.exec();
}

QList<WorkEntry> Database::getWorkEntries(int year, int month, int clientId, const QString& type)
{
    QList<WorkEntry> list;
    QString sql = R"(
        SELECT w.id, w.user_id, w.client_id, c.name, w.work_type, w.description,
               w.date, w.hours, w.price_per_hour, w.total_price, w.is_paid
        FROM work_entries w
        JOIN clients c ON c.id = w.client_id
        WHERE w.user_id=?
    )";
    if (year   > 0) sql += QString(" AND strftime('%Y',w.date)='%1'").arg(year);
    if (month  > 0) sql += QString(" AND strftime('%m',w.date)='%1'").arg(month,2,10,QChar('0'));
    if (clientId>0) sql += QString(" AND w.client_id=%1").arg(clientId);
    if (!type.isEmpty()) sql += QString(" AND w.work_type='%1'").arg(type);
    sql += " ORDER BY w.date DESC, w.id DESC";

    QSqlQuery q(m_db);
    q.prepare(sql);
    q.addBindValue(currentUserId());
    q.exec();
    while (q.next()) {
        WorkEntry w;
        w.id=q.value(0).toInt();   w.userId=q.value(1).toInt();
        w.clientId=q.value(2).toInt(); w.clientName=q.value(3).toString();
        w.workType=q.value(4).toString(); w.description=q.value(5).toString();
        w.date=QDate::fromString(q.value(6).toString(),"yyyy-MM-dd");
        w.hours=q.value(7).toDouble(); w.pricePerHour=q.value(8).toDouble();
        w.totalPrice=q.value(9).toDouble(); w.isPaid=q.value(10).toInt()==1;
        list.append(w);
    }
    return list;
}

// ─── Statistics ───────────────────────────────────────────────────────────────

double Database::monthlyEarnings(int year, int month)
{
    QSqlQuery q(m_db);
    q.prepare(R"(SELECT COALESCE(SUM(total_price),0) FROM work_entries
                 WHERE user_id=? AND strftime('%Y',date)=? AND strftime('%m',date)=?)");
    q.addBindValue(currentUserId());
    q.addBindValue(QString::number(year));
    q.addBindValue(QString("%1").arg(month,2,10,QChar('0')));
    if (q.exec() && q.next()) return q.value(0).toDouble();
    return 0.0;
}

double Database::unpaidTotal()
{
    QSqlQuery q(m_db);
    q.prepare("SELECT COALESCE(SUM(total_price),0) FROM work_entries WHERE user_id=? AND is_paid=0");
    q.addBindValue(currentUserId());
    if (q.exec() && q.next()) return q.value(0).toDouble();
    return 0.0;
}

int Database::workCountThisMonth()
{
    QDate today = QDate::currentDate();
    QSqlQuery q(m_db);
    q.prepare("SELECT COUNT(*) FROM work_entries WHERE user_id=? AND strftime('%Y',date)=? AND strftime('%m',date)=?");
    q.addBindValue(currentUserId());
    q.addBindValue(QString::number(today.year()));
    q.addBindValue(QString("%1").arg(today.month(),2,10,QChar('0')));
    if (q.exec() && q.next()) return q.value(0).toInt();
    return 0;
}

int Database::totalClients()
{
    QSqlQuery q(m_db);
    q.prepare("SELECT COUNT(*) FROM clients WHERE user_id=?");
    q.addBindValue(currentUserId());
    if (q.exec() && q.next()) return q.value(0).toInt();
    return 0;
}

QMap<int,double> Database::earningsPerMonth(int year)
{
    QMap<int,double> map;
    for (int i=1;i<=12;i++) map[i]=0.0;
    QSqlQuery q(m_db);
    q.prepare(R"(SELECT CAST(strftime('%m',date) AS INTEGER),COALESCE(SUM(total_price),0)
                 FROM work_entries WHERE user_id=? AND strftime('%Y',date)=?
                 GROUP BY strftime('%m',date))");
    q.addBindValue(currentUserId());
    q.addBindValue(QString::number(year));
    if (q.exec()) while (q.next()) map[q.value(0).toInt()]=q.value(1).toDouble();
    return map;
}

QMap<QString,double> Database::earningsByType(int year)
{
    QMap<QString,double> map;
    QSqlQuery q(m_db);
    q.prepare(R"(SELECT work_type,COALESCE(SUM(total_price),0)
                 FROM work_entries WHERE user_id=? AND strftime('%Y',date)=?
                 GROUP BY work_type)");
    q.addBindValue(currentUserId());
    q.addBindValue(QString::number(year));
    if (q.exec()) while (q.next()) map[q.value(0).toString()]=q.value(1).toDouble();
    return map;
}
