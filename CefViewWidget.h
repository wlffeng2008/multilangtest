#ifndef CEFVIEWWIDGET_H
#define CEFVIEWWIDGET_H

#include <QCefView.h>

class CefViewWidget : public QCefView
{
    Q_OBJECT
public:
    CefViewWidget(const QString& url,
                  const QCefSetting* setting,
                  QWidget* parent = nullptr,
                  Qt::WindowFlags f = Qt::WindowFlags());

protected:
    bool onNewPopup(const QCefFrameId& frameId,
                    const QString& targetUrl,
                    QString& targetFrameName,
                    QCefView::CefWindowOpenDisposition targetDisposition,
                    QRect& rect,
                    QCefSetting& settings,
                    bool& disableJavascriptAccess) final;

    QCefView* onNewBrowser(const QCefFrameId& sourceFrameId,
                           const QString& url,
                           const QString& name,
                           QCefView::CefWindowOpenDisposition targetDisposition,
                           QRect& rect,
                           QCefSetting& settings) final;

};

#endif // CEFVIEWWIDGET_H
