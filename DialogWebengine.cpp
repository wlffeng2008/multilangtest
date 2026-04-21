#include "DialogWebengine.h"
#include "ui_DialogWebengine.h"

#include <QTimer>
#include <QWebEnginePage>
#include <QWebEngineSettings>
#include <QWebEngineView>
#include <QWebEngineProfile>

DialogWebengine::DialogWebengine(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogWebengine)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint| Qt::WindowMinimizeButtonHint);

    QStringList Urls={
        "http://localhost:5173/",
        "https://www.163.com",
        "https://www.baidu.com",
        "https://www.zkeagle.com",
        "https://demo.hubx.pro/keyboard/",
        "https://www.chinadaily.com.cn",
        "https://localhost/by",
        "https://localhost/phpMyAdmin",
        "http://localhost/hello.html",
        "file:///C:/vcpkg/hello.html"
    };

    ui->comboBox->addItems(Urls);

    auto qView = new EasyWebView(this);
    ui->verticalLayout->addWidget(qView);
    m_pView = qView;
    qView->show();
    //qView->resize(1280,800);

    connect(qView->page(), &QWebEnginePage::featurePermissionRequested,this,[=](const QUrl &securityOrigin, QWebEnginePage::Feature feature)
            {
                qView->page()->setFeaturePermission(securityOrigin,feature,QWebEnginePage::PermissionGrantedByUser);
            });

    auto settings = qView->page()->settings();

    //settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    settings->setAttribute(QWebEngineSettings::AllowRunningInsecureContent, true);

    connect(ui->pushButton_1,&QPushButton::clicked,this,[=]{
        m_pView->back();
    });
    connect(ui->pushButton_2,&QPushButton::clicked,this,[=]{
        m_pView->forward();
    });
    connect(ui->pushButton_3,&QPushButton::clicked,this,[=]{
        m_pView->reload();
    });

    connect(ui->pushButton_4,&QPushButton::clicked,this,[=]{
        QString strUrl = ui->comboBox->currentText().trimmed();
        m_pView->setUrl(strUrl);
    });
    connect(ui->pushButton_5,&QPushButton::clicked,this,[=]{
        QString strNow = QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss");
        QString strFile=QString("%1WebSnap-%2.png").arg(QApplication::applicationDirPath() + "/WebSnap/",strNow);
        m_pView->grab().save(strFile);
    });


    connect(ui->comboBox,&QComboBox::activated,this,[=](int index){
        ui->pushButton_4->click();
    });

    QTimer::singleShot(10,this,[=]{
    });
    ui->comboBox->activated(0);
    resize(1280,850);
}

DialogWebengine::~DialogWebengine()
{
    delete ui;
}

void DialogWebengine::showEvent(QShowEvent *pEv)
{
    if(m_pView)
        m_pView->update();
    update();
}

