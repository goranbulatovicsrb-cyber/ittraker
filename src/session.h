#pragma once

#include <QString>

// ─── Simple session — holds the currently logged-in user ─────────────────────
// Set once at login, read everywhere in DB queries.

class Session
{
public:
    static Session& instance() {
        static Session s;
        return s;
    }

    void   login(int id, const QString& name, bool isAdmin) {
        m_userId  = id;
        m_name    = name;
        m_isAdmin = isAdmin;
    }
    void   logout() { m_userId = -1; m_name.clear(); m_isAdmin = false; }

    int     userId()  const { return m_userId;  }
    QString name()    const { return m_name;    }
    bool    isAdmin() const { return m_isAdmin; }
    bool    isLoggedIn() const { return m_userId > 0; }

private:
    Session() = default;
    int     m_userId  = -1;
    QString m_name;
    bool    m_isAdmin = false;
};
