#include "loginscreen.h"
#include "session.h"
#include "style.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QApplication>

LoginScreen::LoginScreen(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("IT Tracker — Prijava");
    setModal(true);
    setFixedSize(400, 480);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    buildUi();
    loadUsers();
}

void LoginScreen::buildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    // ── Header ──
    auto* header = new QFrame;
    header->setFixedHeight(110);
    header->setStyleSheet("background: #0d1117; border-bottom: 1px solid #30363d;");
    auto* hLay = new QVBoxLayout(header);
    hLay->setAlignment(Qt::AlignCenter);

    auto* icon = new QLabel("💻");
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet("font-size:36px; background:transparent;");
    auto* title = new QLabel("IT Tracker");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size:22px; font-weight:700; color:#f0f6fc; background:transparent;");
    auto* sub = new QLabel("Sistem za evidenciju zarade");
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet("font-size:12px; color:#8b949e; background:transparent;");

    hLay->addWidget(icon);
    hLay->addWidget(title);
    hLay->addWidget(sub);
    root->addWidget(header);

    // ── Stacked pages ──
    m_stack = new QStackedWidget;
    m_stack->setStyleSheet("background:#161b22;");
    root->addWidget(m_stack, 1);

    // ── Page 0: User list ──
    auto* page0 = new QWidget;
    auto* p0Lay = new QVBoxLayout(page0);
    p0Lay->setContentsMargins(30,24,30,24);
    p0Lay->setSpacing(12);

    m_lblSelect = new QLabel("Odaberite korisnika:");
    m_lblSelect->setStyleSheet("font-size:13px; font-weight:600; color:#8b949e;");
    p0Lay->addWidget(m_lblSelect);

    m_userList = new QListWidget;
    m_userList->setStyleSheet(R"(
        QListWidget {
            background:#0d1117; border:1px solid #30363d; border-radius:8px;
            outline:none;
        }
        QListWidget::item {
            padding:14px 16px; border-bottom:1px solid #21262d;
            font-size:14px; color:#f0f6fc; border-radius:0;
        }
        QListWidget::item:selected {
            background:#1f6feb22; color:#58a6ff;
            border-left: 3px solid #58a6ff;
        }
        QListWidget::item:hover { background:#21262d; }
    )");
    m_userList->setCursor(Qt::PointingHandCursor);
    p0Lay->addWidget(m_userList);

    auto* btnExit = new QPushButton("Izlaz");
    btnExit->setStyleSheet("color:#8b949e; background:transparent; border:none; font-size:12px;");
    btnExit->setCursor(Qt::PointingHandCursor);
    p0Lay->addWidget(btnExit, 0, Qt::AlignCenter);

    m_stack->addWidget(page0);

    // ── Page 1: Password ──
    auto* page1 = new QWidget;
    auto* p1Lay = new QVBoxLayout(page1);
    p1Lay->setContentsMargins(30,30,30,30);
    p1Lay->setSpacing(14);
    p1Lay->addStretch();

    m_lblWelcome = new QLabel;
    m_lblWelcome->setAlignment(Qt::AlignCenter);
    m_lblWelcome->setStyleSheet("font-size:18px; font-weight:700; color:#f0f6fc;");
    p1Lay->addWidget(m_lblWelcome);

    auto* lblPass = new QLabel("Lozinka:");
    lblPass->setStyleSheet("color:#8b949e; font-weight:600; font-size:12px;");
    p1Lay->addWidget(lblPass);

    m_edtPass = new QLineEdit;
    m_edtPass->setEchoMode(QLineEdit::Password);
    m_edtPass->setPlaceholderText("Unesite lozinku...");
    m_edtPass->setFixedHeight(42);
    m_edtPass->setStyleSheet(R"(
        QLineEdit {
            background:#0d1117; color:#f0f6fc;
            border:1px solid #30363d; border-radius:8px;
            padding:0 14px; font-size:14px;
        }
        QLineEdit:focus { border-color:#58a6ff; }
    )");
    p1Lay->addWidget(m_edtPass);

    m_lblError = new QLabel;
    m_lblError->setAlignment(Qt::AlignCenter);
    m_lblError->setStyleSheet("color:#f85149; font-size:12px;");
    m_lblError->hide();
    p1Lay->addWidget(m_lblError);

    auto* btnLogin = new QPushButton("🔓  Prijava");
    btnLogin->setFixedHeight(42);
    btnLogin->setObjectName("btnPrimary");
    btnLogin->setStyleSheet(R"(
        QPushButton {
            background:#1f6feb; color:#fff; border:none;
            border-radius:8px; font-size:14px; font-weight:600;
        }
        QPushButton:hover { background:#388bfd; }
    )");
    btnLogin->setCursor(Qt::PointingHandCursor);
    p1Lay->addWidget(btnLogin);

    auto* btnBack = new QPushButton("← Nazad");
    btnBack->setStyleSheet("color:#8b949e; background:transparent; border:none; font-size:12px;");
    btnBack->setCursor(Qt::PointingHandCursor);
    p1Lay->addWidget(btnBack, 0, Qt::AlignCenter);
    p1Lay->addStretch();

    m_stack->addWidget(page1);

    // ── Connections ──
    connect(m_userList, &QListWidget::itemClicked, this, [this](QListWidgetItem*) {
        onUserSelected(m_userList->currentRow());
    });
    connect(m_userList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem*) {
        onUserSelected(m_userList->currentRow());
    });
    connect(btnLogin,   &QPushButton::clicked, this, &LoginScreen::onLogin);
    connect(btnBack,    &QPushButton::clicked, this, &LoginScreen::onBack);
    connect(btnExit,    &QPushButton::clicked, qApp, &QApplication::quit);
    connect(m_edtPass,  &QLineEdit::returnPressed, this, &LoginScreen::onLogin);
}

void LoginScreen::loadUsers()
{
    m_userList->clear();
    for (auto& u : Database::instance().getAllUsers()) {
        QString label = u.displayName;
        if (u.isAdmin) label += "  👑";
        auto* item = new QListWidgetItem("  👤  " + label);
        item->setData(Qt::UserRole, u.id);
        m_userList->addItem(item);
    }
}

void LoginScreen::onUserSelected(int row)
{
    auto* item = m_userList->item(row);
    if (!item) return;

    int id = item->data(Qt::UserRole).toInt();
    auto users = Database::instance().getAllUsers();
    for (auto& u : users) {
        if (u.id == id) { m_selectedUser = u; break; }
    }

    m_lblWelcome->setText("👤  " + m_selectedUser.displayName);
    m_edtPass->clear();
    m_lblError->hide();
    m_stack->setCurrentIndex(1);
    m_edtPass->setFocus();
}

void LoginScreen::onLogin()
{
    QString pass = m_edtPass->text();
    AppUser u = Database::instance().authenticate(m_selectedUser.username, pass);
    if (u.id > 0) {
        Session::instance().login(u.id, u.displayName, u.isAdmin);
        accept();
    } else {
        m_lblError->setText("❌  Pogrešna lozinka!");
        m_lblError->show();
        m_edtPass->clear();
        m_edtPass->setFocus();
    }
}

void LoginScreen::onBack()
{
    m_stack->setCurrentIndex(0);
    m_edtPass->clear();
    m_lblError->hide();
}
