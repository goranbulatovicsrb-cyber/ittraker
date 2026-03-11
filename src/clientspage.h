#pragma once

#include "database.h"
#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

class ClientsPage : public QWidget
{
    Q_OBJECT
public:
    explicit ClientsPage(QWidget* parent = nullptr);
    void refresh();

private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void onFilter(const QString& text);
    void onSelectionChanged();

private:
    void buildUi();
    void populateTable(const QList<Client>& clients);
    int  selectedClientId() const;

    QTableWidget* m_table   = nullptr;
    QLineEdit*    m_search  = nullptr;
    QPushButton*  m_btnEdit = nullptr;
    QPushButton*  m_btnDel  = nullptr;
    QLabel*       m_lblCount= nullptr;

    QList<Client> m_allClients;
};
