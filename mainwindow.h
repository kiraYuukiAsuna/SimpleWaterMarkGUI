#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <Windows.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

#define BUFFERZ_SIZE 1024
    char buffer[BUFFERZ_SIZE];

    BOOL ExcudeCmd(char* szOutPutBuf, char* szCmdLine);
    char* substring(char* ch,int pos,int length);
    bool check_status();

    struct Parameter{
        QString inputFilePath;
        QString outputDirectory="";
        QString outputName="output001.mp4";
        QString waterMarkText="2019213336WRL";
        double position=0.2;
        int frameSize=0;
        bool isYUV=true;
        bool ifDump=false;
        bool isVideo=false;
    };

    Parameter parameter;

private slots:
    void on_pushButton_6_clicked();

    void on_EncodeBtn_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
