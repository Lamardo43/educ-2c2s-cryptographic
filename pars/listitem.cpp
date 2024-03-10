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

// void ListItem::on_lineEdit_2_selectionChanged()
// {
//     ui->loginLineEdit->setText("encrypted");
// }

// void ListItem::on_lineEdit_2_editingFinished()
// {
//     ui->loginLineEdit->setText("******");
// }


void ListItem::on_copyLoginButton_clicked()
{
    // qDebug() << log_encr;

    emit enterPinSignal();
}

void ListItem::on_pinEntered() {

    QString pin = "6060";

    QByteArray hash = QCryptographicHash::hash(pin.toUtf8(), QCryptographicHash::Sha256);

    qDebug() << "***Hash -> " << hash;

    QCryptographicHash::hash(pin.toUtf8(), QCryptographicHash::Sha256);

    unsigned char hash_key[32] = {0};
    memcpy(hash_key, hash.data(), 32);
    qDebug() << "***hash_key -> " << hash_key;


    // qDebug() << "***hexEncryptedBytes" << hexEncryptedBytes;
    QByteArray encryptedBytes = QByteArray::fromHex(log_encr.toLatin1());
    // qDebug() << "***encryptedBytes" << encryptedBytes;
    QByteArray decryptedBytes;
    //
    int ret_code = MainWindow::decryptQByteArray(encryptedBytes, decryptedBytes, hash_key);


    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(decryptedBytes);
}

