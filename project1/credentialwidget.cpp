#include "credentialwidget.h"
#include "ui_credentialwidget.h"

credentialwidget::credentialwidget(const QString &site, QWidget *parent)
    :QWidget(parent)
    , ui(new Ui::credentialwidget)
    , m_id(id)
{
    ui->setupUi(this);
    ui->lblSite->setText(site);
}

credentialwidget::~credentialwidget()
{
    delete ui;
}

void credentialwidget::on_btnLoginCopy_clicked()
{
    qDebug() << "*** Pressed" << m_id;
    emit decryptLogin(m_id);
}

void credentialwidget::on_btnPassCopy_clicked()
{
    qDebug() << "*** Pressed" << m_id;
    emit decryptPassword(m_id);
}
