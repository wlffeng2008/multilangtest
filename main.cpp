#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


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

    MainWindow w;
    w.setTranslater(&translatorC);
    w.show();
    return a.exec();
}
