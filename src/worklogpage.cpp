#include "worklogpage.h"
#include "jobdialog.h"
#include "style.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QDate>
#include <QFrame>
#include <QFont>

static const QStringList MONTH_NAMES = {
    "Svi meseci","Januar","Februar","Mart","April","Maj","Juni",
    "Juli","Avgust","Septembar","Oktobar","Novembar","Decembar"
};
static const QStringList WORK_TYPES_FILTER = {
    "Sve vrste","Popravka hardvera","Instalacija OS/softvera","Pravljenje sajta",
    "Snimanje / Fotografija","Mreža / Ruter","Prenos podataka","Konsultacije","Ostalo"
};

// ─── Constructor ──────────────────────────────────────────────────────────────

WorkLogPage::WorkLogPage(QWidget* parent) : QWidget(parent)
{
    buildUi();
    refresh();
}

// ─── Build UI ─────────────────────────────────────────────────────────────────

void WorkLogPage::buildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(14);

    // ── Header ──
    auto* hdrRow = new QHBoxLayout;
    auto* title = new QLabel("📋  Evidencija poslova");
    title->setObjectName("titleLabel");
    hdrRow->addWidget(title);
    hdrRow->addStretch();
    root->addLayout(hdrRow);

    // ── Filters ──
    auto* filterFrame = new QFrame;
    filterFrame->setObjectName("card");
    auto* filterLay = new QHBoxLayout(filterFrame);
    filterLay->setContentsMargins(12,10,12,10);
    filterLay->setSpacing(10);

    auto mkLabel = [](const QString& t) {
        auto* l = new QLabel(t);
        l->setStyleSheet("color:#8b949e; font-weight:600; font-size:11px;");
        return l;
    };

    // Year
    m_cmbYear = new QComboBox;
    m_cmbYear->setMinimumWidth(80);
    int currentYear = QDate::currentDate().year();
    m_cmbYear->addItem("Sve godine", 0);
    for (int y = currentYear; y >= currentYear - 5; y--)
        m_cmbYear->addItem(QString::number(y), y);
    m_cmbYear->setCurrentIndex(1); // default to current year
    filterLay->addWidget(mkLabel("Godina:"));
    filterLay->addWidget(m_cmbYear);

    // Month
    m_cmbMonth = new QComboBox;
    m_cmbMonth->setMinimumWidth(120);
    for (int i = 0; i < MONTH_NAMES.size(); i++)
        m_cmbMonth->addItem(MONTH_NAMES[i], i);
    m_cmbMonth->setCurrentIndex(QDate::currentDate().month()); // default to current month
    filterLay->addWidget(mkLabel("Mesec:"));
    filterLay->addWidget(m_cmbMonth);

    // Client
    m_cmbClient = new QComboBox;
    m_cmbClient->setMinimumWidth(150);
    filterLay->addWidget(mkLabel("Mušterija:"));
    filterLay->addWidget(m_cmbClient);

    // Type
    m_cmbType = new QComboBox;
    m_cmbType->setMinimumWidth(160);
    for (auto& t : WORK_TYPES_FILTER) m_cmbType->addItem(t);
    filterLay->addWidget(mkLabel("Vrsta:"));
    filterLay->addWidget(m_cmbType);

    filterLay->addStretch();

    auto* btnFilter = new QPushButton("🔍  Filtriraj");
    btnFilter->setObjectName("btnPrimary");
    filterLay->addWidget(btnFilter);

    root->addWidget(filterFrame);

    // ── Toolbar ──
    auto* toolRow = new QHBoxLayout;
    auto* btnAdd = new QPushButton("➕  Dodaj posao");
    btnAdd->setObjectName("btnPrimary");
    m_btnEdit = new QPushButton("✏️  Izmeni");  m_btnEdit->setEnabled(false);
    m_btnDel  = new QPushButton("🗑  Obriši");  m_btnDel->setObjectName("btnDanger"); m_btnDel->setEnabled(false);
    m_btnPaid = new QPushButton("💰  Označi plaćeno"); m_btnPaid->setObjectName("btnSuccess"); m_btnPaid->setEnabled(false);

    toolRow->addWidget(btnAdd);
    toolRow->addWidget(m_btnEdit);
    toolRow->addWidget(m_btnDel);
    toolRow->addWidget(m_btnPaid);
    toolRow->addStretch();

    m_lblCount = new QLabel;
    m_lblCount->setStyleSheet("color:#8b949e;");
    m_lblTotal = new QLabel;
    m_lblTotal->setStyleSheet("font-weight:700; font-size:15px; color:#3fb950;");
    toolRow->addWidget(m_lblCount);
    toolRow->addSpacing(16);
    toolRow->addWidget(new QLabel("Ukupno:"));
    toolRow->addWidget(m_lblTotal);

    root->addLayout(toolRow);

    // ── Table ──
    m_table = new QTableWidget(0, 8);
    m_table->setHorizontalHeaderLabels({"Datum","Mušterija","Vrsta posla","Opis","Sati","Cijena/h","Ukupno","Status"});
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_table->setColumnWidth(0, 90);
    m_table->setColumnWidth(1, 160);
    m_table->setColumnWidth(2, 160);
    m_table->setColumnWidth(3, 200);
    m_table->setColumnWidth(4, 60);
    m_table->setColumnWidth(5, 80);
    m_table->setColumnWidth(6, 90);
    m_table->setColumnWidth(7, 100);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);
    m_table->setSortingEnabled(true);
    root->addWidget(m_table);

    // Connections
    connect(btnAdd,     &QPushButton::clicked, this, &WorkLogPage::onAdd);
    connect(m_btnEdit,  &QPushButton::clicked, this, &WorkLogPage::onEdit);
    connect(m_btnDel,   &QPushButton::clicked, this, &WorkLogPage::onDelete);
    connect(m_btnPaid,  &QPushButton::clicked, this, &WorkLogPage::onTogglePaid);
    connect(btnFilter,  &QPushButton::clicked, this, &WorkLogPage::applyFilters);
    connect(m_cmbYear,  qOverload<int>(&QComboBox::currentIndexChanged), this, &WorkLogPage::applyFilters);
    connect(m_cmbMonth, qOverload<int>(&QComboBox::currentIndexChanged), this, &WorkLogPage::applyFilters);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &WorkLogPage::onSelectionChanged);
    connect(m_table, &QTableWidget::cellDoubleClicked,    this, [this]{ onEdit(); });
}

// ─── Data ─────────────────────────────────────────────────────────────────────

void WorkLogPage::refresh()
{
    // Reload client combo
    auto savedClientId = m_cmbClient->currentData().toInt();
    m_cmbClient->blockSignals(true);
    m_cmbClient->clear();
    m_cmbClient->addItem("Sve mušterije", 0);
    for (auto& c : Database::instance().getAllClients())
        m_cmbClient->addItem(c.name, c.id);
    // restore selection
    int idx = m_cmbClient->findData(savedClientId);
    if (idx >= 0) m_cmbClient->setCurrentIndex(idx);
    m_cmbClient->blockSignals(false);

    applyFilters();
}

void WorkLogPage::applyFilters()
{
    int year     = m_cmbYear->currentData().toInt();
    int month    = m_cmbMonth->currentData().toInt();
    int clientId = m_cmbClient->currentData().toInt();
    QString type = (m_cmbType->currentIndex() == 0) ? "" : m_cmbType->currentText();

    auto entries = Database::instance().getWorkEntries(year, month, clientId, type);
    populateTable(entries);
}

void WorkLogPage::populateTable(const QList<WorkEntry>& entries)
{
    m_table->setSortingEnabled(false);
    m_table->setRowCount(0);

    double total = 0.0;

    for (auto& w : entries) {
        int row = m_table->rowCount();
        m_table->insertRow(row);

        auto setItem = [&](int col, const QString& text, Qt::Alignment align = Qt::AlignVCenter | Qt::AlignLeft) {
            auto* item = new QTableWidgetItem(text);
            item->setData(Qt::UserRole, w.id);
            item->setTextAlignment(align);
            m_table->setItem(row, col, item);
            return item;
        };

        setItem(0, w.date.toString("dd.MM.yyyy"));
        setItem(1, w.clientName);
        setItem(2, w.workType);
        setItem(3, w.description);
        setItem(4, QString::number(w.hours, 'f', 1), Qt::AlignCenter);
        setItem(5, w.pricePerHour > 0 ? QString("%1 €").arg(w.pricePerHour, 0, 'f', 0) : "—", Qt::AlignRight | Qt::AlignVCenter);
        auto* totalItem = setItem(6, QString("%1 €").arg(w.totalPrice, 0, 'f', 2), Qt::AlignRight | Qt::AlignVCenter);
        totalItem->setForeground(QColor(Style::GREEN));
        totalItem->setFont([] { QFont f; f.setBold(true); return f; }());

        auto* statusItem = setItem(7, w.isPaid ? "✅  Plaćeno" : "⏳  Čeka", Qt::AlignCenter);
        statusItem->setForeground(w.isPaid ? QColor(Style::GREEN) : QColor(Style::YELLOW));

        // Dim unpaid rows slightly
        if (!w.isPaid) {
            for (int c = 0; c < 8; c++) {
                if (auto* it = m_table->item(row, c))
                    if (c != 6 && c != 7) it->setForeground(QColor("#c0c7d0"));
            }
        }

        total += w.totalPrice;
    }

    m_table->setSortingEnabled(true);
    m_lblTotal->setText(QString("%1 €").arg(total, 0, 'f', 2));
    m_lblCount->setText(QString("%1 poslova").arg(entries.size()));
    onSelectionChanged();
}

// ─── Slots ────────────────────────────────────────────────────────────────────

void WorkLogPage::onSelectionChanged()
{
    bool sel = (m_table->currentRow() >= 0);
    m_btnEdit->setEnabled(sel);
    m_btnDel->setEnabled(sel);
    m_btnPaid->setEnabled(sel);
}

int WorkLogPage::selectedEntryId() const
{
    auto* item = m_table->item(m_table->currentRow(), 0);
    return item ? item->data(Qt::UserRole).toInt() : -1;
}

WorkEntry WorkLogPage::selectedEntry() const
{
    int id = selectedEntryId();
    if (id < 0) return {};
    auto entries = Database::instance().getWorkEntries();
    for (auto& e : entries) if (e.id == id) return e;
    return {};
}

void WorkLogPage::onAdd()
{
    if (Database::instance().totalClients() == 0) {
        QMessageBox::information(this, "Info",
            "Prvo dodajte barem jednu mušteriju u sekciji 'Mušterije'!");
        return;
    }
    JobDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        WorkEntry w = dlg.result();
        Database::instance().addWork(w);
        refresh();
        emit dataChanged();
    }
}

void WorkLogPage::onEdit()
{
    WorkEntry e = selectedEntry();
    if (e.id < 0) return;
    JobDialog dlg(this, e);
    if (dlg.exec() == QDialog::Accepted) {
        Database::instance().updateWork(dlg.result());
        refresh();
        emit dataChanged();
    }
}

void WorkLogPage::onDelete()
{
    int id = selectedEntryId();
    if (id < 0) return;
    auto ans = QMessageBox::question(this, "Potvrda", "Obrisati ovaj posao?",
                                     QMessageBox::Yes | QMessageBox::No);
    if (ans == QMessageBox::Yes) {
        Database::instance().deleteWork(id);
        refresh();
        emit dataChanged();
    }
}

void WorkLogPage::onTogglePaid()
{
    WorkEntry e = selectedEntry();
    if (e.id < 0) return;
    Database::instance().setWorkPaid(e.id, !e.isPaid);
    refresh();
    emit dataChanged();
}
