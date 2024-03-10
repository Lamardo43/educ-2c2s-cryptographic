#include "listitem.h"
#include "ui_listitem.h"
#include "mainwindow.h"

ListItem::ListItem(QString site, QString login_encrypted, QString password_encrypted, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ListItem)
{
    this->pass_encr = password_encrypted;
    this->log_encr = login_encrypted;

    ui->setupUi(this);

    ui->siteName->setText(site);
    ui->loginLineEdit->setText("******");
    ui->passwordLineEdit->setText("******");

    QPixmap pix(":/img/img/keys.png");
    int w = ui->iconKeys->width();
    int h = ui->iconKeys->height();

    ui->iconKeys->setPixmap(pix.scaled(w,h, Qt::KeepAspectRatio));
}

ListItem::~ListItem()
{
    delete ui;
}

void ListItem::on_lineEdit_2_selectionChanged()
{
    ui->loginLineEdit->setText("encrypted");
}

void ListItem::on_lineEdit_2_editingFinished()
{
    ui->loginLineEdit->setText("******");
}

