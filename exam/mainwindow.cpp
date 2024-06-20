#include "mainwindow.h"
#include <QRandomGenerator>
#include <QMessageBox>
#include <QBuffer>
#include <QCryptographicHash>

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
        int value = random->bounded(-50, 51);

        QByteArray encryptedData;
        encryptQByteArray(QByteArray::number(value), encryptedData, hash_key);
        cardValues.append(encryptedData);

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

        decryptQByteArray(cardValues[index], decryptedBytes, hash_key);

        int value = decryptedBytes.toInt();
        clickedButton->setText(QString::number(value));
        clickedButton->setEnabled(false);
        currentScore += value;
        ++cardsRevealed;

        ui->scoreLabel->setText(QString("Очки: %1").arg(currentScore));

        if (cardsRevealed == 3) {
            QMessageBox::information(this, "Результат", QString("Вы набрали %1 очков").arg(currentScore));
            resetGame();
        }
    }
}

void MainWindow::onResetButtonClicked() {
    resetGame();
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
