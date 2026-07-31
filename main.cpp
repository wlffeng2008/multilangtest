#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDebug>
#include <QStandardPaths>

#include <QWebEngineProfile>
#include <QWebEngineSettings>

#include <QCefView.h>
#include <QCefConfig.h>
#include <QCefContext.h>
#include <QMessageLogContext>


// 全局保存原始消息处理函数指针
static QtMessageHandler g_originalHandler = nullptr;

// 自定义过滤处理器
void customLogHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // 过滤GCM废弃端点报错
    if (msg.contains("google_apis\\gcm") && msg.contains("DEPRECATED_ENDPOINT"))
    {
        return; // 直接丢弃这条日志，不输出
    }

    if(msg.startsWith("js:"))
        return;

    // 其他日志交给原始处理器输出
    if (g_originalHandler)
    {
        g_originalHandler(type, context, msg);
    }
}

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);


    // 先保存系统默认日志处理器
    g_originalHandler = qInstallMessageHandler(customLogHandler);


    // 设置中文语言环境
    QLocale::setDefault( QLocale(QLocale::Chinese, QLocale::China) );
    // 加载 Qt 翻译文件 - 用于自动翻译标准按钮文字
    QTranslator translatorA;
    QTranslator translatorB;
    if( translatorA.load( "qt_zh_CN", QLibraryInfo::path( QLibraryInfo::TranslationsPath ) ) )
    {
        a.installTranslator( &translatorA );
    }
    if( translatorB.load( "qtbase_zh_CN", QLibraryInfo::path( QLibraryInfo::TranslationsPath ) ) )
    {
        a.installTranslator( &translatorB );
    }

    QTranslator translatorC;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        QString baseName = "multilangtest_" + QLocale(locale).name();
        qDebug() << baseName;
        //baseName = "multilangtest_en";
        if (translatorC.load(":/" + baseName)) {
            a.installTranslator(&translatorC);
            break;
        }
    }
   // a.setStyleSheet("QPushButton:hover{cursor: pointer;}");

    // build QCefConfig
    QCefConfig config;
    // set user agent
    // config.setUserAgent("QCefViewTest");
    // set log level
    //config.setLogLevel(QCefConfig::LOGSEVERITY_DEFAULT);
    // set JSBridge object name (default value is CefViewClient)
    config.setBridgeObjectName("CallBridge");
    // set Built-in scheme name (default value is CefView)
    config.setBuiltinSchemeName("CefView");
    // port for remote debugging (default is 0 and means to disable remote debugging)
    config.setRemoteDebuggingPort(9000);
    // set background color for all browsers
    // (QCefSetting.setBackgroundColor will overwrite this value for specified browser instance)
    // config.setBackgroundColor(Qt::lightGray);

    // windowlessRenderingEnabled is set to true by default,
    // set to false to disable the OSR mode
    config.setWindowlessRenderingEnabled(true);
    config.setStandaloneMessageLoopEnabled(true);

    // disable sandbox
    // this is a bit complicated, please refer to:
    // https://developer.apple.com/documentation/xcode/configuring-the-macos-app-sandbox
    config.setSandboxDisabled(true);

    // add command line args, you can any cef supported switches or parameters
    config.addCommandLineSwitch("use-mock-keychain");
    config.addCommandLineSwitch("enable-aggressive-domstorage-flushing");
    // allow remote debugging
    config.addCommandLineSwitchWithValue("remote-allow-origins", "*");

    config.addCommandLineSwitch("enable-experimental-web-platform-features");

    // set cache folder
    config.setCachePath(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    config.addCommandLineSwitch("enable-proprietary-codecs");  // 必须！播放MP4
    config.addCommandLineSwitch("autoplay-policy=no-user-gesture-required"); // 视频自动播放

    config.addCommandLineSwitch("enable-features=WebHID");

    config.addCommandLineSwitch("enable-media-stream");
    config.addCommandLineSwitch("enable-hardware-acceleration");
    config.addCommandLineSwitch("enable-media-decoding");
    config.addCommandLineSwitch("disable-web-security");

    QCefContext cefContext(&a, argc, argv, &config);


    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    //QWebEngineSettings::setAttribute(QWebEngineSettings::AllowRunningInsecureContent, true);

    MainWindow w;
    w.setTranslater(&translatorC);
    w.show();
    return a.exec();
}
