#include "clientprofiledialog.h"
#include "pdfexporter.h"
#include "style.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFrame>
#include <QScrollArea>
#include <QPushButton>
#include <QTableWidgetItem>
#include <QDate>
#include <QFont>
#include <QMap>

static const QStringList MONTH_SHORT_P = {
    "","Jan","Feb","Mar","Apr","Maj","Jun",
    "Jul","Avg","Sep","Okt","Nov","Dec"
};

// ─── Constructor ──────────────────────────────────────────────────────────────

ClientProfileDialog::ClientProfileDialog(const Client& client, QWidget* parent)
    : QDialog(parent), m_client(client)
{
    setWindowTitle("👤  Profil mušterije");
    setModal(true);
    setMinimumSize(820, 620);
    resize(900, 680);
    buildUi();
    loadData();
}

// ─── Build UI ─────────────────────────────────────────────────────────────────

QWidget* ClientProfileDialog::makeStatCard(const QString& title,
                                            const QString& value,
                                            const QString& color,
                                            const QString& icon)
{
    auto* card = new QFrame;
    card->setObjectName("card");
    card->setMinimumWidth(140);

    auto* lay = new QVBoxLayout(card);
    lay->setContentsMargins(16,12,16,12);
    lay->setSpacing(4);

    auto* lIcon = new QLabel(icon);
    lIcon->setStyleSheet("font-size:20px; background:transparent;");
    auto* lTitle = new QLabel(title);
    lTitle->setStyleSheet("font-size:10px; font-weight:700; color:#8b949e; "
                          "text-transform:uppercase; letter-spacing:1px; background:transparent;");
    auto* lValue = new QLabel(value);
    lValue->setObjectName("statVal_" + color);
    lValue->setStyleSheet(QString("font-size:20px; font-weight:700; color:%1; background:transparent;").arg(color));

    lay->addWidget(lIcon);
    lay->addWidget(lTitle);
    lay->addWidget(lValue);

    // Bottom accent bar
    auto* bar = new QFrame;
    bar->setFixedHeight(3);
    bar->setStyleSheet(QString("background:%1; border-radius:2px;").arg(color));
    lay->addWidget(bar);

    return card;
}

void ClientProfileDialog::buildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    // ── Header banner ──
    auto* banner = new QFrame;
    banner->setFixedHeight(90);
    banner->setStyleSheet("background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
                          "stop:0 #0d1117, stop:1 #161b22);"
                          "border-bottom:1px solid #30363d;");
    auto* bannerLay = new QHBoxLayout(banner);
    bannerLay->setContentsMargins(28,0,28,0);

    // Avatar circle
    auto* avatar = new QLabel(QString(m_client.name[0].toUpper()));
    avatar->setFixedSize(56,56);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet("background:#1f6feb; color:#fff; font-size:22px; "
                          "font-weight:700; border-radius:28px;");

    auto* nameCol = new QVBoxLayout;
    m_lblName = new QLabel(m_client.name);
    m_lblName->setStyleSheet("font-size:20px; font-weight:700; color:#f0f6fc;");
    auto* subInfo = new QLabel;
    QStringList parts;
    if (!m_client.phone.isEmpty()) parts << "📞 " + m_client.phone;
    if (!m_client.email.isEmpty()) parts << "✉️  " + m_client.email;
    if (!m_client.address.isEmpty()) parts << "📍 " + m_client.address;
    subInfo->setText(parts.join("   "));
    subInfo->setStyleSheet("font-size:12px; color:#8b949e;");
    nameCol->addWidget(m_lblName);
    nameCol->addWidget(subInfo);

    bannerLay->addWidget(avatar);
    bannerLay->addSpacing(14);
    bannerLay->addLayout(nameCol, 1);

    auto* btnInvoiceAll = new QPushButton("📄  Izvještaj klijenta PDF");
    btnInvoiceAll->setObjectName("btnPrimary");
    bannerLay->addWidget(btnInvoiceAll);

    root->addWidget(banner);

    // ── Scroll content ──
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* content = new QWidget;
    scroll->setWidget(content);
    root->addWidget(scroll, 1);

    auto* cLay = new QVBoxLayout(content);
    cLay->setContentsMargins(24,20,24,20);
    cLay->setSpacing(18);

    // ── Stat cards ──
    auto* statsRow = new QHBoxLayout;
    statsRow->setSpacing(12);

    // placeholders — filled in loadData()
    auto* card1 = makeStatCard("Ukupno fakturisano", "—", Style::BLUE,    "💰");
    auto* card2 = makeStatCard("Plaćeno",            "—", Style::GREEN,   "✅");
    auto* card3 = makeStatCard("Duguje",             "—", Style::RED,     "⏳");
    auto* card4 = makeStatCard("Broj poslova",       "—", Style::PURPLE,  "🔧");

    // Store label refs via findChild
    m_lblTotalEarned = card1->findChild<QLabel*>("statVal_" + QString(Style::BLUE));
    m_lblTotalPaid   = card2->findChild<QLabel*>("statVal_" + QString(Style::GREEN));
    m_lblUnpaid      = card3->findChild<QLabel*>("statVal_" + QString(Style::RED));
    m_lblJobCount    = card4->findChild<QLabel*>("statVal_" + QString(Style::PURPLE));

    statsRow->addWidget(card1);
    statsRow->addWidget(card2);
    statsRow->addWidget(card3);
    statsRow->addWidget(card4);
    cLay->addLayout(statsRow);

    // ── Chart + notes row ──
    auto* midRow = new QHBoxLayout;
    midRow->setSpacing(14);

    // Activity chart card
    auto* chartCard = new QFrame;
    chartCard->setObjectName("card");
    auto* chartCardLay = new QVBoxLayout(chartCard);
    chartCardLay->setContentsMargins(14,12,14,12);
    auto* chartTitle = new QLabel("📊  Aktivnost po mesecima");
    chartTitle->setStyleSheet("font-weight:600; font-size:13px; color:#f0f6fc;");
    chartCardLay->addWidget(chartTitle);
    m_chartView = new QChartView;
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setMinimumHeight(180);
    chartCardLay->addWidget(m_chartView);
    midRow->addWidget(chartCard, 3);

    // Notes card
    if (!m_client.notes.isEmpty()) {
        auto* notesCard = new QFrame;
        notesCard->setObjectName("card");
        auto* nLay = new QVBoxLayout(notesCard);
        nLay->setContentsMargins(14,12,14,12);
        auto* nTitle = new QLabel("📝  Napomene");
        nTitle->setStyleSheet("font-weight:600; font-size:13px; color:#f0f6fc;");
        auto* nText = new QLabel(m_client.notes);
        nText->setWordWrap(true);
        nText->setStyleSheet("color:#8b949e; font-size:12px; line-height:1.5;");
        nLay->addWidget(nTitle);
        nLay->addWidget(nText);
        nLay->addStretch();
        midRow->addWidget(notesCard, 1);
    }

    cLay->addLayout(midRow);

    // ── Work history table ──
    auto* tableTitle = new QLabel("📋  Historija poslova");
    tableTitle->setStyleSheet("font-size:14px; font-weight:700; color:#f0f6fc;");
    cLay->addWidget(tableTitle);

    m_table = new QTableWidget(0, 6);
    m_table->setHorizontalHeaderLabels({"Datum","Vrsta posla","Opis","Sati","Ukupno","Status"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->setColumnWidth(0, 90);
    m_table->setColumnWidth(1, 160);
    m_table->setColumnWidth(3, 60);
    m_table->setColumnWidth(4, 90);
    m_table->setColumnWidth(5, 100);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);
    m_table->setSortingEnabled(true);
    m_table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_table->setMaximumHeight(320);
    m_table->setStyleSheet(R"(
        QTableWidget { color: #f0f6fc; background: #0d1117;
                       alternate-background-color: #161b22; }
        QTableWidget::item { color: #f0f6fc; padding: 6px 10px; }
        QTableWidget::item:selected { background: #1f6feb44; color: #f0f6fc; }
        QScrollBar:vertical { background:#0d1117; width:8px; }
        QScrollBar::handle:vertical { background:#30363d; border-radius:4px; min-height:30px; }
        QScrollBar::handle:vertical:hover { background:#484f58; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }
    )");
    cLay->addWidget(m_table);

    // Close button
    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto* btnClose = new QPushButton("Zatvori");
    btnClose->setFixedWidth(100);
    btnRow->addWidget(btnClose);
    cLay->addLayout(btnRow);

    connect(btnClose,      &QPushButton::clicked, this, &QDialog::accept);
    connect(btnInvoiceAll, &QPushButton::clicked, this, [this]{
        // Export all entries for this client as a combined report
        // We reuse the annual report but filtered - just show a message for now
        QMessageBox::information(this, "Info",
            QString("Izvještaj za mušteriju '%1':\n"
                    "Ukupno poslova: %2\n"
                    "Ukupno fakturisano: %3 €")
                .arg(m_client.name)
                .arg(m_entries.size())
                .arg([this]{ double t=0; for(auto& e:m_entries) t+=e.totalPrice; return QString::number(t,'f',2); }()));
    });
}

// ─── Load data ────────────────────────────────────────────────────────────────

void ClientProfileDialog::loadData()
{
    // Get all work entries for this client (all years)
    m_entries = Database::instance().getWorkEntries(0, 0, m_client.id);

    double total   = 0, paid = 0, unpaid = 0;
    for (auto& e : m_entries) {
        total += e.totalPrice;
        if (e.isPaid) paid   += e.totalPrice;
        else          unpaid += e.totalPrice;
    }

    // Update stat cards
    if (m_lblTotalEarned) m_lblTotalEarned->setText(QString("%1 €").arg(total,  0,'f',2));
    if (m_lblTotalPaid)   m_lblTotalPaid->setText(  QString("%1 €").arg(paid,   0,'f',2));
    if (m_lblUnpaid)      m_lblUnpaid->setText(     QString("%1 €").arg(unpaid, 0,'f',2));
    if (m_lblJobCount)    m_lblJobCount->setText(   QString::number(m_entries.size()));

    // Fill table
    m_table->setSortingEnabled(false);
    m_table->setRowCount(0);
    for (auto& e : m_entries) {
        int row = m_table->rowCount();
        m_table->insertRow(row);

        auto si = [&](int col, const QString& txt,
                      Qt::Alignment a = Qt::AlignVCenter|Qt::AlignLeft) {
            auto* item = new QTableWidgetItem(txt);
            item->setTextAlignment(a);
            item->setForeground(QColor("#f0f6fc"));
            m_table->setItem(row, col, item);
        };

        si(0, e.date.toString("dd.MM.yyyy"));
        si(1, e.workType);
        si(2, e.description);
        si(3, QString::number(e.hours,'f',1), Qt::AlignCenter);

        auto* totalItem = new QTableWidgetItem(
            QString("%1 €").arg(e.totalPrice,0,'f',2));
        totalItem->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        totalItem->setForeground(QColor(Style::GREEN));
        QFont f; f.setBold(true); totalItem->setFont(f);
        m_table->setItem(row, 4, totalItem);

        auto* statusItem = new QTableWidgetItem(
            e.isPaid ? "✅  Plaćeno" : "⏳  Čeka");
        statusItem->setTextAlignment(Qt::AlignCenter);
        statusItem->setForeground(e.isPaid ? QColor(Style::GREEN) : QColor(Style::YELLOW));
        m_table->setItem(row, 5, statusItem);
    }
    m_table->setSortingEnabled(true);

    buildActivityChart();
}

void ClientProfileDialog::buildActivityChart()
{
    // Build monthly earnings map for current year
    int year = QDate::currentDate().year();
    QMap<int,double> monthly;
    for (int m = 1; m <= 12; m++) monthly[m] = 0.0;
    for (auto& e : m_entries) {
        if (e.date.year() == year)
            monthly[e.date.month()] += e.totalPrice;
    }

    auto* series = new QBarSeries;
    auto* set = new QBarSet("Zarada €");
    set->setColor(QColor("#1f6feb"));
    set->setBorderColor(QColor("#58a6ff"));
    for (int m = 1; m <= 12; m++) *set << monthly[m];
    series->append(set);
    series->setLabelsVisible(false);

    auto* chart = new QChart;
    chart->addSeries(series);
    chart->setTitle(QString("Zarada %1").arg(year));
    chart->setTitleFont([] { QFont f("Segoe UI",10,QFont::Bold); return f; }());
    chart->setTitleBrush(QColor("#8b949e"));
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setBackgroundBrush(QColor("#161b22"));
    chart->setBackgroundPen(Qt::NoPen);
    chart->setPlotAreaBackgroundBrush(QColor("#0d1117"));
    chart->setPlotAreaBackgroundVisible(true);
    chart->legend()->setVisible(false);

    auto* axisX = new QBarCategoryAxis;
    QStringList cats;
    for (int m = 1; m <= 12; m++) cats << MONTH_SHORT_P[m];
    axisX->append(cats);
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

    m_chartView->setChart(chart);
    m_chartView->setBackgroundBrush(QColor("#161b22"));
    m_chartView->setFrameShape(QFrame::NoFrame);
}
