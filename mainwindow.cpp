#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fullscrdialog.h"
#include "zint.h"


#include <QDialog>
#include <QDir>
#include <QStandardPaths>
#include <QSettings>
#include <QTextToSpeech>
#include <QImage>
#include <QPixmap>
#include <QDateTime>
#include <QAbstractSocket>
#include <windows.h>
#include <QrCodeGenerator.h>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSqlDatabase>
#include <QSqlError>
#include <string>
#include <QNetworkInterface>
#include <QProgressBar>
#include <QMessageBox>
#include <QMap>

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothLocalDevice>
#include <QLayout>
#include <QVBoxLayout>

#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxchartsheet.h"
#include "xlsxdocument.h"
#include "xlsxrichstring.h"
#include "xlsxworkbook.h"

QString getCPUSerialNumber() {
    int CPUInfo[4] = { -1 };
    __cpuid(CPUInfo, 1);

    // 部分 CPU 的序列号信息可能存储在 EBX、EDX 和 ECX 寄存器中
    // 不同 CPU 存储方式和支持情况不同
    char szCPU[256];
    sprintf_s(szCPU, sizeof(szCPU), "%08X%08X%08X%08X",(DWORD)CPUInfo[0], (DWORD)CPUInfo[1], (DWORD)CPUInfo[2], (DWORD)CPUInfo[3]);
    sprintf_s(szCPU, sizeof(szCPU), "%08X%08X",(DWORD)CPUInfo[3], (DWORD)CPUInfo[0]);
    QString serialNumber = QString::fromLatin1(szCPU);

    {
        int cpu_info[4];
        __cpuid(cpu_info, 0);  // 查询厂商 ID

        // 厂商 ID 存储在 cpu_info[1], cpu_info[3], cpu_info[2] 中
        char vendor_id[13];
        *((int*)vendor_id) = cpu_info[1];       // EBX
        *((int*)(vendor_id + 4)) = cpu_info[3]; // EDX
        *((int*)(vendor_id + 8)) = cpu_info[2]; // ECX
        vendor_id[12] = '\0';                   // 字符串结束符

        qDebug() << "Vendor ID: " << vendor_id;
    }

    return serialNumber;
}


QString getHardDiskSerialNumber() {
    QString serialNumber;
    HANDLE hDevice = CreateFile(
        QString("\\\\.\\PhysicalDrive0").toStdWString().c_str(), // 打开第一个物理磁盘
        0,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
        );
    if (hDevice == INVALID_HANDLE_VALUE) {
        qDebug() << "Failed to open physical drive.";
        return serialNumber;
    }

    STORAGE_PROPERTY_QUERY query;
    DWORD bytesReturned;
    BYTE buffer[1024];

    ZeroMemory(&query, sizeof(query));
    query.PropertyId = StorageDeviceProperty;
    query.QueryType = PropertyStandardQuery;

    if (DeviceIoControl(
            hDevice,
            IOCTL_STORAGE_QUERY_PROPERTY,
            &query,
            sizeof(query),
            buffer,
            sizeof(buffer),
            &bytesReturned,
            NULL
            )) {
        STORAGE_DEVICE_DESCRIPTOR* descriptor = (STORAGE_DEVICE_DESCRIPTOR*)buffer;
        if (descriptor->SerialNumberOffset > 0) {
            char* serial = (char*)(buffer + descriptor->SerialNumberOffset);
            serialNumber = QString::fromLocal8Bit(serial).trimmed();
        }
    } else {
        qDebug() << "Failed to get device property.";
    }

    CloseHandle(hDevice);
    return serialNumber;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/audio.png")) ;
    setIconSize(QSize(32,32)) ;


    QXlsx::Document xlsx;
    xlsx.write("A1", "Hello");  // 写入字符串
    xlsx.write("B1", 42);       // 写入整数
    xlsx.write("C1", QDateTime::currentDateTime()); // 写入日期时间
    xlsx.saveAs("test.xlsx");   // 保存文件[1,5](@ref)。


    QMap<QString,int>TM ;

    TM["韦立峰"]=11120 ;
    TM["15"]=60;
    if(TM.contains("韦立峰"))
        qDebug() << TM["韦立峰"] ;

    qDebug() << TM["国家008"] ;

    connect(ui->pushButton,&QPushButton::clicked,this,[this](){
        bool bLoad = m_pTranslator->load(":/multilangtest_zh_CN") ;
        ui->radioButton_2->setChecked(true);
    }) ;

    ui->pushButtonEnglish->setStyleSheet("QPushButton{border:2px solid rgb(0, 253, 155);border-radius:6px;background-color:red;color:white;}");
    connect(ui->pushButtonEnglish,&QPushButton::clicked,this,[this](){
        bool bLoad = m_pTranslator->load(":/multilangtest_en") ;
        ui->radioButton->setChecked(true);
    }) ;

    // 为所有按钮设置手型光标
    QList<QPushButton*> allButtons = findChildren<QPushButton*>();
    foreach (QPushButton *button, allButtons) {
        button->setCursor(Qt::PointingHandCursor);
    }

    ui->label_4->setText(tr("中央人民广播电台")  + QT_VERSION_STR);

    m_pTestDlg = new TestDialog(this) ;

    ui->radioButton->setChecked(true);

    m_pTray = new QSystemTrayIcon(this) ;
    m_pMenu = new QMenu(this) ;

    pActAbout = new QAction(tr("关于")) ;
    pActExit = new QAction(tr("退出")) ;
    m_pMenu->addAction(pActAbout) ;
    m_pMenu->addAction(pActExit) ;

    connect(pActExit,&QAction::triggered,this,[this](){
        close() ;
    });

    m_pTray->setToolTip(tr("我的托盘测试"));
    m_pTray->setIcon(QIcon(":/audio.png"));

    m_pTray->setContextMenu(m_pMenu);
    m_pTray->show();

    QString qtoxPlist = QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +
                        QDir::separator() + "Library" + QDir::separator() + "LaunchAagents" + QDir::separator() + "multilangtest.plist") ;

    QSettings autoRun(qtoxPlist) ;
    ui->checkBox->setChecked(autoRun.value("RunAtLoad").toBool());

    connect(ui->lineEditQrText,&QLineEdit::textChanged,[=](const QString &strText){
        QrCodeGenerator A ;

        QImage B = A.generateQr(strText) ;
        ui->labelQrCode->setPixmap(QPixmap::fromImage(B)) ;
    });

    ui->lineEditQrText->setText("Hello, 美丽中国-Beautiful World!");

    //create_xml("D:\\xml1.xml") ;
    //create_xml2("D:\\xml2.xml") ;

    {
        const char *lpszText = "860001000003000503844" ;
        struct zint_symbol *symbol = ZBarcode_Create();
        if (symbol != NULL)
        {
            symbol->scale = 4;
            symbol->option_1 = 1; //容错级别
            symbol->option_2 = 1; //版本，决定图片大小
            symbol->symbology = BARCODE_CODE128;
            symbol->output_options = 0;
            symbol->show_hrt = 0; //可显示信息，如果设置为1，则需要设置text值
            //symbol->fgcolor = (char *)"00FF00" ;
            strcpy(symbol->fgcolour , "00FF00") ;
            //symbol->input_mode = UNICODE_MODE;
            strcpy(symbol->outfile,"d:\\1234567.bmp") ;
            int nLen = strlen(lpszText);

            int nRet = ::ZBarcode_Encode(symbol,(const unsigned char *)lpszText,nLen); //编码
            if (nRet == 0)
                nRet = ::ZBarcode_Print(symbol,0); //antate angle 旋转角度

            ::ZBarcode_Delete(symbol);
        }
    }

        connect(&m_Client,&QMqttClient::connected,[=](){
            qDebug()<<"Mqtt Login OK!" ;
            m_Client.subscribe(QMqttTopicFilter("/z03y/wt009/device"),0) ;
        }) ;

        connect(&m_Client,&QMqttClient::messageReceived,[=](const QByteArray &message){
            qDebug().noquote() <<"Mqtt Data:" << message.data();
        }) ;

        connect(ui->pushButtonSend,&QPushButton::clicked,[=](){
            QString strText = ui->lineEditMqText->text() ;
            QString strTopic = ui->lineEditTopic->text() ;

            QJsonObject jData;
            jData["text"] = strText ;
            jData["time"] = time(nullptr) ;
            QJsonDocument jDoc(jData) ;

            // QMqttPublishProperties properties ;
            // properties.setContentType("text/plain");
            // properties.setQos(1);
            // properties.setRetain(false);

            m_Client.publish(QMqttTopicName(strTopic),QByteArray(jDoc.toJson(QJsonDocument::JsonFormat::Compact)),0,false) ;
        }) ;

        QString strHDSN = getHardDiskSerialNumber() ;
        qDebug()<< "HardDisk SerialNumber: " << strHDSN;
        QString strCPUSN = getCPUSerialNumber() ;
        qDebug()<< "CPU SerialNumber: " << strCPUSN;

        m_Client.setHostname("www.zkeagle.com") ;
        m_Client.setPort(1883) ;
        m_Client.setUsername("zkeagle008") ;
        m_Client.setPassword("zkeagle008") ;
        m_Client.connectToHost() ;

        {
            QString pluginsPath = QCoreApplication::applicationDirPath() + "/plugins/sqldrivers";
            QDir pluginsDir(pluginsPath);
            if (!pluginsDir.exists())
            {
                qDebug() << "插件目录不存在: "<<pluginsPath;
            }
            QCoreApplication::addLibraryPath(pluginsPath);

            qDebug()<<"support drivers:"<<QSqlDatabase::drivers();

            QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
            db.setHostName("www.zkeagle.com");
            db.setPort(3306);
            db.setDatabaseName("zkeagledb");
            db.setUserName("zkeagle");
            db.setPassword("12345678");

            if (db.open())
            {
                qDebug()<<"MySQL Connect OK!";
            }
            else
            {
                qDebug()<< db.lastError() ;
            }
        }


        QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

        for (const QNetworkInterface &ACard : interfaces)
        {
            // 检查是否为板载网卡，这里假设板载网卡的名称包含 "eth" 或 "en" （以太网接口）
            qDebug()<<ACard.name() << ACard.hardwareAddress().toLocal8Bit();
            //if (ACard.name().contains("eth") || ACard.name().contains("en"))
            {
                // 获取硬件地址（MAC 地址）
                QNetworkInterface::InterfaceFlags flags = ACard.flags();
                if ((flags & QNetworkInterface::IsUp) &&!(flags & QNetworkInterface::IsLoopBack))
                {
                    QList<QNetworkAddressEntry> entries = ACard.addressEntries();
                    if (!entries.isEmpty())
                    {
                        QNetworkAddressEntry entry = entries.first();
                        QNetworkInterface::InterfaceType type = ACard.type();
                        if (type == QNetworkInterface::Ethernet || type == QNetworkInterface::Virtual)
                        {
                            QByteArray macAddress = ACard.hardwareAddress().toLocal8Bit();
                            qDebug() << "MAC Address of" << ACard.name() << "is:" << macAddress << entry.ip() ;
                        }
                    }
                }
            }
        }

    QBluetoothDeviceDiscoveryAgent *discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);

    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            [=](const QBluetoothDeviceInfo&device){
                qDebug()<< device.deviceUuid() << device.name() << device.address();
    });

    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred,
            this,[](QBluetoothDeviceDiscoveryAgent::Error error){
                qDebug()<< "onDeviceDiscoverError: " << error;
            });

    discoveryAgent->setLowEnergyDiscoveryTimeout(25000);
    discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod) ;

    int status = 0 ;
    QStringList allMsg = {"normal", "dark", "photo", "mask", "badusb"};
    for(QString &strMsg:allMsg)
    {
        if(strMsg.compare("mask",Qt::CaseInsensitive) == 0)
        {
            status = allMsg.indexOf(strMsg) ;
            break;
        }
    }
    qDebug()<<status ;

    m_udpSock = new QUdpSocket(this) ;
    m_udpSock->open(QIODeviceBase::ReadWrite);

    //m_udpSock->abort();
    //m_udpSock->connectToHost("202.120.2.101", 123);
    m_udpSock->connectToHost("c1.aliyun.com", 123);
    //m_udpSock->connectToHost("120.25.115.20", 23);

    {
        QHostAddress destinationAddress("120.25.115.20");
        quint16 destinationPort = 123;
        qint8 LI = 0;
        qint8 VN = 3;
        qint8 MODE = 3;
        qint8 STRATUM = 0;
        qint8 POLL = 4;
        qint8 PREC = -6;
        QDateTime epoch(QDate(1900, 1, 1), QTime(0, 0, 0));
        qint32 second = quint32(epoch.secsTo(QDateTime::currentDateTime()));

        qint32 temp = 0;
        QByteArray timeRequest(48, 0);
        timeRequest[0] = (LI << 6) | (VN << 3) | (MODE);
        timeRequest[1] = STRATUM;
        timeRequest[2] = POLL;
        timeRequest[3] = PREC & 0xff;
        timeRequest[5] = 1;
        timeRequest[9] = 1;
        timeRequest[40] = (temp = (second & 0xff000000) >> 24);
        temp = 0;
        timeRequest[41] = (temp = (second & 0x00ff0000) >> 16);
        temp = 0;
        timeRequest[42] = (temp = (second & 0x0000ff00) >> 8);
        temp = 0;
        timeRequest[43] = ((second & 0x000000ff));

        m_udpSock->writeDatagram(timeRequest, destinationAddress, destinationPort);
    }

    connect(m_udpSock, &QAbstractSocket::connected, this, [this](){

        qint8 LI = 0;
        qint8 VN = 3;
        qint8 MODE = 3;
        qint8 STRATUM = 0;
        qint8 POLL = 4;
        qint8 PREC = -6;
        QDateTime epoch(QDate(1900, 1, 1), QTime(0, 0, 0));
        qint32 second = quint32(epoch.secsTo(QDateTime::currentDateTime()));

        qint32 temp = 0;
        QByteArray timeRequest(48, 0);
        timeRequest[0] = (LI << 6) | (VN << 3) | (MODE);
        timeRequest[1] = STRATUM;
        timeRequest[2] = POLL;
        timeRequest[3] = PREC & 0xff;
        timeRequest[5] = 1;
        timeRequest[9] = 1;
        timeRequest[40] = (temp = (second & 0xff000000) >> 24);
        temp = 0;
        timeRequest[41] = (temp = (second & 0x00ff0000) >> 16);
        temp = 0;
        timeRequest[42] = (temp = (second & 0x0000ff00) >> 8);
        temp = 0;
        timeRequest[43] = ((second & 0x000000ff));

        m_udpSock->write(timeRequest);
        qDebug()<< "UDP Connencted !!!" ;
    });

    connect(m_udpSock, &QUdpSocket::readyRead, this, [this](){
        QByteArray newTime;
        QDateTime epoch(QDate(1900, 1, 1), QTime(0, 0, 0));
        QDateTime unixStart(QDate(1970, 1, 1), QTime(0, 0, 0));

        while (m_udpSock->hasPendingDatagrams()) {
            newTime.resize(m_udpSock->pendingDatagramSize());
            m_udpSock->read(newTime.data(), newTime.size());
        };

        QByteArray transmitTimeStamp ;
        transmitTimeStamp = newTime.right(8);
        quint32 seconds = transmitTimeStamp.at(0);
        quint8 temp = 0;

        for (int i = 1; i <= 3; i++) {
            seconds = (seconds << 8);
            temp = transmitTimeStamp.at(i);
            seconds = seconds + temp;
        }

        QDateTime dateTime;
        dateTime.setSecsSinceEpoch(seconds - epoch.secsTo(unixStart));

        qDebug() << dateTime.toString() ;

#ifdef __arm__
#ifdef arma9
        dateTime = dateTime.addSecs(60 * 60 * 8);
#endif
#endif
        m_udpSock->disconnectFromHost();

        //有些时候返回的数据可能有误或者解析不正确,导致填充的时间不正确
        if (dateTime.isValid()) {
            //emit receiveTime(dateTime);
        }
    });

    QVBoxLayout *pVLay= new QVBoxLayout(this) ;
    ui->scrollAreaWidgetContents->setLayout(pVLay) ;
//

    QCursor A(Qt::PointingHandCursor);
    for (int i = 0; i < 80; ++i) {
        QPushButton *button = new QPushButton(QString("Button %1").arg(i + 1),this);
        button->setMaximumWidth(200) ;
        button->setMinimumHeight(30) ;
        // button->setFixedSize(150,25) ;
        button->setCursor(A) ;

        ui->scrollAreaWidgetContents->layout()->addWidget(button);
    }

    ui->progressBar_1->setStyleSheet("QProgressBar{text-align:center;background-color:#DDDDDD;border: 1px solid #ff0000;}"
                                   "QProgressBar::chunk{background-color:#05B8CC;}");

    //文字居中 text-align:center;
    //设置圆角 border-radius 这里要注意如果设置的圆角大于或等进度条高度的一半，圆角设置无效， 比如进度条的高度为10， 圆然设置为5px或以上就不会起作用
    ui->progressBar_2->setStyleSheet("QProgressBar{text-align:center;background-color:#DD55DD;border: 0px solid #DDDDDD;border-radius:5px;}"
                                     "QProgressBar::chunk{background-color:#05B8CC; border-radius: 5px;}");

    //块显示
    ui->progressBar_3->setStyleSheet("QProgressBar{text-align:center;background-color:#33DDDD;border: 0px solid #DDDDDD;border-radius:5px;}"
                                     "QProgressBar::chunk{background-color:#05B8CC;border-radius:5px; width:8px;margin:0.5px;}");

    //进度块渐变
    ui->progressBar_4->setStyleSheet("QProgressBar{height:22px; text-align:center; font-size:14px; color:white; border-radius:11px; background:#1D5573;}"
                                     "QProgressBar::chunk{border-radius:11px;background:qlineargradient(spread:pad,x1:0,y1:0,x2:1,y2:0,stop:0 #99ffff,stop:1 #9900ff);}");

    //设置进度值
    ui->progressBar_1->setValue(80);
    ui->progressBar_2->setValue(70);
    ui->progressBar_3->setValue(60);
    ui->progressBar_4->setValue(50);
    //隐藏百分百文字
    ui->progressBar_4->setTextVisible(false);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *pEvt)
{
    if(pEvt->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);

        ui->label_4->setText(tr("中央人民广播电台"));
        m_pTray->setToolTip(tr("我的托盘测试"));
        pActAbout->setText(tr("关于")) ;
        pActExit->setText(tr("退出")) ;
    }

    QWidget::changeEvent(pEvt) ;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    qDebug()<<"keyPress: " << event->key();
    QWidget::keyPressEvent(event) ;
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    qDebug()<<"KeyRelease: " << event->key();
    QWidget::keyReleaseEvent(event) ;
}

void MainWindow::on_pushButton_2_clicked()
{
   int nRet = m_pTestDlg->exec() ;

   ui->label_5->setText(nRet == QDialog::Accepted ? tr("点击了 确定 按钮"): tr("点击了 取消 按钮"));
}


void MainWindow::on_radioButton_clicked()
{
    m_pTranslator->load(":/multilangtest_en") ;
}


void MainWindow::on_radioButton_2_clicked()
{
    m_pTranslator->load(":/multilangtest_zh_CN") ;
}


void MainWindow::on_checkBox_clicked(bool checked)
{
    QString qtoxPlist = QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+
                    QDir::separator() +"Library" + QDir::separator() + "LaunchAagents" + QDir::separator() + "multilangtest.plist") ;
    QString qtoxDir = QDir::cleanPath(QCoreApplication::applicationFilePath()) ;

    QString qtWork = QDir::cleanPath(QCoreApplication::applicationDirPath())  ;
    qDebug()<< qtoxPlist;
    qDebug()<< qtoxDir;
    qDebug()<< qtWork;

    QSettings autoRun(qtoxPlist) ;
    autoRun.setValue("Label","multilangtest.app");
    autoRun.setValue("Program",qtoxDir);
    autoRun.setValue("WorkingDirectory",qtWork);
    autoRun.setValue("Disabled",false);
    autoRun.setValue("RunAtLoad",checked);
}


void MainWindow::on_pushButtonFull_clicked()
{
    FullScrDialog dlg ;
    dlg.exec() ;
}
/*
void create_xml(const char* xmlPath)
{
    std::cout << "\ncreate_xml:" << xmlPath << std::endl;
    //【】构造一个xml文档类
    XMLDocument doc;
    //【】操作文档相关接口
    //创建与此文档关联的新声明。对象的内存由文档管理。
    //如果'text'参数为null，则使用标准声明:
    //<?xml version="1.0" encoding="UTF-8"?>
    XMLDeclaration* declaration = doc.NewDeclaration();
    //创建与此文档关联的新元素。元素的内存由文档管理。
    XMLElement* element = doc.NewElement("ElementA");
    //创建与此文档关联的新注释。注释的内存由文档管理。
    XMLComment* comment = doc.NewComment("My TinyXml2 Test... ...");
    //创建与此文档关联的新文本。文本的存储由文档管理。
    XMLText* text = doc.NewText("Some Test");
    //创建与此文档关联的新的未知节点。对象的内存由文档管理。
    XMLUnknown* unknown = doc.NewUnknown("Unknown");
    //【】创建了节点还要插入文档中
    //添加一个子节点作为最后一个（右）子节点。如果子节点已经是文档的一部分，则将其从其旧位置移至新位置。
    //XMLNode* tinyxml2::XMLNode::InsertEndChild(XMLNode * addThis);
    //添加一个子节点作为第一个（左）子节点。如果子节点已经是文档的一部分，则将其从其旧位置移至新位置。
    //XMLNode* tinyxml2::XMLNode::InsertFirstChild(XMLNode* addThis);
    //在指定的子节点之后添加一个节点。如果子节点已经是文档的一部分，则将其从其旧位置移至新位置。
    //XMLNode* tinyxml2::XMLNode::InsertAfterChild(XMLNode * afterThis, XMLNode * addThis);
    doc.InsertFirstChild(declaration);
    doc.InsertAfterChild(declaration, element);
    doc.InsertEndChild(comment);
    doc.InsertEndChild(text);
    doc.InsertEndChild(unknown);

    //【】操作节点相关接口
    //将命名属性设置为对应类型的value
    element->SetAttribute("Level", "A");
    element->SetAttribute("Value", 1992);
    //设置文本
    element->SetText("ElementA text");
    //注释
    element->InsertNewComment("ElementA comment");
    //添加子节点，接口已经带insert功能了
    XMLElement* sub_element = element->InsertNewChildElement("ElementB");
    sub_element->SetAttribute("Level", "B");
    sub_element->SetText("ElementB text");
    sub_element->InsertNewComment("ElementB comment");


    //【】存储到文件，参数2 compact紧凑默认false
    //结构写的不规范，感觉应该拿一个根节点把那些注释，文本包起来
    //不然解析的时候没法访问啊
    doc.SaveFile(xmlPath);
}

void parse_xml(const char* xmlPath)
{
    std::cout << "\nparse_xml:" << xmlPath << std::endl;
    //【】构造一个xml文档类
    XMLDocument doc;
    //【】读取文件
    //从磁盘加载XML文件。成功返回XML_SUCCESS（0），或者返回errorID。
    XMLError error = doc.LoadFile(xmlPath);
    //也可以解析字符串
    //从字符串解析XML文件。成功返回XML_SUCCESS（0），或者返回errorID。
    //XMLError tinyxml2::XMLDocument::Parse(const char *xml,size_t nBytes = static_cast<size_t>(-1));
    if (error != XMLError::XML_SUCCESS)
        return;
    //注意，实际解析时返回的指针记得判空，不然遇到解析失败异常就遭了

    //【】解析根元素
    //返回DOM的根元素。等效于FirstChildElement。要获取第一个节点，请使用FirstChild。
    XMLElement* root = doc.RootElement();
    std::cout << "RootElement name:" << root->Name() << std::endl;
    //获取第一个子元素，或者选择具有指定名称的第一个子元素。
    XMLElement* first = doc.FirstChildElement("ElementA");
    //给定一个属性名称，Attribute返回该名称的属性的值；如果不存在，则返回null。
    std::cout << "FirstChildElement Attr Level:" << first->Attribute("Level") << std::endl;
    std::cout << "FirstChildElement Attr Value:" << first->Attribute("Value") << std::endl;
    //如果'this'的第一个Child是XMLText，则GetText返回Text节点的字符串，否则返回null。
    std::cout << "FirstChildElement Text:" << first->GetText() << std::endl;

    //【】解析子元素
    XMLElement* sub = root->FirstChildElement("ElementB");
    std::cout << "SubElement Attr Level:" << sub->Attribute("Level") << std::endl;
    std::cout << "SubElement Text:" << sub->GetText() << std::endl;

    //【】
    //可使用FirstChild+NextSibling遍历子节点
    std::cout << "\ntraversal_xml:" << std::endl;
    traversal_node(&doc);
    //或者FirstChildElement+NextSiblingElement遍历子元素
    std::cout << "\ntraversal_element:" << std::endl;
    traversal_element(&doc, 0);
}

void traversal_node(XMLNode* node)
{
    if (!node)
        return;
    for (XMLNode* current = node->FirstChild(); current != nullptr; current = current->NextSibling())
    {
        XMLNode* temp = current;
        if (temp->Value() != nullptr)
            std::cout << temp->Value() << std::endl;
        if (!temp->NoChildren())
            traversal_node(temp);
    }
}

void traversal_element(XMLNode* node, int level)
{
    if (!node)
        return;
    for (XMLElement* current = node->FirstChildElement(); current != nullptr; current = current->NextSiblingElement())
    {
        XMLElement* temp = current;
        //这里我使用点号来表示层级缩进
        std::cout << std::string(level * 5, '.') << temp->Name() << std::endl;
        if (temp->GetText() != nullptr)
            std::cout << std::string(level * 5, '.') << "> Text" << ":" << temp->GetText() << std::endl;
        const XMLAttribute* attr = temp->FirstAttribute();
        if (attr != nullptr)
            std::cout << std::string(level * 5, '.') << "> Attr" << ":" << attr->Value() << std::endl;
        if (temp->FirstChildElement() != nullptr)
            traversal_element(temp, level + 1);
    }
}

void create_xml2(const char* xmlPath)
{
    std::cout << "\ncreate_xml2:" << xmlPath << std::endl;
    //【】构造一个xml文档类
    XMLDocument doc;

    //【】构建我们的xml数据结构
    XMLDeclaration* declaration = doc.NewDeclaration();
    doc.InsertFirstChild(declaration);
    //创建与此文档关联的新元素。元素的内存由文档管理。
    XMLElement* root = doc.NewElement("Root");
    doc.InsertEndChild(root);
    //子节点
    XMLElement* group_a = root->InsertNewChildElement("GroupA");
    group_a->SetAttribute("Type", "A");
    XMLElement* a_1 = group_a->InsertNewChildElement("Name");
    a_1->SetText("中文");
    XMLElement* a_1_sub = a_1->InsertNewChildElement("Content");
    a_1_sub->SetText("111111");
    XMLElement* a_2 = group_a->InsertNewChildElement("Name");
    a_2->SetText("English");
    XMLElement* a_2_sub = a_2->InsertNewChildElement("Content");
    a_2_sub->SetText("222222");
    XMLElement* a_3 = group_a->InsertNewChildElement("Name");
    a_3->SetText("123");
    XMLElement* a_3_sub = a_3->InsertNewChildElement("Content");
    a_3_sub->SetText("333333");
    XMLElement* group_b = root->InsertNewChildElement("GroupB");
    group_b->SetAttribute("Type", "B");
    XMLElement* group_c = root->InsertNewChildElement("GroupC");
    group_c->SetAttribute("Type", "C");


    //【】存储到文件，参数2 compact紧凑默认false
    doc.SaveFile(xmlPath);
}

void parse_xml2(const char* xmlPath)
{
    std::cout << "\nparse_xml2:" << xmlPath << std::endl;
    //【】构造一个xml文档类
    XMLDocument doc;
    //【】读取文件
    //从磁盘加载XML文件。成功返回XML_SUCCESS（0），或者返回errorID。
    XMLError error = doc.LoadFile(xmlPath);
    //也可以解析字符串
    //从字符串解析XML文件。成功返回XML_SUCCESS（0），或者返回errorID。
    //XMLError tinyxml2::XMLDocument::Parse(const char *xml,size_t nBytes = static_cast<size_t>(-1));
    if (error != XMLError::XML_SUCCESS)
        return;
    //注意，实际解析时返回的指针记得判空，不然遇到解析失败异常就遭了

    //【】解析根节点
    //返回DOM的根元素。等效于FirstChildElement。要获取第一个节点，请使用FirstChild。
    XMLElement* root = doc.RootElement();
    //【】查找
    XMLElement* find_ele = root->FirstChildElement("GroupA");
    if (find_ele) {
        std::cout << find_ele->Name() << std::endl;
        const XMLAttribute* attr = find_ele->FindAttribute("Type");
        if (attr) {
            std::cout << attr->Name() << ":" << attr->Value() << std::endl;
        }
        XMLElement* find_sub = find_ele->FirstChildElement("Name");
        if (find_sub && find_sub->GetText()) {
            std::cout << find_sub->GetText() << std::endl;
        }
    }

    //【】遍历
    std::cout << "\ntraversal_element:" << std::endl;
    traversal_element(&doc, 0);
}*/

void MainWindow::on_fontComboBox_currentFontChanged(const QFont &f)
{
    qDebug()<< f ;
    ui->label_6->setFont(f);
}


void MainWindow::on_pushButton_3_clicked()
{
    QMessageBox box;
    //设置文本框的大小
    box.setStyleSheet("QLabel#qt_msgbox_label{"
                      "min-width: 400px;"
                      "min-height: 100px; "
                      "font-size:14px;"
                      "qproperty-alignment: AlignLeft;"
                      "}");
    box.setText("<font size='13' color='blue'>这是一个QMessageBox</font>");
    box.setWindowTitle("关于");
    box.setIcon(QMessageBox::Information);
    box.exec();
}

