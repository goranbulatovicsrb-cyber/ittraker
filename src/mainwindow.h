#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>

class Dashboard;
class ClientsPage;
class WorkLogPage;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void buildUi();
    void switchPage(int index);

    QStackedWidget* m_stack    = nullptr;
    Dashboard*      m_dash     = nullptr;
    ClientsPage*    m_clients  = nullptr;
    WorkLogPage*    m_worklog  = nullptr;

    QList<QPushButton*> m_navBtns;
};
