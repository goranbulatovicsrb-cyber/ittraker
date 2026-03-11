#pragma once

#include "database.h"
#include "paginationwidget.h"
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
    void dataChanged();

private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void onTogglePaid();
    void onExportCsv();
    void onExportInvoice();
    void applyFilters();
    void onSelectionChanged();
    void onPageChanged();

private:
    void buildUi();
    void populateTable();
    int  selectedEntryId() const;
    WorkEntry selectedEntry() const;

    QList<WorkEntry>  m_allEntries;

    QComboBox*        m_cmbYear    = nullptr;
    QComboBox*        m_cmbMonth   = nullptr;
    QComboBox*        m_cmbClient  = nullptr;
    QComboBox*        m_cmbType    = nullptr;
    QTableWidget*     m_table      = nullptr;
    QPushButton*      m_btnEdit    = nullptr;
    QPushButton*      m_btnDel     = nullptr;
    QPushButton*      m_btnPaid    = nullptr;
    QPushButton*      m_btnInvoice = nullptr;
    QLabel*           m_lblTotal   = nullptr;
    QLabel*           m_lblCount   = nullptr;
    PaginationWidget* m_pager      = nullptr;
};
