#pragma once

#include "database.h"
#include <QDialog>
#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QLabel>

class JobDialog : public QDialog
{
    Q_OBJECT
public:
    explicit JobDialog(QWidget* parent = nullptr, const WorkEntry& entry = WorkEntry{});

    WorkEntry result() const { return m_entry; }

private slots:
    void recalcTotal();
    void onAccept();

private:
    void buildUi();
    void loadClients();
    void populate(const WorkEntry& e);

    WorkEntry m_entry;
    bool      m_editMode = false;

    QComboBox*     m_cmbClient       = nullptr;
    QComboBox*     m_cmbType         = nullptr;
    QDateEdit*     m_dateEdit        = nullptr;
    QDoubleSpinBox* m_spnHours       = nullptr;
    QDoubleSpinBox* m_spnPriceHour   = nullptr;
    QDoubleSpinBox* m_spnTotal       = nullptr;
    QTextEdit*     m_txtDesc         = nullptr;
    QCheckBox*     m_chkPaid         = nullptr;
    QLabel*        m_lblTotalDisplay = nullptr;
};
