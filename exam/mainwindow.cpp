#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRandomGenerator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentScore(0), cardsRevealed(0) {
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    setupLoginPage();
    setupGamePage();

    stackedWidget->addWidget(loginPage);
    stackedWidget->addWidget(gamePage);

    stackedWidget->setCurrentWidget(loginPage);
}

MainWindow::~MainWindow() {
}

void MainWindow::setupLoginPage() {
    loginPage = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(loginPage);

    pinCodeEdit = new QLineEdit(this);
    pinCodeEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(pinCodeEdit);

    loginButton = new QPushButton("Вход", this);
    connect(loginButton, &QPushButton::clicked, this, &MainWindow::onLoginButtonClicked);
    layout->addWidget(loginButton);

    loginPage->setLayout(layout);
}

void MainWindow::setupGamePage() {
    gamePage = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(gamePage);

    QHBoxLayout *topLayout = new QHBoxLayout();
    scoreLabel = new QLabel("Очки: 0", this);
    topLayout->addWidget(scoreLabel);

    resetButton = new QPushButton("Сброс", this);
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::onResetButtonClicked);
    topLayout->addWidget(resetButton);

    mainLayout->addLayout(topLayout);

    QGridLayout *gridLayout = new QGridLayout();
    for (int i = 0; i < 9; ++i) {
        QPushButton *cardButton = new QPushButton("Карта", this);
        cardButton->setFixedSize(100, 100);
        connect(cardButton, &QPushButton::clicked, this, &MainWindow::onCardClicked);
        cards.append(cardButton);
        gridLayout->addWidget(cardButton, i / 3, i % 3);
    }
    mainLayout->addLayout(gridLayout);

    gamePage->setLayout(mainLayout);
    resetGame();
}

void MainWindow::resetGame() {
    cardValues.clear();
    currentScore = 0;
    cardsRevealed = 0;
    scoreLabel->setText("Очки: 0");

    QRandomGenerator *random = QRandomGenerator::global();
    for (int i = 0; i < 9; ++i) {
        int value = random->bounded(-50, 51);
        cardValues.append(value);
        cards[i]->setText("Карта");
        cards[i]->setEnabled(true);
    }
}

void MainWindow::onLoginButtonClicked() {
    if (pinCodeEdit->text().toInt() == correctPinCode) {
        stackedWidget->setCurrentWidget(gamePage);
    } else {
        QMessageBox::warning(this, "Ошибка", "Неверный пин-код");
        pinCodeEdit->clear();
    }
}

void MainWindow::onCardClicked() {
    QPushButton *clickedButton = qobject_cast<QPushButton*>(sender());
    int index = cards.indexOf(clickedButton);

    if (index != -1 && cardsRevealed < 3) {
        int value = cardValues[index];
        clickedButton->setText(QString::number(value));
        clickedButton->setEnabled(false);
        currentScore += value;
        ++cardsRevealed;

        scoreLabel->setText(QString("Очки: %1").arg(currentScore));

        if (cardsRevealed == 3) {
            QMessageBox::information(this, "Результат", QString("Вы набрали %1 очков").arg(currentScore));
            resetGame();
        }
    }
}

void MainWindow::onResetButtonClicked() {
    resetGame();
}
