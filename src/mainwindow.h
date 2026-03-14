#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>

class Dashboard;
class ClientsPage;
class WorkLogPage;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onLogout();
    void onUserManager();

private:
    void buildUi();
    void switchPage(int index);

    QStackedWidget* m_stack   = nullptr;
    Dashboard*      m_dash    = nullptr;
    ClientsPage*    m_clients = nullptr;
    WorkLogPage*    m_worklog = nullptr;

    QLabel*         m_lblUser = nullptr;
    QList<QPushButton*> m_navBtns;
};
