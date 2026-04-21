#include "easywebview.h"

EasyWebView::EasyWebView(QWidget *parent): QWebEngineView(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    connect(this->page(),&QWebEnginePage::linkHovered,this,&EasyWebView::onLinkHovered) ;
}

QWebEngineView *EasyWebView::createWindow(QWebEnginePage::WebWindowType type)
{
    this->load(m_Url);
    return this ;
}


void EasyWebView::onLinkHovered(const QString&url)
{
    m_Url.setUrl(url);
}
