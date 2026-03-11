#pragma once

#include <QWidget>
#include <QLabel>
#include <QtCharts>

class Dashboard : public QWidget
{
    Q_OBJECT
public:
    explicit Dashboard(QWidget* parent = nullptr);
    void refresh();

private:
    void buildUi();
    void updateStats();
    void updateBarChart();
    void updatePieChart();

    // Stat cards
    QLabel* m_lblMonthEarnings   = nullptr;
    QLabel* m_lblUnpaid          = nullptr;
    QLabel* m_lblJobsThisMonth   = nullptr;
    QLabel* m_lblTotalClients    = nullptr;

    // Charts
    QChartView* m_barView  = nullptr;
    QChartView* m_pieView  = nullptr;

    QWidget* makeStatCard(const QString& title, const QString& color,
                          QLabel*& valueLabel);
};
