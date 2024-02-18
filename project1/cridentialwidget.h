#ifndef CRIDENTIALWIDGET_H
#define CRIDENTIALWIDGET_H

#include <QDialog>

namespace Ui {
class CridentialWidget;
}

class CridentialWidget : public QDialog
{
    Q_OBJECT

public:
    explicit CridentialWidget(const QString&site,QWidget *parent = nullptr);
    ~CridentialWidget();

private:
    Ui::CridentialWidget *ui;
};

#endif // CRIDENTIALWIDGET_H
