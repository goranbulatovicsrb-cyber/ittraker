#pragma once

#include "database.h"
#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>

class WorkLogPage : public QWidget
{
    Q_OBJECT
public:
    explicit WorkLogPage(QWidget* parent = nullptr);
    void refresh();

signals:
    void dataChanged();   // notify dashboard to refresh

private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void onTogglePaid();
    void applyFilters();
    void onSelectionChanged();

private:
    void buildUi();
    void populateTable(const QList<WorkEntry>& entries);
    int  selectedEntryId() const;
    WorkEntry selectedEntry() const;

    QComboBox*   m_cmbYear    = nullptr;
    QComboBox*   m_cmbMonth   = nullptr;
    QComboBox*   m_cmbClient  = nullptr;
    QComboBox*   m_cmbType    = nullptr;
    QTableWidget* m_table     = nullptr;
    QPushButton* m_btnEdit    = nullptr;
    QPushButton* m_btnDel     = nullptr;
    QPushButton* m_btnPaid    = nullptr;
    QLabel*      m_lblTotal   = nullptr;
    QLabel*      m_lblCount   = nullptr;
};
