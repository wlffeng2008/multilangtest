#include "DialogWebCef.h"
#include "ui_DialogWebCef.h"

#include <QCefView.h>
#include <CefViewWidget.h>
#include <QCefSetting.h>

DialogWebCef::DialogWebCef(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogWebCef)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint| Qt::WindowMinimizeButtonHint);

    static QCefSetting setting;
    setting.setHardwareAccelerationEnabled(true);
    setting.setWindowlessFrameRate(240);
    //setting.setJavascript(true);

    QStringList Urls={
        "https://www.163.com",
        "https://hub.akkogear.com/",
        "https://www.baidu.com",
        "https://www.zkeagle.com",
        "https://demo.hubx.pro/keyboard/",
        "https://www.chinadaily.com.cn",
        "https://localhost/phpMyAdmin",
        "http://localhost/by",
        "http://localhost:5173/",
        "http://localhost/hello.html",
        "http://localhost/by",
        "file:///C:/vcpkg/hello.html"
    };

    ui->comboBox->addItems(Urls);

    // Create the QCefView widget  file:///C:/vcpkg/hello.html  https://demo.hubx.pro/keyboard/  https://www.163.com
    auto qCefView = new CefViewWidget("https://web.akkogear.com", &setting, this);
    ui->verticalLayout->addWidget(qCefView);
    m_pView = qCefView;

    connect(ui->pushButton_1,&QPushButton::clicked,this,[=]{
        m_pView->browserGoBack();
    });
    connect(ui->pushButton_2,&QPushButton::clicked,this,[=]{
        m_pView->browserGoForward();
    });
    connect(ui->pushButton_3,&QPushButton::clicked,this,[=]{
        m_pView->browserReload();
    });

    connect(ui->pushButton_4,&QPushButton::clicked,this,[=]{
        QString strUrl = ui->comboBox->currentText().trimmed();
        m_pView->navigateToUrl(strUrl);
    });

    connect(ui->comboBox,&QComboBox::activated,this,[=](int index){
        ui->pushButton_4->click();
    });

    connect(ui->pushButton_5,&QPushButton::clicked,this,[=]{
        QString strNow = QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss");
        QString strFile= QString("%1WebSnap-%2.png").arg(QApplication::applicationDirPath() + "/WebSnap/",strNow);
        //m_pView->children()[0]->
        grab().save(strFile);
    });

    resize(1600,1050);
}

DialogWebCef::~DialogWebCef()
{
    delete ui;
}
