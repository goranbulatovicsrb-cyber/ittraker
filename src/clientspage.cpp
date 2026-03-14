#include "clientspage.h"
#include "clientdialog.h"
#include "clientprofiledialog.h"
#include "csvexporter.h"
#include "style.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QTableWidgetItem>

ClientsPage::ClientsPage(QWidget* parent) : QWidget(parent)
{
    buildUi();
    refresh();
}

void ClientsPage::buildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(16);

    // Header
    auto* hdrRow = new QHBoxLayout;
    auto* title = new QLabel("👤  Mušterije");
    title->setObjectName("titleLabel");
    hdrRow->addWidget(title);
    hdrRow->addStretch();
    m_lblCount = new QLabel;
    m_lblCount->setStyleSheet("color:#8b949e;");
    hdrRow->addWidget(m_lblCount);
    root->addLayout(hdrRow);

    // Toolbar
    auto* toolRow = new QHBoxLayout;
    toolRow->setSpacing(10);

    m_search = new QLineEdit;
    m_search->setPlaceholderText("🔍  Pretraži mušterije...");
    m_search->setMaximumWidth(300);
    toolRow->addWidget(m_search);
    toolRow->addStretch();

    auto* btnAdd = new QPushButton("➕  Nova mušterija"); btnAdd->setObjectName("btnPrimary");
    m_btnEdit = new QPushButton("✏️  Izmeni");           m_btnEdit->setEnabled(false);
    m_btnDel  = new QPushButton("🗑  Obriši");           m_btnDel->setObjectName("btnDanger"); m_btnDel->setEnabled(false);

    toolRow->addWidget(btnAdd);
    toolRow->addWidget(m_btnEdit);
    toolRow->addWidget(m_btnDel);
    auto* btnExportCsv = new QPushButton("📊  Excel/CSV");
    toolRow->addWidget(btnExportCsv);
    root->addLayout(toolRow);

    // Table
    m_table = new QTableWidget(0, 5);
    m_table->setHorizontalHeaderLabels({"Ime", "Email", "Telefon", "Adresa", "Napomene"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
    m_table->setColumnWidth(0, 200);
    m_table->setColumnWidth(1, 180);
    m_table->setColumnWidth(2, 130);
    m_table->setColumnWidth(3, 180);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);
    m_table->setSortingEnabled(true);
    root->addWidget(m_table);

    // Pagination
    m_pager = new PaginationWidget(this);
    root->addWidget(m_pager);

    connect(btnAdd,       &QPushButton::clicked,        this, &ClientsPage::onAdd);
    connect(m_btnEdit,    &QPushButton::clicked,        this, &ClientsPage::onEdit);
    connect(m_btnDel,     &QPushButton::clicked,        this, &ClientsPage::onDelete);
    connect(btnExportCsv, &QPushButton::clicked,        this, &ClientsPage::onExportCsv);
    connect(m_search,     &QLineEdit::textChanged,      this, &ClientsPage::onFilter);
    connect(m_table,  &QTableWidget::itemSelectionChanged, this, &ClientsPage::onSelectionChanged);
    connect(m_table,  &QTableWidget::cellDoubleClicked,    this, [this]{
        int id = selectedClientId();
        if (id < 0) return;
        Client c = Database::instance().getClientById(id);
        ClientProfileDialog dlg(c, this);
        dlg.exec();
    });
    connect(m_pager,  &PaginationWidget::pageChanged,      this, &ClientsPage::onPageChanged);
}

void ClientsPage::refresh()
{
    m_allClients = Database::instance().getAllClients();
    onFilter(m_search->text());
}

void ClientsPage::onFilter(const QString& text)
{
    if (text.isEmpty()) {
        m_filtered = m_allClients;
    } else {
        m_filtered.clear();
        for (auto& c : m_allClients)
            if (c.name.contains(text, Qt::CaseInsensitive) ||
                c.email.contains(text, Qt::CaseInsensitive) ||
                c.phone.contains(text, Qt::CaseInsensitive))
                m_filtered.append(c);
    }
    m_lblCount->setText(QString("%1 mušterija").arg(m_filtered.size()));
    m_pager->setTotal(m_filtered.size());
    m_pager->reset();
    populateTable();
}

void ClientsPage::onPageChanged()
{
    populateTable();
}

void ClientsPage::populateTable()
{
    m_table->setSortingEnabled(false);
    m_table->setRowCount(0);

    int offset   = m_pager->offset();
    int pageSize = m_pager->pageSize();
    int end      = qMin(offset + pageSize, m_filtered.size());

    for (int i = offset; i < end; i++) {
        const Client& c = m_filtered[i];
        int row = m_table->rowCount();
        m_table->insertRow(row);

        auto setItem = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setData(Qt::UserRole, c.id);
            m_table->setItem(row, col, item);
        };
        setItem(0, c.name);
        setItem(1, c.email);
        setItem(2, c.phone);
        setItem(3, c.address);
        setItem(4, c.notes);
    }

    m_table->setSortingEnabled(true);
    onSelectionChanged();
}

void ClientsPage::onSelectionChanged()
{
    bool sel = (m_table->currentRow() >= 0);
    m_btnEdit->setEnabled(sel);
    m_btnDel->setEnabled(sel);
}

int ClientsPage::selectedClientId() const
{
    auto* item = m_table->item(m_table->currentRow(), 0);
    return item ? item->data(Qt::UserRole).toInt() : -1;
}

void ClientsPage::onExportCsv()
{
    CsvExporter::exportClients(m_filtered, this);
}

void ClientsPage::onAdd()
{
    ClientDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        Client c = dlg.result();
        Database::instance().addClient(c);
        refresh();
    }
}

void ClientsPage::onEdit()
{
    int id = selectedClientId();
    if (id < 0) return;
    Client c = Database::instance().getClientById(id);
    ClientDialog dlg(this, c);
    if (dlg.exec() == QDialog::Accepted) {
        Database::instance().updateClient(dlg.result());
        refresh();
    }
}

void ClientsPage::onDelete()
{
    int id = selectedClientId();
    if (id < 0) return;
    Client c = Database::instance().getClientById(id);
    auto ans = QMessageBox::question(this, "Potvrda brisanja",
        QString("Obrisati mušteriju \"%1\"?\nSvi poslovi bit će obrisani.").arg(c.name),
        QMessageBox::Yes | QMessageBox::No);
    if (ans == QMessageBox::Yes) {
        Database::instance().deleteClient(id);
        refresh();
    }
}
