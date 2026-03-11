#include "dashboard.h"
#include "database.h"
#include "style.h"
#include "pdfexporter.h"
#include "backupmanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QDate>
#include <QScrollArea>
#include <QFont>
#include <QPushButton>

// Month names
static const QStringList MONTHS = {
    "Jan","Feb","Mar","Apr","Maj","Jun",
    "Jul","Avg","Sep","Okt","Nov","Dec"
};

// ─── Constructor ──────────────────────────────────────────────────────────────

Dashboard::Dashboard(QWidget* parent) : QWidget(parent)
{
    buildUi();
    refresh();
}

// ─── Build UI ─────────────────────────────────────────────────────────────────

void Dashboard::buildUi()
{
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* content = new QWidget;
    scroll->setWidget(content);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->addWidget(scroll);

    auto* lay = new QVBoxLayout(content);
    lay->setContentsMargins(28, 24, 28, 24);
    lay->setSpacing(20);

    // ── Header ──
    auto* hdr = new QLabel("📊  Dashboard");
    hdr->setObjectName("titleLabel");
    lay->addWidget(hdr);

    auto* sub = new QLabel(QString("Dobrodošli! Danas je %1")
                               .arg(QDate::currentDate().toString("dd.MM.yyyy")));
    sub->setObjectName("subtitleLabel");
    lay->addWidget(sub);

    // ── Stat cards row ──
    auto* cardsLay = new QHBoxLayout;
    cardsLay->setSpacing(16);
    cardsLay->addWidget(makeStatCard("Zarada ovog meseca", Style::GREEN,  m_lblMonthEarnings));
    cardsLay->addWidget(makeStatCard("Neplaćeno",          Style::RED,    m_lblUnpaid));
    cardsLay->addWidget(makeStatCard("Poslovi ovog meseca",Style::BLUE,   m_lblJobsThisMonth));
    cardsLay->addWidget(makeStatCard("Ukupno mušterija",   Style::PURPLE, m_lblTotalClients));
    lay->addLayout(cardsLay);

    // ── Charts row ──
    // Year selector + action buttons
    auto* actionsRow = new QHBoxLayout;
    actionsRow->setSpacing(10);

    auto* lblYear = new QLabel("Prikaži godinu:");
    lblYear->setStyleSheet("color:#8b949e; font-weight:600;");
    actionsRow->addWidget(lblYear);

    m_spnYear = new QSpinBox;
    m_spnYear->setRange(2000, 2099);
    m_spnYear->setValue(QDate::currentDate().year());
    m_spnYear->setFixedWidth(90);
    m_spnYear->setAlignment(Qt::AlignCenter);
    m_spnYear->setStyleSheet(
        "QSpinBox { padding: 4px 2px 4px 8px; font-weight:600; color:#f0f6fc; }"
    );
    actionsRow->addWidget(m_spnYear);

    actionsRow->addStretch();

    auto* btnReport = new QPushButton("📄  Godišnji izvještaj PDF");
    btnReport->setObjectName("btnPrimary");
    auto* btnBackup = new QPushButton("💾  Backup baze");
    btnBackup->setObjectName("btnSuccess");
    auto* btnRestore = new QPushButton("🔄  Obnovi backup");

    actionsRow->addWidget(btnReport);
    actionsRow->addWidget(btnBackup);
    actionsRow->addWidget(btnRestore);
    lay->addLayout(actionsRow);

    connect(btnReport,  &QPushButton::clicked, this, &Dashboard::onExportAnnualReport);
    connect(btnBackup,  &QPushButton::clicked, this, &Dashboard::onBackup);
    connect(btnRestore, &QPushButton::clicked, this, &Dashboard::onRestore);
    connect(m_spnYear, qOverload<int>(&QSpinBox::valueChanged), this, [this]{ updateBarChart(); updatePieChart(); });

    auto* chartsLay = new QHBoxLayout;
    chartsLay->setSpacing(16);

    // Bar chart card
    auto* barCard = new QFrame;
    barCard->setObjectName("card");
    auto* barCardLay = new QVBoxLayout(barCard);
    barCardLay->setContentsMargins(16,16,16,16);
    auto* barTitle = new QLabel("Zarada po mesecima — " + QString::number(m_spnYear->value()));
    barTitle->setStyleSheet("font-weight:600; font-size:14px; color:#f0f6fc;");
    barCardLay->addWidget(barTitle);
    m_barView = new QChartView;
    m_barView->setRenderHint(QPainter::Antialiasing);
    m_barView->setMinimumHeight(260);
    barCardLay->addWidget(m_barView);
    chartsLay->addWidget(barCard, 3);

    // Pie chart card
    auto* pieCard = new QFrame;
    pieCard->setObjectName("card");
    auto* pieCardLay = new QVBoxLayout(pieCard);
    pieCardLay->setContentsMargins(16,16,16,16);
    auto* pieTitle = new QLabel("Zarada po vrsti posla");
    pieTitle->setStyleSheet("font-weight:600; font-size:14px; color:#f0f6fc;");
    pieCardLay->addWidget(pieTitle);
    m_pieView = new QChartView;
    m_pieView->setRenderHint(QPainter::Antialiasing);
    m_pieView->setMinimumHeight(260);
    pieCardLay->addWidget(m_pieView);
    chartsLay->addWidget(pieCard, 2);

    lay->addLayout(chartsLay);
    lay->addStretch();
}

QWidget* Dashboard::makeStatCard(const QString& title, const QString& color, QLabel*& valueLabel)
{
    auto* card = new QFrame;
    card->setObjectName("card");
    card->setMinimumWidth(160);

    auto* lay = new QVBoxLayout(card);
    lay->setContentsMargins(20, 16, 20, 16);
    lay->setSpacing(6);

    auto* lbl = new QLabel(title);
    lbl->setStyleSheet("font-size:11px; font-weight:600; color:#8b949e; text-transform:uppercase; letter-spacing:1px;");
    lbl->setWordWrap(true);
    lay->addWidget(lbl);

    valueLabel = new QLabel("—");
    valueLabel->setStyleSheet(QString("font-size:26px; font-weight:700; color:%1;").arg(color));
    lay->addWidget(valueLabel);

    // colored bottom bar
    auto* bar = new QFrame;
    bar->setFixedHeight(3);
    bar->setStyleSheet(QString("background:%1; border-radius:2px;").arg(color));
    lay->addWidget(bar);

    return card;
}

// ─── Refresh ──────────────────────────────────────────────────────────────────

void Dashboard::refresh()
{
    updateStats();
    updateBarChart();
    updatePieChart();
}

void Dashboard::updateStats()
{
    auto& db = Database::instance();
    QDate today = QDate::currentDate();

    double monthly = db.monthlyEarnings(today.year(), today.month());
    double unpaid  = db.unpaidTotal();
    int    jobs    = db.workCountThisMonth();
    int    clients = db.totalClients();

    m_lblMonthEarnings->setText(QString("%1 €").arg(monthly, 0, 'f', 2));
    m_lblUnpaid->setText(QString("%1 €").arg(unpaid, 0, 'f', 2));
    m_lblJobsThisMonth->setText(QString::number(jobs));
    m_lblTotalClients->setText(QString::number(clients));
}

void Dashboard::onExportAnnualReport()
{
    PdfExporter::exportAnnualReport(m_spnYear->value(), this);
}

void Dashboard::onBackup()
{
    BackupManager::backup(this);
}

void Dashboard::onRestore()
{
    BackupManager::restore(this);
}

void Dashboard::updateBarChart()
{
    int year = m_spnYear->value();
    auto monthly = Database::instance().earningsPerMonth(year);

    auto* series = new QBarSeries;
    auto* set = new QBarSet("Zarada (€)");
    set->setColor(QColor("#1f6feb"));
    set->setBorderColor(QColor("#58a6ff"));
    for (int m = 1; m <= 12; m++)
        *set << monthly.value(m, 0.0);
    series->append(set);
    series->setLabelsVisible(false);

    auto* chart = new QChart;
    chart->addSeries(series);
    chart->setTitle("");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setBackgroundBrush(QColor("#161b22"));
    chart->setBackgroundPen(Qt::NoPen);
    chart->setPlotAreaBackgroundBrush(QColor("#0d1117"));
    chart->setPlotAreaBackgroundVisible(true);
    chart->legend()->setVisible(false);

    auto* axisX = new QBarCategoryAxis;
    axisX->append(MONTHS);
    axisX->setLabelsColor(QColor("#8b949e"));
    axisX->setGridLineColor(Qt::transparent);
    axisX->setLinePen(QPen(QColor("#30363d")));
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    auto* axisY = new QValueAxis;
    axisY->setLabelFormat("%.0f €");
    axisY->setLabelsColor(QColor("#8b949e"));
    axisY->setGridLineColor(QColor("#21262d"));
    axisY->setLinePen(QPen(Qt::transparent));
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    m_barView->setChart(chart);
    m_barView->setBackgroundBrush(QColor("#161b22"));
    m_barView->setFrameShape(QFrame::NoFrame);
}

void Dashboard::updatePieChart()
{
    int year = m_spnYear->value();
    auto byType = Database::instance().earningsByType(year);

    auto* series = new QPieSeries;
    series->setHoleSize(0.4);

    static const QStringList COLORS = {
        "#58a6ff","#3fb950","#f85149","#d29922","#bc8cff","#ffa657"
    };

    int ci = 0;
    for (auto it = byType.cbegin(); it != byType.cend(); ++it, ++ci) {
        if (it.value() <= 0.0) continue;
        auto* slice = series->append(it.key(), it.value());
        slice->setColor(QColor(COLORS[ci % COLORS.size()]));
        slice->setBorderColor(QColor("#0d1117"));
        slice->setBorderWidth(2);
        slice->setLabelVisible(true);
        slice->setLabelColor(QColor("#f0f6fc"));
        slice->setLabel(QString("%1\n%2 €").arg(it.key()).arg(it.value(), 0, 'f', 0));
    }

    if (series->count() == 0) {
        auto* sl = series->append("Nema podataka", 1);
        sl->setColor(QColor("#30363d"));
        sl->setBorderColor(QColor("#0d1117"));
        sl->setLabelVisible(true);
        sl->setLabelColor(QColor("#8b949e"));
        sl->setLabel("Nema podataka");
    }

    auto* chart = new QChart;
    chart->addSeries(series);
    chart->setTitle("");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setBackgroundBrush(QColor("#161b22"));
    chart->setBackgroundPen(Qt::NoPen);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->legend()->setLabelColor(QColor("#8b949e"));
    chart->legend()->setBackgroundVisible(false);

    m_pieView->setChart(chart);
    m_pieView->setBackgroundBrush(QColor("#161b22"));
    m_pieView->setFrameShape(QFrame::NoFrame);
}
