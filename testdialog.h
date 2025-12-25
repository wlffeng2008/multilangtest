#ifndef TESTDIALOG_H
#define TESTDIALOG_H

#include <QDialog>
#include <QTextToSpeech>
#include <QModelIndex>

namespace Ui {
class TestDialog;
}

class TestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TestDialog(QWidget *parent = nullptr);
    ~TestDialog();

    void playFile(const QString&strFile);

private slots:
    void on_pushButtonOK_clicked();
    void on_pushButtonCancel_clicked();

public slots:
    void speak();
    void pause();
    void resume();
    void stop();

    void setRate(int);
    void setPitch(int);
    void setVolume(int volume);

    void stateChanged(QTextToSpeech::State state);
    void engineSelected(int index);
    void languageSelected(int language);
    void voiceSelected(int index);

    void localeChanged(const QLocale &locale);

protected:
    void changeEvent(QEvent *pEvt);

private:
    Ui::TestDialog *ui;
    QTextToSpeech *m_speech=nullptr;
    QVector<QVoice> m_voices;

    bool m_bPause = false;
    qint64 m_nDur ;
    QString m_strSelFile ;
    QModelIndex m_nCurItem;
    void playNext(int nOffset=1) ;
};

#endif // TESTDIALOG_H
