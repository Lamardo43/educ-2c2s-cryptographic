#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "cridentialwidget.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug() << "*** readJSON() = " << readJSON() << "\n";

    connect(ui->lineEdit, &QLineEdit::textChanged, this, &MainWindow::filterText);

    for(int i = 0; i < m_jsonarray.size(); i++) {
        //qDebug() << "*** m_jsonarray[" << i << "] = " << m_jsonarray[i]; это встроенный делегат
        //ui->listWidget->addItem(m_jsonarray[i].toObject()["site"].toString());   отображает на пробу только 1 строку в каждой учетке
        QListWidgetItem * newItem = new QListWidgetItem();
        /*if(m_jsonarray[i].isObject()){ // есть ли поле site
            if(m_jsonarray[i].toObject().contains("site")){
                if(m_jsonarray[i].toObject()["site"].isString()) {

                }
            }
        }*/

        CridentialWidget * itemWidget =
            new CridentialWidget(m_jsonarray[i].toObject()["site"].toString());

        newItem->setSizeHint(itemWidget->sizeHint());
        ui->listWidget->addItem(newItem);
        ui->listWidget->setItemWidget(newItem, itemWidget);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

// функция с фильтром
void MainWindow::filterText(const QString &text)
{
    if (text.isEmpty()){
        return;
    }

    ui->listWidget->clear();

    for (int i = 0; i < m_jsonarray.size(); i++) {
        QString site = m_jsonarray[i].toObject()["site"].toString();
        if (site.contains(text)) {
            QListWidgetItem *newItem = new QListWidgetItem();
            CridentialWidget *itemWidget = new CridentialWidget(site);
            newItem->setSizeHint(itemWidget->sizeHint());
            ui->listWidget->addItem(newItem);
            ui->listWidget->setItemWidget(newItem, itemWidget);
        }
    }
}

/*функция считывает учетные записи из файла JSON в структуру данных QList*/
bool MainWindow::readJSON()
{
    QFile jsonFile("cridentials.json");
    jsonFile.open(QFile::ReadOnly);
    if (!jsonFile.isOpen())
        return false;

    QJsonDocument jsonDoc = QJsonDocument(QJsonDocument::fromJson(jsonFile.readAll()));
    qDebug() << "*** jsonDoc = " << jsonDoc << "\n";
    QJsonObject rootObject = jsonDoc.object();
    qDebug() << "*** rootObject = " << rootObject << "\n";

    m_jsonarray = rootObject["cridentials"].toArray();

    jsonFile.close();

    return true;
}
