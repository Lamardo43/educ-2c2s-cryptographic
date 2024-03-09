#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "credentialwidget.h"

#include <QBuffer>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLineEdit>
#include <QJsonParseError>

#include <openssl/evp.h>

/*TODO
 * 1) создать незащищённффй файл с учетными записями
 * 2) считать его в приложении Qt
 */


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

/* Функция считывает учётные записи из файла jsom в структуру данных Qlist*/
bool MainWindow::readJSON(const QByteArray & aes256_key)
{
    QFile jsonFile("cridentials_enc.json");
    jsonFile.open(QFile::ReadOnly);
    if(!jsonFile.isOpen())
        return false;

    // jsonFile.readAll() -> QByteArray(hex+encrypted)
    // QByteArray(hex+encrypted) -> QByteArray::fromHex -> QByteArray(encrypted)
    // QByteArray(encrypted) -> расшифровка -> QByteArray(decrypted)
    // QByteArray(decrypted) -> парсинг JSON

    QByteArray hexEncryptedBytes = jsonFile.readAll();
    qDebug() << "*** hexEncryptedBytes" << hexEncryptedBytes;
    QByteArray encryptedBytes = QByteArray::fromHex(hexEncryptedBytes);
    qDebug() << "*** hexEncryptedBytes" << encryptedBytes;
    QByteArray decryptedBytes;
    int ret_code = decryptFile(aes256_key, encryptedBytes, decryptedBytes);
    qDebug() << "*** decryptFile(), decryptedBytes = " << decryptedBytes;

    QJsonParseError jsonErr;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(decryptedBytes, &jsonErr);
    if(jsonErr.error != QJsonParseError::NoError)
        return false;
    qDebug() << "*** jsonDoc = " << jsonDoc;
    QJsonObject rootObject = jsonDoc.object();
    qDebug() << "*** rootObject = " << rootObject;

    m_jsonarray = rootObject["cridentials"].toArray();

    jsonFile.close();
}


void MainWindow::filterListWidget(const QString &searchString)
{

        ui->listWidget->clear();
        for (int i = 0; i < m_jsonarray.size(); i++){
            QListWidgetItem * newItem = new QListWidgetItem();
            if(m_jsonarray[i].toObject()["site"]
                    .toString().toLower()
                    .contains(searchString.toLower())
                || searchString.isEmpty())
            {
                credentialwidget * itemWidget =
                    new credentialwidget(m_jsonarray[i].toObject()["site"].toString(), i);
                QObject::connect(itemWidget, &credentialwidget::decryptLogin,
                                 this, &MainWindow::decryptLogin);
                QObject::connect(itemWidget, &credentialwidget::decryptPassword,
                                 this, &MainWindow::decryptPassword);
                QListWidgetItem * newItem = new QListWidgetItem();
                newItem->setSizeHint(itemWidget->sizeHint());
                ui->listWidget->addItem(newItem);
                ui->listWidget->setItemWidget(newItem, itemWidget);
            }
        }
}

int MainWindow::decryptFile(
    const QByteArray & aes256_key,
    const QByteArray & encryptedBytes,
    QByteArray & decryptedBytes
    )
{
   // https://cryptii.com/pipes/aes-encryption
   // iv: de1358eb7cd471c58dc76ea9a5977983
   // key: 67fda910f381a20eac75f6244cbf93d9190f89050489fdd1d215ac48aca21efd
   //
   // задать ключ и инициализирующий вектор
   //unsigned char key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
   //unsigned char iv[] = {1,2,3,4,5,6,7,8};

    //QByteArray key_hex("67fda910f381a20eac75f6244cbf93d9190f89050489fdd1d215ac48aca21efd");
    //QByteArray key_ba = QByteArray::fromHex(key_hex);

    qDebug() << "*** key_ba " << aes256_key;
    unsigned char key[32] = {0};
    memcpy(key, aes256_key.data(), 32);
    qDebug() << "*** key " << key;


    QByteArray iv_hex("de1358eb7cd471c58dc76ea9a5977983");
    QByteArray iv_ba = QByteArray::fromHex(iv_hex);
    unsigned char iv[16] = {0};
    memcpy(iv, iv_ba.data(), 16);
    qDebug() << "*** iv " << iv;

    EVP_CIPHER_CTX *ctx;
    ctx = EVP_CIPHER_CTX_new();
    if (!EVP_EncryptInit_ex2(ctx, EVP_aes_256_cbc(), key, iv, NULL)) {
           qDebug() << "*** EVP_DecryptInit_ex2() ERROR ";
           EVP_CIPHER_CTX_free(ctx);
           return 0;
    }
    qDebug() << "*** EVP_DecryptInit_ex2() OK ";
    #define BUF_LEN 256
    unsigned char encrypted_buf[BUF_LEN] = {0}, decrypted_buf[BUF_LEN] = {0};
    int encr_len, decr_len;

    QDataStream encrypted_stream(encryptedBytes);
    decryptedBytes.clear();

    QBuffer decrypted_buffer(&decryptedBytes);
    decrypted_buffer.open(QIODevice::WriteOnly);
    // QDataStream decrypted_stream(&decryptedBytes, QIODevice::ReadWrite);
    encr_len = encrypted_stream.readRawData(reinterpret_cast<char*>(encrypted_buf), BUF_LEN);
    while(encr_len > 0)
    {
        if (!EVP_DecryptUpdate(ctx,
                               decrypted_buf, &decr_len,
                               encrypted_buf, encr_len)) {
            EVP_CIPHER_CTX_free(ctx);
            return 0;
        }
    qDebug() << "*** EVP_DecryptUpdate():" << reinterpret_cast<char*>(decrypted_buf);
    // накопление расшифрованного результата
    QByteArray tmp_buf(reinterpret_cast<char*>(decrypted_buf), decr_len);
    qDebug() << "*** tmp_buf = " << tmp_buf;
    decrypted_buffer.write(reinterpret_cast<char*>(decrypted_buf), decr_len);
    // считывание след порции шифр массива
    encr_len = encrypted_stream.readRawData(reinterpret_cast<char*>(encrypted_buf), BUF_LEN);
    }
    qDebug() << "*** before EVP_DecryptFinale_ex" << decr_len <<
        reinterpret_cast<char*>(decrypted_buf);

    int tmplen;
    if (!EVP_DecryptFinal_ex(ctx, decrypted_buf, &tmplen)){
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    qDebug() << "*** after EVP_DecryptFinal_ex" << tmplen << reinterpret_cast<char*>(decrypted_buf);
    decrypted_buffer.write(reinterpret_cast<char*>(decrypted_buf), tmplen);
    decrypted_buffer.close();
    EVP_CIPHER_CTX_free(ctx);

    return 0;
}

void MainWindow::on_editPin_returnPressed()
{
    // получить ключ из пин-кода
    QByteArray hash = QCryptographicHash::hash(
        ui->editPin->text().toUtf8(),
        QCryptographicHash::Sha256);
    qDebug() << "*** Sha256 = " << hash.toHex();

    // расшифровать файл и проверить верность пин-кода
    // если верный - сменить панель и отрисовать список
    // если неверный - предупреждение
    if(readJSON(hash)) {
        ui->stackedWidget->setCurrentIndex(1);
        filterListWidget("");
    }else {
        ui->lblLogin->setText("Неверный пин");
        ui->lblLogin->setStyleSheet("color:red;");
    }

    else {
          QByteArray encrypted_creds = QByteArray::FromHex(
            m_jsonarray[m_current_id].toObject()["logpass"].toString().toUtf8()
            );
        QByteArray decrypted_creds;

        decryptFle(hash, encrypted_creds, decrypted_creds);
        QGuiApplication::clipboard() -> setText(QString::fromUtf8(decrypted_creds));
        ui->stackedWidget->setCurrentIndex(1);
    }

    ui->editPin->setText(QString().fill('*', ui->editPin->text().size()));
    ui->editPin->clear();
    hash.setRawData(
        const_cast<char*>(QByteArray().fill('*', 32).data() ),
        32);
    hash.clear();
    // удалить ключ и пин код
}

void MainWindow::decryptLogin(int id)
{

}
