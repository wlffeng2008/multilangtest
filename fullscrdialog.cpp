#include "fullscrdialog.h"
#include "ui_fullscrdialog.h"

FullScrDialog::FullScrDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FullScrDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog|Qt::FramelessWindowHint);
    connect(ui->pushButton,&QPushButton::clicked,[=]{close();});
    showFullScreen();
}

FullScrDialog::~FullScrDialog()
{
    delete ui;
}
