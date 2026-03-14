#pragma once

#include "database.h"
#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QLabel>
#include <QStackedWidget>

class LoginScreen : public QDialog
{
    Q_OBJECT
public:
    explicit LoginScreen(QWidget* parent = nullptr);

private slots:
    void onUserSelected(int row);
    void onLogin();
    void onBack();

private:
    void buildUi();
    void loadUsers();

    // Page 0: user list
    QListWidget* m_userList   = nullptr;
    QLabel*      m_lblSelect  = nullptr;

    // Page 1: password entry
    QLabel*      m_lblWelcome = nullptr;
    QLineEdit*   m_edtPass    = nullptr;
    QLabel*      m_lblError   = nullptr;

    QStackedWidget* m_stack   = nullptr;

    AppUser      m_selectedUser;
};
