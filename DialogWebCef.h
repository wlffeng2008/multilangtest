#ifndef DIALOGWEBCEF_H
#define DIALOGWEBCEF_H

#include <QDialog>

namespace Ui {
class DialogWebCef;
}

class CefViewWidget;

class DialogWebCef : public QDialog
{
    Q_OBJECT

public:
    explicit DialogWebCef(QWidget *parent = nullptr);
    ~DialogWebCef();

private:
    Ui::DialogWebCef *ui;
    CefViewWidget *m_pView = nullptr;
};

#endif // DIALOGWEBCEF_H
