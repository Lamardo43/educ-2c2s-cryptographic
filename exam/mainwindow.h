#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

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
    void resetGame();

    Ui::MainWindow *ui;
    int correctPinCode = 1234;
    QVector<int> cardValues;
    int currentScore;
    int cardsRevealed;
};

#endif // MAINWINDOW_H
