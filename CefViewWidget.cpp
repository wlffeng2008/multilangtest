#include "CefViewWidget.h"

CefViewWidget::CefViewWidget(const QString &url, const QCefSetting *setting, QWidget *parent, Qt::WindowFlags f):
    QCefView(url, setting, parent, f)
{}


bool CefViewWidget::onNewPopup(const QCefFrameId& frameId,
                const QString& targetUrl,
                QString& targetFrameName,
                QCefView::CefWindowOpenDisposition targetDisposition,
                QRect& rect,
                QCefSetting& settings,
                bool& disableJavascriptAccess)
{
    // 核心逻辑：直接让当前的 QCefView 实例跳转到目标 URL
    // 这样就把“新窗口”的请求变成了“当前窗口”的导航
    this->navigateToUrl(targetUrl);
    qDebug() << targetUrl;

    // 返回 true 表示我们已经处理了这个请求，阻止 CEF 创建默认的弹出窗口
    return true;
}

QCefView* CefViewWidget::onNewBrowser(const QCefFrameId& sourceFrameId,
                       const QString& url,
                       const QString& name,
                       QCefView::CefWindowOpenDisposition targetDisposition,
                       QRect& rect,
                       QCefSetting& settings)
{
    this->navigateToUrl(url);

    qDebug() << url;
    return this;
}