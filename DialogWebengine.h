#ifndef DIALOGWEBENGINE_H
#define DIALOGWEBENGINE_H

#include <QDialog>

#include <easywebview.h>

namespace Ui {
class DialogWebengine;
}

class DialogWebengine : public QDialog
{
    Q_OBJECT

public:
    explicit DialogWebengine(QWidget *parent = nullptr);
    ~DialogWebengine();

protected:
    void showEvent(QShowEvent *pEv) override;


private:
    Ui::DialogWebengine *ui;

    EasyWebView *m_pView = nullptr;
};

#endif // DIALOGWEBENGINE_H
