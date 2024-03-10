#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "listitem.h"

#include <openssl/evp.h>


#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QPixmap>
#include <QLineEdit>
#include <QBuffer>
#include <QCryptographicHash>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    QObject::connect(ui->lineEdit, &QLineEdit::textEdited, this, &MainWindow::filterListWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::readJSON(unsigned char *key)
{
    QFile jsonFile("D:/Education/Education4/enc/pars/json/cridentials_encrypted.json");
    if(!jsonFile.open(QIODevice::ReadOnly))
        return false;

    QByteArray hexEncryptedBytes = jsonFile.readAll();
//    qDebug() << "***hexEncryptedBytes" << hexEncryptedBytes;
    QByteArray encryptedBytes = QByteArray::fromHex(hexEncryptedBytes);
//    qDebug() << "***encryptedBytes" << encryptedBytes;
    QByteArray decryptedBytes;
//    qDebug() << "***decryptedBytes" << decryptedBytes;
    int ret_code = decryptQByteArray(encryptedBytes, decryptedBytes, key);

//    qDebug() << "***decryptedBytes " << decryptedBytes;


    QJsonDocument jsonDoc = QJsonDocument::fromJson(decryptedBytes);
//    qDebug() << "***jsonDoc " << jsonDoc;

    QJsonObject jsonObj = jsonDoc.object();
//    qDebug() << "***jsonObj " << jsonObj;

    jsonArr = jsonObj["cridentials"].toArray();
//    qDebug() << "***jsonArr " << jsonArr;

    jsonFile.close();
    return true;
}

void MainWindow::filterListWidget(const QString &searchStrings)
{
    ui->listWidget->clear();

    for (int i = 0; i != jsonArr.size(); ++i)

    {
        QJsonObject jsonItem = jsonArr[i].toObject();

        if ((searchStrings == "") || jsonItem["site"].toString().toLower().contains(searchStrings.toLower()))
        {
            QListWidgetItem *newItem = new QListWidgetItem();
            ListItem *itemWidget = new ListItem(jsonItem["site"].toString(), jsonItem["login"].toString(), jsonItem["password"].toString());

            ui->listWidget->addItem(newItem);
            ui->listWidget->setItemWidget(newItem, itemWidget);

            newItem->setSizeHint(itemWidget->sizeHint());
        }
    }
}

int MainWindow::decryptQByteArray(const QByteArray& encryptedBytes, QByteArray& decryptedBytes, unsigned char *key)
{

//    QByteArray key_hex("060e33205a731400c2eb92bc12cf921a4e44cf1851d216f144337dd6ec5350a7");
//    QByteArray key_ba = QByteArray::fromHex(key_hex);
//    qDebug() << "***key_ba " << key_ba;
//    unsigned char key[32] = {0};
//    memcpy(key, key_ba.data(), 32);
    qDebug() << "key " << key;

    QByteArray iv_hex("00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f");
    QByteArray iv_ba = QByteArray::fromHex(iv_hex);
//    qDebug() << "***iv_ba " << iv_ba;
    unsigned char iv[16] = {0};
    memcpy(iv, iv_ba.data(), 16);
//    qDebug() << "iv " << iv;

    EVP_CIPHER_CTX *ctx;
    ctx = EVP_CIPHER_CTX_new();
    if (!EVP_DecryptInit_ex2(ctx, EVP_aes_256_cbc(), key, iv, NULL)) {
        qDebug() << "Error";
        /* Error */
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    qDebug() << "NoError";

    #define BUF_LEN 256
    unsigned char encrypted_buf[BUF_LEN] = {0}, decrypted_buf[BUF_LEN] = {0};
    int encr_len, decr_len;

    QDataStream encrypted_stream(encryptedBytes);

    decryptedBytes.clear();
    QBuffer decryptedBuffer(&decryptedBytes);
    decryptedBuffer.open(QIODevice::ReadWrite);
//    QDataStream decrypted_stream(&buffer);


    encr_len = encrypted_stream.readRawData(reinterpret_cast<char*>(encrypted_buf), BUF_LEN);
    while(encr_len > 0){
//        encr_len = encrypted_stream.readRawData(reinterpret_cast<char*>(encrypted_buf), BUF_LEN);
        qDebug() << "***encr_len " << encr_len;
        if (!EVP_DecryptUpdate(ctx, decrypted_buf, &decr_len, encrypted_buf, encr_len)) {
            /* Error */
            qDebug() << "Error";
            EVP_CIPHER_CTX_free(ctx);
            return 0;
        }

        decryptedBuffer.write(reinterpret_cast<char*>(decrypted_buf), decr_len);
        encr_len = encrypted_stream.readRawData(reinterpret_cast<char*>(encrypted_buf), BUF_LEN);
        qDebug() << "***EVP_EncryptUpdate " << reinterpret_cast<char*>(decrypted_buf);
    }

    int tmplen;
    if (!EVP_DecryptFinal_ex(ctx, decrypted_buf, &tmplen)) {
          /* Error */
          EVP_CIPHER_CTX_free(ctx);
          return -1;
      }
      qDebug() << "***EVP_DecryptFinal_ex " << reinterpret_cast<char*>(decrypted_buf);
      decryptedBuffer.write(reinterpret_cast<char*>(decrypted_buf), tmplen);
      EVP_CIPHER_CTX_free(ctx);

    decryptedBuffer.close();
    return 0;
}

//IV: AAB1C2D3A4B5C6B7A8A9BA0B0C0D0E0F

//password = 6060

//key = sha256(password) = 060e33205a731400c2eb92bc12cf921a4e44cf1851d216f144337dd6ec5350a7

//void MainWindow::on_lineEdit_2_returnPressed()
//{
//    ui->stackedWidget->setCurrentIndex(1);
//}


//void MainWindow::on_pushButton_clicked()
//{
//    ui->stackedWidget->setCurrentIndex(0);
//}


void MainWindow::on_lineEdit_2_returnPressed()
{
    qDebug() << "***Password -> " <<  ui->lineEdit_2->text().toUtf8();

    QByteArray hash = QCryptographicHash::hash(ui->lineEdit_2->text().toUtf8(), QCryptographicHash::Sha256);

    qDebug() << "***Hash -> " << hash;


    unsigned char hash_key[32] = {0};
    memcpy(hash_key, hash.data(), 32);
    qDebug() << "***hash_key -> " << hash_key;

    qDebug() << "***readJSON -> " << readJSON(hash_key);

    if (readJSON(hash_key) == 1)
    {
        ui->stackedWidget->setCurrentIndex(0);
        filterListWidget("");
    }


}

