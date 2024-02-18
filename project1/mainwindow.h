#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QJsonArray>
#include <QMainWindow>

class Cridential {
public:
    QString hostname;
    QString login;
    QString password;

};

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool readJSON();

private:
    Ui::MainWindow *ui;
    QJsonArray m_jsonarray; //структура данных, содержащая учетные записи

private:
    void filterText(const QString &text);
};

#endif // MAINWINDOW_H
