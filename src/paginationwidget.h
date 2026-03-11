#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QHBoxLayout>

class PaginationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PaginationWidget(QWidget* parent = nullptr);

    void setTotal(int totalItems);          // call after data loads
    void setPageSize(int size);
    void reset();                           // go back to page 1

    int  currentPage() const  { return m_currentPage; }
    int  pageSize()    const  { return m_pageSize; }
    int  offset()      const  { return (m_currentPage - 1) * m_pageSize; }

signals:
    void pageChanged();   // connect this → reload table

private slots:
    void goFirst();
    void goPrev();
    void goNext();
    void goLast();
    void onPageSizeChanged(int index);

private:
    void updateUi();

    int m_currentPage = 1;
    int m_totalPages  = 1;
    int m_totalItems  = 0;
    int m_pageSize    = 50;

    QPushButton* m_btnFirst = nullptr;
    QPushButton* m_btnPrev  = nullptr;
    QPushButton* m_btnNext  = nullptr;
    QPushButton* m_btnLast  = nullptr;
    QLabel*      m_lblInfo  = nullptr;
    QComboBox*   m_cmbSize  = nullptr;
};
