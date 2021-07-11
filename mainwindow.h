#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <Windows.h>
#include <QString>
#include <QFileDialog>
#include <QDebug>
#include <iostream>
#include <QMessageBox>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    BOOL ExcudeCmd( char* szCmdLine);
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

    QString constructParameter(QString mode);

    #define BUFSIZE 1024

    CHAR chBuf[BUFSIZE];
    DWORD dwRead;

    HANDLE g_hChildStd_IN_Rd = NULL;
    HANDLE g_hChildStd_IN_Wr = NULL;
    HANDLE g_hChildStd_OUT_Rd = NULL;
    HANDLE g_hChildStd_OUT_Wr = NULL;

    HANDLE g_hInputFile = NULL;

    void CreateChildProcess(char* szCmdlinein);
    void WriteToPipe(void);
    void ReadFromPipe(void);
    void ErrorExit(PTSTR lpszFunction);

private slots:
    void on_pushButton_6_clicked();

    void on_EncodeBtn_clicked();

    void on_pushButton_3_clicked();

    void on_DecodeBtn_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
