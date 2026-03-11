#include "mainwindow.h"
#include "dashboard.h"
#include "clientspage.h"
#include "worklogpage.h"
#include "style.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QStatusBar>

// ─── Constructor ──────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("IT Tracker — Evidencija Zarade");
    setMinimumSize(1100, 680);
    resize(1280, 760);
    buildUi();
    switchPage(0);
}

// ─── Build UI ─────────────────────────────────────────────────────────────────

void MainWindow::buildUi()
{
    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* rootLay = new QHBoxLayout(central);
    rootLay->setContentsMargins(0,0,0,0);
    rootLay->setSpacing(0);

    // ── Sidebar ──────────────────────────────────────────────────────────────
    auto* sidebar = new QFrame;
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(210);

    auto* sidebarLay = new QVBoxLayout(sidebar);
    sidebarLay->setContentsMargins(0,0,0,0);
    sidebarLay->setSpacing(0);

    // App logo / title
    auto* logo = new QWidget;
    logo->setFixedHeight(64);
    logo->setStyleSheet("background:#0d1117; border-bottom: 1px solid #21262d;");
    auto* logoLay = new QHBoxLayout(logo);
    logoLay->setContentsMargins(20,0,20,0);
    auto* logoIcon = new QLabel("💻");
    logoIcon->setStyleSheet("font-size:22px;");
    auto* logoText = new QLabel("IT Tracker");
    logoText->setStyleSheet("font-size:16px; font-weight:700; color:#f0f6fc; letter-spacing:0.5px;");
    logoLay->addWidget(logoIcon);
    logoLay->addWidget(logoText);
    logoLay->addStretch();
    sidebarLay->addWidget(logo);

    // Nav section label
    auto* navLabel = new QLabel("NAVIGACIJA");
    navLabel->setContentsMargins(20,16,0,6);
    navLabel->setStyleSheet("font-size:10px; font-weight:700; color:#484f58; letter-spacing:1.5px;");
    sidebarLay->addWidget(navLabel);

    // Nav buttons
    struct NavItem { QString icon; QString label; };
    const QList<NavItem> items = {
        {"📊", "Dashboard"},
        {"📋", "Evidencija poslova"},
        {"👤", "Mušterije"},
    };

    for (int i = 0; i < items.size(); i++) {
        auto* btn = new QPushButton(items[i].icon + "  " + items[i].label);
        btn->setObjectName("sidebarBtn");
        btn->setCheckable(true);
        btn->setFixedHeight(44);
        btn->setCursor(Qt::PointingHandCursor);
        sidebarLay->addWidget(btn);
        m_navBtns.append(btn);

        connect(btn, &QPushButton::clicked, this, [this, i]{
            switchPage(i);
        });
    }

    sidebarLay->addStretch();

    // Footer
    auto* footer = new QLabel("v1.0  |  IT Tracker");
    footer->setAlignment(Qt::AlignCenter);
    footer->setContentsMargins(0,0,0,12);
    footer->setStyleSheet("font-size:10px; color:#484f58;");
    sidebarLay->addWidget(footer);

    rootLay->addWidget(sidebar);

    // ── Separator ────────────────────────────────────────────────────────────
    auto* sep = new QFrame;
    sep->setFrameShape(QFrame::VLine);
    sep->setStyleSheet("background:#21262d;");
    sep->setFixedWidth(1);
    rootLay->addWidget(sep);

    // ── Main stack ───────────────────────────────────────────────────────────
    m_stack = new QStackedWidget;

    m_dash    = new Dashboard(m_stack);
    m_worklog = new WorkLogPage(m_stack);
    m_clients = new ClientsPage(m_stack);

    m_stack->addWidget(m_dash);    // index 0
    m_stack->addWidget(m_worklog); // index 1
    m_stack->addWidget(m_clients); // index 2

    rootLay->addWidget(m_stack, 1);

    // ── Cross-page signals ───────────────────────────────────────────────────
    connect(m_worklog, &WorkLogPage::dataChanged, m_dash, &Dashboard::refresh);
    connect(m_worklog, &WorkLogPage::dataChanged, m_clients, &ClientsPage::refresh);

    // Status bar
    statusBar()->setStyleSheet("background:#010409; color:#8b949e; border-top:1px solid #21262d;");
    statusBar()->showMessage("IT Tracker pokrenut.");
}

// ─── Navigation ───────────────────────────────────────────────────────────────

void MainWindow::switchPage(int index)
{
    for (int i = 0; i < m_navBtns.size(); i++)
        m_navBtns[i]->setChecked(i == index);

    m_stack->setCurrentIndex(index);

    // Refresh data on page switch
    switch (index) {
        case 0: m_dash->refresh();    break;
        case 1: m_worklog->refresh(); break;
        case 2: m_clients->refresh(); break;
    }
}
