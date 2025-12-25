#include "testdialog.h"
#include "ui_testdialog.h"
#include <stdio.h>

#include <QDebug>
#include <QLoggingCategory>

#include <QMediaPlayer>
#include <QAudioOutput>
#include <QTimer>
#include <QStringListModel>
#include <QStandardPaths>
#include <QDirIterator>
#include <QCheckBox>

QMediaPlayer *m_pPlayer = nullptr;
QAudioOutput *m_pAudOut = nullptr;

TestDialog::TestDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TestDialog)
{
    ui->setupUi(this);

    QLoggingCategory::setFilterRules(QStringLiteral("qt.speech.tts=true \n qt.speech.tts.*=true"));

    // Populate engine selection list
    ui->engine->addItem("Default", QString("default"));
    foreach (QString engine, QTextToSpeech::availableEngines())
        ui->engine->addItem(engine, engine);
    ui->engine->setCurrentIndex(0);
    engineSelected(0);
    setFixedSize(600,620);

    connect(ui->speakButton, &QPushButton::clicked, this, &TestDialog::speak);
    connect(ui->pauseButton, &QPushButton::clicked, this, &TestDialog::pause);
    connect(ui->resumeButton, &QPushButton::clicked, this, &TestDialog::resume);
    connect(ui->stopButton, &QPushButton::clicked, this, &TestDialog::stop);

    connect(ui->pitch, &QSlider::valueChanged, this, &TestDialog::setPitch);
    connect(ui->rate, &QSlider::valueChanged, this, &TestDialog::setRate);
    connect(ui->volume, &QSlider::valueChanged, this, &TestDialog::setVolume);
    connect(ui->engine, &QComboBox::currentIndexChanged, this, &TestDialog::engineSelected);

    ui->volume->setValue(100);
    ui->rate->setValue(50);
    ui->pitch->setValue(50);
    ui->plainTextEdit->appendPlainText("经测试，可以正常的播报中文和英文，并且该示例包含了所有的接口调用，包括音量、速率、引擎选择、语音选择、地区选择等等，非常全面，只是暂停功能会随着平台的不同而有着不同的表现，上面的接口说明已经提到过了。");

    m_pPlayer = new QMediaPlayer(this);
    m_pAudOut = new QAudioOutput(this);
    m_pPlayer->setAudioOutput(m_pAudOut);
    m_pAudOut->setVolume(0.3);
    ui->horizontalSliderVol->setValue(30);
    ui->checkBox->setChecked(true);
    connect(ui->musicButton,&QPushButton::clicked,this,[=](){
        if(m_bPause)
            m_pPlayer->play();
        else
            playFile(m_strSelFile);
    });

    connect(m_pPlayer,&QMediaPlayer::durationChanged,[=](qint64 nDur){
        ui->horizontalSliderDur->setRange(0,nDur);
        m_nDur = nDur ;
    });

    connect(m_pPlayer,&QMediaPlayer::playbackStateChanged,[=](QMediaPlayer::PlaybackState newState){
        qDebug()<< newState ;
    });

    connect(m_pPlayer,&QMediaPlayer::mediaStatusChanged,[=](QMediaPlayer::MediaStatus status){
        qDebug()<< status ;
        if(QMediaPlayer::EndOfMedia == status && ui->checkBox->checkState() == Qt::Checked)
        {
            playNext() ;
        }
    });

    connect(ui->btnStop,&QPushButton::clicked,[=](){
        m_pPlayer->stop();
        ui->labelInfo->setText("00:00 / 00:00");
    });

    connect(m_pPlayer,&QMediaPlayer::positionChanged,[=](qint64 nPos){
        ui->horizontalSliderDur->setValue(nPos);
        ui->horizontalSliderDur->setSliderPosition(nPos);

        QString strInfo ;
        char szOut[1024]={0} ;
        int nSecs = m_nDur/1000 ;
        int nRuns = nPos/1000 ;
        int nH1 = nRuns/3600 ;
        int nM1 = (nRuns%3600)/60 ;
        int nS1 = nRuns%60 ;

        int nH2 = nSecs/3600 ;
        int nM2 = (nSecs%3600)/60 ;
        int nS2 = nSecs%60 ;
        if(nSecs>3600)
        {
            sprintf(szOut,"%02d:%02d:%02d / %02d:%02d:%02d",nH1,nM1,nS1,nH2,nM2,nS2) ;
            strInfo = szOut;
        }
        else
        {
            sprintf(szOut,"%02d:%02d / %02d:%02d",nM1,nS1,nM2,nS2) ;
            strInfo = szOut;
        }
        ui->labelInfo->setText(strInfo);
    });

    connect(ui->btnPause,&QPushButton::clicked,[=](){
        if(m_pPlayer->isPlaying())
            m_pPlayer->pause();
        else
            m_pPlayer->play();
    });

    connect(ui->btnPrve,&QPushButton::clicked,[=](){
        playNext(-1) ;
    });

    connect(ui->btnNext,&QPushButton::clicked,[=](){
        playNext(1) ;
    });

    connect(ui->horizontalSliderDur,&QSlider::sliderMoved,[=](int nPos){
        m_pPlayer->setPosition(nPos);
    });

    connect(ui->horizontalSliderDur,&QSlider::sliderPressed,[=](){
        m_pPlayer->setPosition(ui->horizontalSliderDur->sliderPosition());
    });

    connect(ui->horizontalSliderVol,&QSlider::sliderMoved,[=](int nPos){ m_pAudOut->setVolume(nPos/100.0); });

    QStringList strFolders = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation) ;
    QString strDLFolder = strFolders[0];
    qDebug()<< strDLFolder;
    QStringList  listItems;

    QDirIterator it(strDLFolder);
    while(it.hasNext())
    {
        QString strFile = it.next() ;
        QString strExt = strFile.mid(strFile.length()-4).toLower() ;
        if(strExt == ".wav" || strExt == ".mp3" || strExt == "flac")
            listItems<<strFile ;
    }

    {
        QDirIterator it("D:\\MyMedia\\HDMusic");
        while(it.hasNext())
        {
            QString strFile = it.next() ;
            QString strExt = strFile.mid(strFile.length()-4).toLower() ;

            if(strExt == ".wav" || strExt == ".mp3" || strExt == "flac")
                listItems<<strFile ;
        }
    }

    QStringListModel *listModel = new QStringListModel(listItems,this) ;
    ui->listView->setModel(listModel);

    connect(ui->listView,&QListView::clicked,[=](const QModelIndex&index){
        m_nCurItem = index;
        m_strSelFile = index.data().toString() ;
    });

    connect(ui->listView,&QListView::doubleClicked,[=](const QModelIndex&index){
        m_nCurItem = index;
        QString strFile = index.data().toString() ;
        playFile(strFile) ;
    });

    //listModel->removeRow(3);

    //listModel->insertRow(-1,new QModelIndex());
    QModelIndex *item = new QModelIndex() ;
    //item->
    //listModel->insertRow(-1,) ;
}

void TestDialog::playNext(int nOffset)
{
    QModelIndex Next = m_nCurItem.sibling(m_nCurItem.row()+nOffset,0) ;
    if(!Next.isValid())
    {
        if(nOffset == 1)
            Next = m_nCurItem.sibling(0,0) ;
        else
            Next = m_nCurItem.sibling(0,0) ;
    }

    m_nCurItem = Next ;
    m_strSelFile = Next.data().toString() ;
    playFile(m_strSelFile);
}

void TestDialog::playFile(const QString&strFile)
{
    m_bPause = false ;
    m_pPlayer->stop();
    m_pPlayer->setSource(QUrl::fromLocalFile(strFile));
    m_pPlayer->play();
}

TestDialog::~TestDialog()
{
    delete ui;
}

void TestDialog::on_pushButtonOK_clicked()
{
    accept();
}


void TestDialog::on_pushButtonCancel_clicked()
{
    reject();
}

void TestDialog::changeEvent(QEvent *pEvt)
{
    if(pEvt->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QDialog::changeEvent(pEvt) ;
}

void TestDialog::speak()
{
    m_speech->say(ui->plainTextEdit->toPlainText());
}
void TestDialog::pause()
{
    m_speech->pause();
}
void TestDialog::resume()
{
    m_speech->resume();
}
void TestDialog::stop()
{
    m_speech->stop();
}

void TestDialog::setRate(int rate)
{
    m_speech->setRate(rate / 10.0);
}

void TestDialog::setPitch(int pitch)
{
    m_speech->setPitch(pitch / 10.0);
}

void TestDialog::setVolume(int volume)
{
    m_speech->setVolume(volume / 100.0);
}

void TestDialog::stateChanged(QTextToSpeech::State state)
{
    if (state == QTextToSpeech::Speaking) {
        qDebug()<<("Speech started...");
    } else if (state == QTextToSpeech::Ready)
        qDebug()<<("Speech stopped...");
    else if (state == QTextToSpeech::Paused)
        qDebug()<<("Speech paused...");
    else
        qDebug()<<("Speech error!");

    ui->pauseButton->setEnabled(state == QTextToSpeech::Speaking);
    ui->resumeButton->setEnabled(state == QTextToSpeech::Paused);
    ui->stopButton->setEnabled(state == QTextToSpeech::Speaking || state == QTextToSpeech::Paused);
}

void TestDialog::engineSelected(int index)
{
    QString engineName = ui->engine->itemData(index).toString();
    if(m_speech)
    delete m_speech;
    if (engineName == "default")
        m_speech = new QTextToSpeech(this);
    else
        m_speech = new QTextToSpeech(engineName, this);
    disconnect(ui->language, &QComboBox::currentIndexChanged, this, &TestDialog::languageSelected);
    ui->language->clear();
    // Populate the languages combobox before connecting its signal.
    QVector<QLocale> locales = m_speech->availableLocales();
    QLocale current = m_speech->locale();
    foreach (const QLocale &locale, locales) {
        QString name(QString("%1 (%2)")
                         .arg(QLocale::languageToString(locale.language()))
                         .arg(QLocale::countryToString(locale.country())));
        QVariant localeVariant(locale);
        ui->language->addItem(name, localeVariant);
        if (locale.name() == current.name())
            current = locale;
    }
    setRate(ui->rate->value());
    setPitch(ui->pitch->value());
    setVolume(ui->volume->value());

    connect(m_speech, &QTextToSpeech::stateChanged, this, &TestDialog::stateChanged);
    connect(m_speech, &QTextToSpeech::localeChanged, this, &TestDialog::localeChanged);

    connect(ui->language, &QComboBox::currentIndexChanged, this, &TestDialog::languageSelected);
    localeChanged(current);
}

void TestDialog::languageSelected(int language)
{
    QLocale locale = ui->language->itemData(language).toLocale();
    m_speech->setLocale(locale);
}

void TestDialog::voiceSelected(int index)
{
    m_speech->setVoice(m_voices.at(index));
}

void TestDialog::localeChanged(const QLocale &locale)
{
    QVariant localeVariant(locale);
    ui->language->setCurrentIndex(ui->language->findData(localeVariant));

    disconnect(ui->voice, &QComboBox::currentIndexChanged, this, &TestDialog::voiceSelected);
    ui->voice->clear();

    m_voices = m_speech->availableVoices();
    QVoice currentVoice = m_speech->voice();
    foreach (const QVoice &voice, m_voices) {
        ui->voice->addItem(QString("%1 - %2 - %3").arg(voice.name())
                              .arg(QVoice::genderName(voice.gender()))
                              .arg(QVoice::ageName(voice.age())));
        if (voice.name() == currentVoice.name())
            ui->voice->setCurrentIndex(ui->voice->count() - 1);
    }
    connect(ui->voice, &QComboBox::currentIndexChanged, this, &TestDialog::voiceSelected);
}
