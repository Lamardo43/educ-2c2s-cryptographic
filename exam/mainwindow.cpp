#include "mainwindow.h"
#include <QRandomGenerator>
#include <QMessageBox>
#include <QBuffer>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include <openssl/evp.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), currentScore(0), cardsRevealed(0) {
    ui->setupUi(this);

    ui->stackedWidget->setCurrentWidget(ui->loginPage);

    connect(ui->loginButton, &QPushButton::clicked, this, &MainWindow::onLoginButtonClicked);
    connect(ui->resetButton, &QPushButton::clicked, this, &MainWindow::onResetButtonClicked);

    for (int i = 0; i < 9; ++i) {
        QPushButton *cardButton = findChild<QPushButton*>(QString("card%1").arg(i + 1));
        if (cardButton) {
            connect(cardButton, &QPushButton::clicked, this, &MainWindow::onCardClicked);
        }
    }

    resetGame();
    loadGame();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::resetGame() {
    cardValues.clear();

    currentScore = 0;
    cardsRevealed = 0;
    ui->scoreLabel->setText("Очки: 0");


    QByteArray hash = QCryptographicHash::hash(QByteArray::number(correctPinCode), QCryptographicHash::Sha256);
    unsigned char hash_key[32] = {0};
    memcpy(hash_key, hash.data(), 32);

    QRandomGenerator *random = QRandomGenerator::global();
    for (int i = 0; i < 9; ++i) {

        QByteArray encryptedData;
        encryptQByteArray(QByteArray::number(random->bounded(-50, 51)), encryptedData, hash_key);

        cardValues.append(QPair<QByteArray, bool>(encryptedData, false));

        QPushButton *cardButton = findChild<QPushButton*>(QString("card%1").arg(i + 1));
        if (cardButton) {
            cardButton->setText("");
            cardButton->setEnabled(true);
        }
    }
}

void MainWindow::onLoginButtonClicked() {
    if (ui->pinCodeEdit->text().toInt() == correctPinCode) {
        ui->stackedWidget->setCurrentWidget(ui->gamePage);
    } else {
        QMessageBox::warning(this, "Ошибка", "Неверный пин-код");
    }
}

void MainWindow::onCardClicked() {
    QPushButton *clickedButton = qobject_cast<QPushButton*>(sender());
    if (!clickedButton) return;

    int index = clickedButton->objectName().mid(4).toInt() - 1;

    QByteArray decryptedBytes;
    QByteArray hash = QCryptographicHash::hash(QByteArray::number(correctPinCode), QCryptographicHash::Sha256);
    unsigned char hash_key[32] = {0};
    memcpy(hash_key, hash.data(), 32);

    if (index != -1 && cardsRevealed < 3) {

        decryptQByteArray(cardValues[index].first, decryptedBytes, hash_key);

        int value = decryptedBytes.toInt();
        clickedButton->setText(QString::number(value));
        clickedButton->setEnabled(false);
        cardValues[index].second = true;
        currentScore += value;
        ++cardsRevealed;

        ui->scoreLabel->setText(QString("Очки: %1").arg(currentScore));

        if (cardsRevealed == 3) {
            QMessageBox::information(this, "Результат", QString("Набрано очков: %1").arg(currentScore));
            resetGame();
        }
    }
}

void MainWindow::onResetButtonClicked() {
    resetGame();
}

void MainWindow::loadGame() {
    cardValues.clear();

    QFile jsonFile(jsonPath);
    if (!jsonFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Не удалось открыть файл для чтения";
        return;
    }

    QByteArray jsonData;
    QByteArray hash = QCryptographicHash::hash(QByteArray::number(correctPinCode), QCryptographicHash::Sha256);
    unsigned char hash_key[32] = {0};
    memcpy(hash_key, hash.data(), 32);

    decryptQByteArray(jsonFile.readAll(), jsonData, hash_key);
    jsonFile.close();

    QJsonParseError error;
    QJsonObject jsonObject = QJsonDocument::fromJson(jsonData, &error).object();

    currentScore = jsonObject["currentScore"].toInt();
    cardsRevealed = jsonObject["cardsRevealed"].toInt();
    QJsonArray jsonArray = jsonObject["cardValues"].toArray();

    if(jsonArray.size() == 9) {
        for (int index = 0; index < jsonArray.size(); index++) {
            const QJsonObject &obj = jsonArray[index].toObject();

            QByteArray hash = QCryptographicHash::hash(QByteArray::number(correctPinCode), QCryptographicHash::Sha256);
            unsigned char hash_key[32] = {0};
            memcpy(hash_key, hash.data(), 32);

            QByteArray decryptedBytes;
            QByteArray byteArrayValue = QByteArray::fromBase64(obj["value"].toString().toUtf8());

            decryptQByteArray(byteArrayValue, decryptedBytes, hash_key);

            bool boolValue = obj["checked"].toBool();
            cardValues.append(QPair<QByteArray, bool>(byteArrayValue, boolValue));

            if (boolValue) {
                int card_value = decryptedBytes.toInt();

                QPushButton *cardButton = findChild<QPushButton*>(QString("card%1").arg(index + 1));
                cardButton->setText(QString::number(card_value));
                cardButton->setEnabled(false);

                ui->scoreLabel->setText(QString("Очки: %1").arg(currentScore));
            }
        }

    } else {
        resetGame();
    }


}

void MainWindow::saveJson() {
    QFile jsonFile(jsonPath);

    jsonFile.resize(0);

    QJsonArray jsonArray;

    for (const auto &pair : cardValues) {
        QJsonObject jsonObject;
        jsonObject["value"] = QString::fromUtf8(pair.first.toBase64());
        jsonObject["checked"] = pair.second;
        jsonArray.append(jsonObject);
    }

    QJsonObject jsonMainObject;
    jsonMainObject["cardValues"] = jsonArray;
    jsonMainObject["currentScore"] = currentScore;
    jsonMainObject["cardsRevealed"] = cardsRevealed;

    QJsonDocument jsonDocument(jsonMainObject);

    QByteArray hash = QCryptographicHash::hash(QByteArray::number(correctPinCode), QCryptographicHash::Sha256);
    unsigned char hash_key[32] = {0};
    memcpy(hash_key, hash.data(), 32);

    QByteArray encryptedData;

    encryptQByteArray(jsonDocument.toJson(), encryptedData, hash_key);


    if (jsonFile.open(QIODevice::WriteOnly)) {
        jsonFile.write(encryptedData);
        jsonFile.close();
        qDebug() << "JSON данные сохранены в файл output.json";
    } else {
        qWarning() << "Не удалось открыть файл для записи";
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    saveJson();

    QMainWindow::closeEvent(event);
}

int MainWindow::decryptQByteArray(const QByteArray& encryptedBytes, QByteArray& decryptedBytes, unsigned char *key)
{
    qDebug() << "Decrypting";
    QByteArray iv_hex("00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f");
    QByteArray iv_ba = QByteArray::fromHex(iv_hex);

    unsigned char iv[16] = {0};
    memcpy(iv, iv_ba.data(), 16);

    EVP_CIPHER_CTX *ctx;
    ctx = EVP_CIPHER_CTX_new();
    if (!EVP_DecryptInit_ex2(ctx, EVP_aes_256_cbc(), key, iv, NULL)) {
        qDebug() << "Error";
        /* Error */
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }

    #define BUF_LEN 256
    unsigned char encrypted_buf[BUF_LEN] = {0}, decrypted_buf[BUF_LEN] = {0};
    int encr_len, decr_len;

    QDataStream encrypted_stream(encryptedBytes);

    decryptedBytes.clear();
    QBuffer decryptedBuffer(&decryptedBytes);
    decryptedBuffer.open(QIODevice::ReadWrite);


    encr_len = encrypted_stream.readRawData(reinterpret_cast<char*>(encrypted_buf), BUF_LEN);
    while(encr_len > 0){

        if (!EVP_DecryptUpdate(ctx, decrypted_buf, &decr_len, encrypted_buf, encr_len)) {
            /* Error */
            qDebug() << "Error";
            EVP_CIPHER_CTX_free(ctx);
            return 0;
        }

        decryptedBuffer.write(reinterpret_cast<char*>(decrypted_buf), decr_len);
        encr_len = encrypted_stream.readRawData(reinterpret_cast<char*>(encrypted_buf), BUF_LEN);
    }

    int tmplen;
    if (!EVP_DecryptFinal_ex(ctx, decrypted_buf, &tmplen)) {
        /* Error */
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    decryptedBuffer.write(reinterpret_cast<char*>(decrypted_buf), tmplen);
    EVP_CIPHER_CTX_free(ctx);

    decryptedBuffer.close();
    return 0;
}

int MainWindow::encryptQByteArray(const QByteArray &plainBytes, QByteArray &encryptedBytes, unsigned char *key)
{
    qDebug() << "Encrypting";

    QByteArray iv_hex("00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f");
    QByteArray iv_ba = QByteArray::fromHex(iv_hex);
    unsigned char iv[16] = {0};
    memcpy(iv, iv_ba.data(), 16);

    EVP_CIPHER_CTX *ctx;
    ctx = EVP_CIPHER_CTX_new();
    if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv)) {
        qDebug() << "Error: EVP_EncryptInit_ex";
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }

    #define BUF_LEN 256
    unsigned char encrypted_buf[BUF_LEN] = {0}, plain_buf[BUF_LEN] = {0};
    int encr_len, plain_len;

    QDataStream plain_stream(plainBytes);

    encryptedBytes.clear();
    QBuffer encryptedBuffer(&encryptedBytes);
    encryptedBuffer.open(QIODevice::ReadWrite);

    plain_len = plain_stream.readRawData(reinterpret_cast<char*>(plain_buf), BUF_LEN);
    while (plain_len > 0) {
        if (!EVP_EncryptUpdate(ctx, encrypted_buf, &encr_len, plain_buf, plain_len)) {
            qDebug() << "Error: EVP_EncryptUpdate";
            EVP_CIPHER_CTX_free(ctx);
            return 0;
        }

        encryptedBuffer.write(reinterpret_cast<char*>(encrypted_buf), encr_len);
        plain_len = plain_stream.readRawData(reinterpret_cast<char*>(plain_buf), BUF_LEN);
    }

    int tmplen;
    if (!EVP_EncryptFinal_ex(ctx, encrypted_buf, &tmplen)) {
        qDebug() << "Error: EVP_EncryptFinal_ex";
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    encryptedBuffer.write(reinterpret_cast<char*>(encrypted_buf), tmplen);
    EVP_CIPHER_CTX_free(ctx);

    encryptedBuffer.close();
    return 0;
}
