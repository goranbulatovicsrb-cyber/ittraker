#include "paginationwidget.h"
#include <QSpacerItem>

PaginationWidget::PaginationWidget(QWidget* parent) : QWidget(parent)
{
    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 6, 0, 2);
    lay->setSpacing(6);

    // Page size selector
    auto* lblSize = new QLabel("Redova po stranici:");
    lblSize->setStyleSheet("color:#8b949e; font-size:12px;");
    lay->addWidget(lblSize);

    m_cmbSize = new QComboBox;
    m_cmbSize->setFixedWidth(70);
    m_cmbSize->addItem("25",  25);
    m_cmbSize->addItem("50",  50);
    m_cmbSize->addItem("100", 100);
    m_cmbSize->addItem("200", 200);
    m_cmbSize->setCurrentIndex(1); // default 50
    lay->addWidget(m_cmbSize);

    lay->addStretch();

    // Info label  "Prikazano 1-50 od 347"
    m_lblInfo = new QLabel;
    m_lblInfo->setStyleSheet("color:#8b949e; font-size:12px;");
    lay->addWidget(m_lblInfo);

    lay->addSpacing(12);

    // Nav buttons
    auto mkBtn = [](const QString& text, const QString& tip) {
        auto* b = new QPushButton(text);
        b->setFixedSize(32, 28);
        b->setToolTip(tip);
        b->setStyleSheet(R"(
            QPushButton {
                background:#21262d; color:#f0f6fc;
                border:1px solid #30363d; border-radius:5px;
                font-size:14px; padding:0;
            }
            QPushButton:hover   { background:#30363d; }
            QPushButton:disabled{ color:#484f58; background:#0d1117; border-color:#21262d; }
        )");
        return b;
    };

    m_btnFirst = mkBtn("«", "Prva stranica");
    m_btnPrev  = mkBtn("‹", "Prethodna");
    m_btnNext  = mkBtn("›", "Sljedeća");
    m_btnLast  = mkBtn("»", "Zadnja stranica");

    lay->addWidget(m_btnFirst);
    lay->addWidget(m_btnPrev);
    lay->addWidget(m_btnNext);
    lay->addWidget(m_btnLast);

    connect(m_btnFirst, &QPushButton::clicked, this, &PaginationWidget::goFirst);
    connect(m_btnPrev,  &QPushButton::clicked, this, &PaginationWidget::goPrev);
    connect(m_btnNext,  &QPushButton::clicked, this, &PaginationWidget::goNext);
    connect(m_btnLast,  &QPushButton::clicked, this, &PaginationWidget::goLast);
    connect(m_cmbSize,  qOverload<int>(&QComboBox::currentIndexChanged),
            this, &PaginationWidget::onPageSizeChanged);

    updateUi();
}

void PaginationWidget::setTotal(int totalItems)
{
    m_totalItems  = totalItems;
    m_totalPages  = qMax(1, (totalItems + m_pageSize - 1) / m_pageSize);
    m_currentPage = qMin(m_currentPage, m_totalPages);
    updateUi();
}

void PaginationWidget::setPageSize(int size)
{
    m_pageSize   = size;
    m_totalPages = qMax(1, (m_totalItems + m_pageSize - 1) / m_pageSize);
    m_currentPage = 1;
    updateUi();
}

void PaginationWidget::reset()
{
    m_currentPage = 1;
    updateUi();
}

void PaginationWidget::goFirst() { m_currentPage = 1;            updateUi(); emit pageChanged(); }
void PaginationWidget::goLast()  { m_currentPage = m_totalPages; updateUi(); emit pageChanged(); }

void PaginationWidget::goPrev()
{
    if (m_currentPage > 1) { m_currentPage--; updateUi(); emit pageChanged(); }
}

void PaginationWidget::goNext()
{
    if (m_currentPage < m_totalPages) { m_currentPage++; updateUi(); emit pageChanged(); }
}

void PaginationWidget::onPageSizeChanged(int index)
{
    m_pageSize    = m_cmbSize->itemData(index).toInt();
    m_totalPages  = qMax(1, (m_totalItems + m_pageSize - 1) / m_pageSize);
    m_currentPage = 1;
    updateUi();
    emit pageChanged();
}

void PaginationWidget::updateUi()
{
    bool hasPrev = m_currentPage > 1;
    bool hasNext = m_currentPage < m_totalPages;

    m_btnFirst->setEnabled(hasPrev);
    m_btnPrev->setEnabled(hasPrev);
    m_btnNext->setEnabled(hasNext);
    m_btnLast->setEnabled(hasNext);

    if (m_totalItems == 0) {
        m_lblInfo->setText("Nema podataka");
        return;
    }

    int from = offset() + 1;
    int to   = qMin(offset() + m_pageSize, m_totalItems);
    m_lblInfo->setText(QString("Stranica <b>%1</b> / %2  —  prikazano <b>%3–%4</b> od <b>%5</b>")
                           .arg(m_currentPage).arg(m_totalPages)
                           .arg(from).arg(to).arg(m_totalItems));
}
