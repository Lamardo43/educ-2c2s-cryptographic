#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QMessageBox>
#include <QVector>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoginButtonClicked();
    void onCardClicked();
    void onResetButtonClicked();

private:
    void setupLoginPage();
    void setupGamePage();
    void resetGame();

    QStackedWidget *stackedWidget;
    QWidget *loginPage;
    QWidget *gamePage;

    QLineEdit *pinCodeEdit;
    QPushButton *loginButton;

    QVector<QPushButton*> cards;
    QPushButton *resetButton;
    QLabel *scoreLabel;

    int correctPinCode = 1234;
    QVector<int> cardValues;
    int currentScore;
    int cardsRevealed;
};

#endif // MAINWINDOW_H
