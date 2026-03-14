#include "usermanagerdialog.h"
#include "session.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>

UserManagerDialog::UserManagerDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("👥  Upravljanje korisnicima");
    setMinimumSize(420, 380);
    setModal(true);
    buildUi();
    loadUsers();
}

void UserManagerDialog::buildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(20,16,20,16);
    root->setSpacing(12);

    auto* title = new QLabel("👥  Korisnici programa");
    title->setStyleSheet("font-size:16px; font-weight:700; color:#f0f6fc;");
    root->addWidget(title);

    auto* sub = new QLabel("Svaki korisnik ima svoju evidenciju poslova i mušterija.");
    sub->setStyleSheet("color:#8b949e; font-size:12px;");
    sub->setWordWrap(true);
    root->addWidget(sub);

    m_list = new QListWidget;
    m_list->setStyleSheet(R"(
        QListWidget { background:#0d1117; border:1px solid #30363d; border-radius:6px; }
        QListWidget::item { padding:12px 14px; border-bottom:1px solid #21262d; color:#f0f6fc; }
        QListWidget::item:selected { background:#1f6feb33; color:#58a6ff; }
    )");
    root->addWidget(m_list);

    m_lblInfo = new QLabel;
    m_lblInfo->setStyleSheet("color:#8b949e; font-size:11px;");
    root->addWidget(m_lblInfo);

    auto* btnRow = new QHBoxLayout;
    auto* btnAdd = new QPushButton("➕  Novi korisnik");
    btnAdd->setObjectName("btnPrimary");
    m_btnPass = new QPushButton("🔑  Promijeni lozinku");
    m_btnPass->setEnabled(false);
    m_btnDel = new QPushButton("🗑  Obriši");
    m_btnDel->setObjectName("btnDanger");
    m_btnDel->setEnabled(false);
    auto* btnClose = new QPushButton("Zatvori");

    btnRow->addWidget(btnAdd);
    btnRow->addWidget(m_btnPass);
    btnRow->addWidget(m_btnDel);
    btnRow->addStretch();
    btnRow->addWidget(btnClose);
    root->addLayout(btnRow);

    connect(btnAdd,   &QPushButton::clicked, this, &UserManagerDialog::onAdd);
    connect(m_btnDel, &QPushButton::clicked, this, &UserManagerDialog::onDelete);
    connect(m_btnPass,&QPushButton::clicked, this, &UserManagerDialog::onChangePassword);
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_list, &QListWidget::itemSelectionChanged, this, &UserManagerDialog::onSelectionChanged);
}

void UserManagerDialog::loadUsers()
{
    m_list->clear();
    auto users = Database::instance().getAllUsers();
    for (auto& u : users) {
        QString label = u.displayName + "  (@" + u.username + ")";
        if (u.isAdmin) label += "  👑 Admin";
        auto* item = new QListWidgetItem(label);
        item->setData(Qt::UserRole, u.id);
        // Can't delete yourself or admin
        if (u.isAdmin || u.id == Session::instance().userId())
            item->setForeground(QColor("#8b949e"));
        m_list->addItem(item);
    }
    m_lblInfo->setText(QString("Ukupno korisnika: %1").arg(users.size()));
    onSelectionChanged();
}

int UserManagerDialog::selectedUserId() const
{
    auto* item = m_list->currentItem();
    return item ? item->data(Qt::UserRole).toInt() : -1;
}

void UserManagerDialog::onSelectionChanged()
{
    int id = selectedUserId();
    bool canModify = (id > 0 && id != Session::instance().userId());
    auto users = Database::instance().getAllUsers();
    bool isAdmin = false;
    for (auto& u : users) if (u.id == id) { isAdmin = u.isAdmin; break; }

    m_btnDel->setEnabled(canModify && !isAdmin);
    m_btnPass->setEnabled(id > 0);
}

void UserManagerDialog::onAdd()
{
    auto* dlg = new QDialog(this);
    dlg->setWindowTitle("Novi korisnik");
    dlg->setModal(true);
    dlg->setMinimumWidth(340);

    auto* lay = new QVBoxLayout(dlg);
    lay->setContentsMargins(20,16,20,16);
    lay->setSpacing(10);
    lay->addWidget([] { auto* l = new QLabel("Novi korisnik"); l->setStyleSheet("font-size:15px; font-weight:700; color:#f0f6fc;"); return l; }());

    auto* form = new QFormLayout;
    auto mkLabel = [](const QString& t) { auto* l = new QLabel(t); l->setStyleSheet("color:#8b949e; font-weight:600;"); return l; };

    auto* edtDisplay = new QLineEdit; edtDisplay->setPlaceholderText("Ime i prezime");
    auto* edtUser    = new QLineEdit; edtUser->setPlaceholderText("korisnicko.ime");
    auto* edtPass    = new QLineEdit; edtPass->setEchoMode(QLineEdit::Password); edtPass->setPlaceholderText("Lozinka");
    auto* edtPass2   = new QLineEdit; edtPass2->setEchoMode(QLineEdit::Password); edtPass2->setPlaceholderText("Ponovi lozinku");
    auto* chkAdmin   = new QCheckBox("Admin (može upravljati korisnicima)");

    form->addRow(mkLabel("Ime *"),          edtDisplay);
    form->addRow(mkLabel("Korisničko *"),   edtUser);
    form->addRow(mkLabel("Lozinka *"),      edtPass);
    form->addRow(mkLabel("Ponovi *"),       edtPass2);
    form->addRow(mkLabel(""),               chkAdmin);
    lay->addLayout(form);

    auto* errLabel = new QLabel; errLabel->setStyleSheet("color:#f85149;"); errLabel->hide();
    lay->addWidget(errLabel);

    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto* btnCancel = new QPushButton("Otkaži");
    auto* btnSave   = new QPushButton("✅  Kreiraj");
    btnSave->setObjectName("btnPrimary");
    btnRow->addWidget(btnCancel); btnRow->addWidget(btnSave);
    lay->addLayout(btnRow);

    connect(btnCancel, &QPushButton::clicked, dlg, &QDialog::reject);
    connect(btnSave,   &QPushButton::clicked, dlg, [=] {
        QString display = edtDisplay->text().trimmed();
        QString user    = edtUser->text().trimmed().toLower();
        QString pass    = edtPass->text();
        QString pass2   = edtPass2->text();

        if (display.isEmpty() || user.isEmpty() || pass.isEmpty()) {
            errLabel->setText("Sva polja su obavezna!"); errLabel->show(); return;
        }
        if (pass != pass2) {
            errLabel->setText("Lozinke se ne poklapaju!"); errLabel->show(); return;
        }
        if (pass.length() < 3) {
            errLabel->setText("Lozinka mora imati bar 3 karaktera!"); errLabel->show(); return;
        }

        AppUser u;
        u.displayName = display;
        u.username    = user;
        u.isAdmin     = chkAdmin->isChecked();

        if (Database::instance().addUser(u, pass) < 0) {
            errLabel->setText("Korisničko ime već postoji!"); errLabel->show(); return;
        }
        dlg->accept();
    });

    if (dlg->exec() == QDialog::Accepted) loadUsers();
    dlg->deleteLater();
}

void UserManagerDialog::onDelete()
{
    int id = selectedUserId();
    if (id <= 0) return;
    auto ans = QMessageBox::question(this, "Potvrda",
        "Obrisati korisnika?\nSvi njegovi poslovi i mušterije bit će trajno obrisani!",
        QMessageBox::Yes | QMessageBox::No);
    if (ans == QMessageBox::Yes) {
        Database::instance().deleteUser(id);
        loadUsers();
    }
}

void UserManagerDialog::onChangePassword()
{
    int id = selectedUserId();
    if (id <= 0) return;

    bool ok;
    QString newPass = QInputDialog::getText(this, "Promjena lozinke",
        "Nova lozinka:", QLineEdit::Password, "", &ok);
    if (!ok || newPass.trimmed().isEmpty()) return;
    if (newPass.length() < 3) {
        QMessageBox::warning(this, "Greška", "Lozinka mora imati bar 3 karaktera!");
        return;
    }
    Database::instance().updateUserPassword(id, newPass);
    QMessageBox::information(this, "✅ Uspjeh", "Lozinka je promijenjena.");
}
