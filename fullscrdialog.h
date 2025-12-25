#ifndef FULLSCRDIALOG_H
#define FULLSCRDIALOG_H

#include <QDialog>

namespace Ui {
class FullScrDialog;
}

class FullScrDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FullScrDialog(QWidget *parent = nullptr);
    ~FullScrDialog();

private:
    Ui::FullScrDialog *ui;
};

#endif // FULLSCRDIALOG_H
