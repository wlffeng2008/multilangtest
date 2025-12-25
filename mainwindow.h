#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTranslator>
#include <QEvent>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QTimer>
#include <QKeyEvent>
#include <QUdpSocket>

#include "testdialog.h"

#include <QtMqtt/QMqttClient>
/*
#include "tinyxml2.h"
using namespace tinyxml2;

//测试生成xml
void create_xml(const char* xmlPath);
//测试解析xml
void parse_xml(const char* xmlPath);
//遍历xml node
void traversal_node(XMLNode* node);
//遍历xml element
void traversal_element(XMLNode* node, int level);
//生成预定格式xml
void create_xml2(const char* xmlPath);
//解析预定义格式xml
void parse_xml2(const char* xmlPath);
*/

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setTranslater(QTranslator *pTranslator ){m_pTranslator = pTranslator;}

protected:
    void changeEvent(QEvent *pEvt);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private slots:
    void on_pushButton_2_clicked();

    void on_radioButton_clicked();

    void on_radioButton_2_clicked();

    void on_checkBox_clicked(bool checked);

    void on_pushButtonFull_clicked();

    void on_fontComboBox_currentFontChanged(const QFont &f);

    void on_pushButton_3_clicked();

private:
    Ui::MainWindow *ui;
    QTranslator *m_pTranslator ;

    TestDialog *m_pTestDlg ;

    QSystemTrayIcon *m_pTray ;
    QMenu *m_pMenu ;
    QAction *pActAbout  ;
    QAction *pActExit  ;

    QMqttClient m_Client ;

    QUdpSocket *m_udpSock = nullptr ;
};
#endif // MAINWINDOW_H
