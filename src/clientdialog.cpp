#include "clientdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

ClientDialog::ClientDialog(QWidget* parent, const Client& c)
    : QDialog(parent), m_client(c)
{
    m_editMode = (c.id != -1);
    setWindowTitle(m_editMode ? "✏️  Izmeni mušteriju" : "👤  Nova mušterija");
    setModal(true);
    setMinimumWidth(400);
    buildUi();
    if (m_editMode) {
        m_edtName->setText(c.name);
        m_edtEmail->setText(c.email);
        m_edtPhone->setText(c.phone);
        m_edtAddress->setText(c.address);
        m_txtNotes->setPlainText(c.notes);
    }
}

void ClientDialog::buildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24,20,24,20);
    root->setSpacing(14);

    auto* form = new QFormLayout;
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    auto mkLabel = [](const QString& t) {
        auto* l = new QLabel(t);
        l->setStyleSheet("color:#8b949e; font-weight:600;");
        return l;
    };

    m_edtName    = new QLineEdit; m_edtName->setPlaceholderText("Ime i prezime / Naziv firme");
    m_edtEmail   = new QLineEdit; m_edtEmail->setPlaceholderText("email@primjer.com");
    m_edtPhone   = new QLineEdit; m_edtPhone->setPlaceholderText("+387 61 ...");
    m_edtAddress = new QLineEdit; m_edtAddress->setPlaceholderText("Adresa (opcionalno)");
    m_txtNotes   = new QTextEdit; m_txtNotes->setPlaceholderText("Napomene..."); m_txtNotes->setMaximumHeight(80);

    form->addRow(mkLabel("Ime *"),    m_edtName);
    form->addRow(mkLabel("Email"),    m_edtEmail);
    form->addRow(mkLabel("Telefon"),  m_edtPhone);
    form->addRow(mkLabel("Adresa"),   m_edtAddress);
    form->addRow(mkLabel("Napomene"), m_txtNotes);

    root->addLayout(form);

    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto* btnCancel = new QPushButton("Otkaži"); btnCancel->setFixedWidth(90);
    auto* btnSave   = new QPushButton(m_editMode ? "💾  Sačuvaj" : "✅  Dodaj");
    btnSave->setObjectName("btnPrimary"); btnSave->setFixedWidth(120);
    btnRow->addWidget(btnCancel);
    btnRow->addWidget(btnSave);
    root->addLayout(btnRow);

    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(btnSave,   &QPushButton::clicked, this, &ClientDialog::onAccept);
}

void ClientDialog::onAccept()
{
    QString name = m_edtName->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Greška", "Ime mušterije je obavezno!");
        return;
    }
    m_client.name    = name;
    m_client.email   = m_edtEmail->text().trimmed();
    m_client.phone   = m_edtPhone->text().trimmed();
    m_client.address = m_edtAddress->text().trimmed();
    m_client.notes   = m_txtNotes->toPlainText().trimmed();
    accept();
}
