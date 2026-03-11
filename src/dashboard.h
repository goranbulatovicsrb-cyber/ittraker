#pragma once

#include <QWidget>
#include <QLabel>
#include <QSpinBox>
#include <QtCharts>

class Dashboard : public QWidget
{
    Q_OBJECT
public:
    explicit Dashboard(QWidget* parent = nullptr);
    void refresh();

private slots:
    void onExportAnnualReport();
    void onBackup();
    void onRestore();

private:
    void buildUi();
    void updateStats();
    void updateBarChart();
    void updatePieChart();

    QLabel*     m_lblMonthEarnings = nullptr;
    QLabel*     m_lblUnpaid        = nullptr;
    QLabel*     m_lblJobsThisMonth = nullptr;
    QLabel*     m_lblTotalClients  = nullptr;
    QSpinBox*   m_spnYear          = nullptr;

    QChartView* m_barView = nullptr;
    QChartView* m_pieView = nullptr;

    QWidget* makeStatCard(const QString& title, const QString& color,
                          QLabel*& valueLabel);
};
