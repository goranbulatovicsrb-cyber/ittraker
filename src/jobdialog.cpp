#include "jobdialog.h"
#include "style.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>
#include <QFrame>

static const QStringList WORK_TYPES = {
    "Popravka hardvera", "Instalacija OS/softvera", "Pravljenje sajta",
    "Snimanje / Fotografija", "Mreža / Ruter", "Prenos podataka",
    "Konsultacije", "Ostalo"
};

// ─── Constructor ──────────────────────────────────────────────────────────────

JobDialog::JobDialog(QWidget* parent, const WorkEntry& entry)
    : QDialog(parent), m_entry(entry)
{
    m_editMode = (entry.id != -1);
    setWindowTitle(m_editMode ? "✏️  Izmeni posao" : "➕  Dodaj posao");
    setModal(true);
    setMinimumWidth(460);
    buildUi();
    loadClients();
    if (m_editMode) populate(entry);
}

// ─── Build UI ─────────────────────────────────────────────────────────────────

void JobDialog::buildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(16);

    // ── Form ──
    auto* form = new QFormLayout;
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    auto mkLabel = [](const QString& t) {
        auto* l = new QLabel(t);
        l->setStyleSheet("color:#8b949e; font-weight:600;");
        return l;
    };

    // Client
    m_cmbClient = new QComboBox;
    m_cmbClient->setMinimumWidth(280);
    form->addRow(mkLabel("Mušterija *"), m_cmbClient);

    // Work type
    m_cmbType = new QComboBox;
    for (auto& t : WORK_TYPES) m_cmbType->addItem(t);
    form->addRow(mkLabel("Vrsta posla *"), m_cmbType);

    // Date
    m_dateEdit = new QDateEdit(QDate::currentDate());
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDisplayFormat("dd.MM.yyyy");
    form->addRow(mkLabel("Datum *"), m_dateEdit);

    // Separator
    auto* sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background:#30363d;");
    sep->setFixedHeight(1);
    form->addRow(sep);

    // Hours
    m_spnHours = new QDoubleSpinBox;
    m_spnHours->setRange(0.25, 999.0);
    m_spnHours->setSingleStep(0.5);
    m_spnHours->setValue(1.0);
    m_spnHours->setSuffix(" h");
    form->addRow(mkLabel("Broj sati"), m_spnHours);

    // Price per hour
    m_spnPriceHour = new QDoubleSpinBox;
    m_spnPriceHour->setRange(0, 99999.0);
    m_spnPriceHour->setSingleStep(5.0);
    m_spnPriceHour->setValue(0.0);
    m_spnPriceHour->setSuffix(" €/h");
    form->addRow(mkLabel("Cijena / sat"), m_spnPriceHour);

    // Total price
    m_spnTotal = new QDoubleSpinBox;
    m_spnTotal->setRange(0, 999999.0);
    m_spnTotal->setSingleStep(5.0);
    m_spnTotal->setSuffix(" €");
    m_spnTotal->setStyleSheet("font-weight:700; color:#3fb950;");
    form->addRow(mkLabel("UKUPNO *"), m_spnTotal);

    // Paid?
    m_chkPaid = new QCheckBox("Plaćeno");
    form->addRow(mkLabel("Status"), m_chkPaid);

    // Description
    m_txtDesc = new QTextEdit;
    m_txtDesc->setPlaceholderText("Opis posla (opcionalno)...");
    m_txtDesc->setMaximumHeight(90);
    form->addRow(mkLabel("Opis"), m_txtDesc);

    root->addLayout(form);

    // ── Buttons ──
    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto* btnCancel = new QPushButton("Otkaži");
    btnCancel->setFixedWidth(100);
    auto* btnSave = new QPushButton(m_editMode ? "💾  Sačuvaj" : "✅  Dodaj");
    btnSave->setObjectName("btnPrimary");
    btnSave->setFixedWidth(120);
    btnRow->addWidget(btnCancel);
    btnRow->addWidget(btnSave);
    root->addLayout(btnRow);

    // ── Connections ──
    connect(m_spnHours,     qOverload<double>(&QDoubleSpinBox::valueChanged), this, &JobDialog::recalcTotal);
    connect(m_spnPriceHour, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &JobDialog::recalcTotal);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(btnSave,   &QPushButton::clicked, this, &JobDialog::onAccept);
}

void JobDialog::loadClients()
{
    m_cmbClient->clear();
    for (auto& c : Database::instance().getAllClients())
        m_cmbClient->addItem(c.name, c.id);
}

void JobDialog::populate(const WorkEntry& e)
{
    // Client
    int idx = m_cmbClient->findData(e.clientId);
    if (idx >= 0) m_cmbClient->setCurrentIndex(idx);

    // Type
    idx = m_cmbType->findText(e.workType);
    if (idx >= 0) m_cmbType->setCurrentIndex(idx);

    m_dateEdit->setDate(e.date);
    m_spnHours->setValue(e.hours);
    m_spnPriceHour->setValue(e.pricePerHour);
    m_spnTotal->setValue(e.totalPrice);
    m_chkPaid->setChecked(e.isPaid);
    m_txtDesc->setPlainText(e.description);
}

// ─── Slots ────────────────────────────────────────────────────────────────────

void JobDialog::recalcTotal()
{
    double total = m_spnHours->value() * m_spnPriceHour->value();
    m_spnTotal->setValue(total);
}

void JobDialog::onAccept()
{
    if (m_cmbClient->count() == 0) {
        QMessageBox::warning(this, "Greška", "Morate dodati bar jednu mušteriju!");
        return;
    }
    if (m_spnTotal->value() <= 0) {
        QMessageBox::warning(this, "Greška", "Ukupna cijena mora biti veća od 0!");
        return;
    }

    m_entry.clientId     = m_cmbClient->currentData().toInt();
    m_entry.clientName   = m_cmbClient->currentText();
    m_entry.workType     = m_cmbType->currentText();
    m_entry.date         = m_dateEdit->date();
    m_entry.hours        = m_spnHours->value();
    m_entry.pricePerHour = m_spnPriceHour->value();
    m_entry.totalPrice   = m_spnTotal->value();
    m_entry.isPaid       = m_chkPaid->isChecked();
    m_entry.description  = m_txtDesc->toPlainText();

    accept();
}
