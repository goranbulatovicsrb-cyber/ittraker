#pragma once

#include "database.h"
#include <QDialog>
#include <QListWidget>
#include <QLabel>

class UserManagerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UserManagerDialog(QWidget* parent = nullptr);

private slots:
    void onAdd();
    void onDelete();
    void onChangePassword();
    void onSelectionChanged();

private:
    void buildUi();
    void loadUsers();
    int  selectedUserId() const;

    QListWidget* m_list    = nullptr;
    QPushButton* m_btnDel  = nullptr;
    QPushButton* m_btnPass = nullptr;
    QLabel*      m_lblInfo = nullptr;
};
