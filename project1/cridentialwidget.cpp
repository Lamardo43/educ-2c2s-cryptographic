#include "cridentialwidget.h"
#include "ui_cridentialwidget.h"

#include <QDialog>

CridentialWidget::CridentialWidget(const QString &site, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CridentialWidget)
{
    ui->setupUi(this);
    ui->lblSite->setText(site);
}

CridentialWidget::~CridentialWidget()
{
    delete ui;
}
