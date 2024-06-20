#include "mainwindow.h"
#include <QRandomGenerator>
#include <QMessageBox>

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

    QRandomGenerator *random = QRandomGenerator::global();
    for (int i = 0; i < 9; ++i) {
        int value = random->bounded(-50, 51);
        cardValues.append(value);
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

    if (index != -1 && cardsRevealed < 3) {
        int value = cardValues[index];
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
