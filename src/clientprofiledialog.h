#pragma once

#include "database.h"
#include <QDialog>
#include <QLabel>
#include <QTableWidget>
#include <QtCharts>

class ClientProfileDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ClientProfileDialog(const Client& client, QWidget* parent = nullptr);

private:
    void buildUi();
    void loadData();
    void buildActivityChart();

    QWidget* makeStatCard(const QString& title, const QString& value,
                          const QString& color, const QString& icon);

    Client           m_client;
    QList<WorkEntry> m_entries;

    QLabel*      m_lblName         = nullptr;
    QLabel*      m_lblTotalEarned  = nullptr;
    QLabel*      m_lblTotalPaid    = nullptr;
    QLabel*      m_lblUnpaid       = nullptr;
    QLabel*      m_lblJobCount     = nullptr;
    QTableWidget* m_table          = nullptr;
    QChartView*  m_chartView       = nullptr;
};
