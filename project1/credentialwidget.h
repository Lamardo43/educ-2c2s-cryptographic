#ifndef CREDENTIALWIDGET_H
#define CREDENTIALWIDGET_H

#include <QWidget>

namespace Ui {
class credentialwidget;
}

class credentialwidget : public QWidget
{
    Q_OBJECT

public:
    explicit credentialwidget(const QString&site,
                              const int id,
                              QWidget *parent = nullptr);
    ~credentialwidget();

private slots:
    void on_btnPassCopy_clicked();
    void on_btnLoginCopy_clicked();
signals:
    void decryptPassword(int id);
    void decryptLogin(int id);

private:
    Ui::credentialwidget *ui;
    int m_id = -1;
};

#endif // CREDENTIALWIDGET_H
