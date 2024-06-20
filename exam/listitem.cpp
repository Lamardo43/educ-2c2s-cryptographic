#include "listitem.h"
#include "ui_listitem.h"
#include "mainwindow.h"
#include <QCryptographicHash>
#include <QClipboard>

ListItem::ListItem(QString site, QString login_encrypted, QString password_encrypted, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ListItem)
{
    this->pass_encr = password_encrypted;
    this->log_encr = login_encrypted;

    ui->setupUi(this);
}

ListItem::~ListItem()
{
    delete ui;
}



void ListItem::on_copyLoginButton_clicked()
{
    emit enterPinSignal(log_encr);
}


void ListItem::on_copyPasswordButton_clicked()
{
    emit enterPinSignal(pass_encr);
}

