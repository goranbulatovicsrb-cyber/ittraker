#pragma once

#include "database.h"
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>

class ClientDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ClientDialog(QWidget* parent = nullptr, const Client& c = Client{});
    Client result() const { return m_client; }

private slots:
    void onAccept();

private:
    void buildUi();

    Client     m_client;
    bool       m_editMode = false;

    QLineEdit* m_edtName    = nullptr;
    QLineEdit* m_edtEmail   = nullptr;
    QLineEdit* m_edtPhone   = nullptr;
    QLineEdit* m_edtAddress = nullptr;
    QTextEdit* m_txtNotes   = nullptr;
};
