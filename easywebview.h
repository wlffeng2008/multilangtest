#ifndef EASYWEBVIEW_H
#define EASYWEBVIEW_H

#include <QWidget>
#include <QWebEngineView>
#include <QContextMenuEvent>
#include <QWebEnginePage>
#include <QWebEnginePermission>

class EasyWebView : public QWebEngineView
{
    Q_OBJECT

public:
    explicit EasyWebView(QWidget *parent = nullptr);

protected:
    QWebEngineView *createWindow(QWebEnginePage::WebWindowType type) override ;



    void contextMenuEvent(QContextMenuEvent *event) override {  event->ignore(); }

private slots:
    void onLinkHovered(const QString&url) ;

private:
    QUrl m_Url ;
};



#endif // EASYWEBVIEW_H
