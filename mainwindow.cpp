#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QString>
#include <QFileDialog>
#include <QDebug>
#include <iostream>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    ExcudeCmd(buffer,"python --version");
    //ExcudeCmd(buffer,"SimpleWaterMark.exe -i input.jpg -o output001.jpg  -m encode  -v -t pic -w HELLOWORLD");
}

MainWindow::~MainWindow()
{
    delete ui;
}

char* MainWindow::substring(char* ch,int pos,int length)
{
    //定义字符指针 指向传递进来的ch地址
    char* pch=ch;
    //通过calloc来分配一个length长度的字符数组，返回的是字符指针。
    char* subch=(char*)calloc(sizeof(char),length+1);
    pch=pch+pos;
    //是pch指针指向pos位置。
    for(int i=0;i<length;i++)
    {
        subch[i]=*(pch++);
        //循环遍历赋值数组。
    }
    subch[length]='\0';//加上字符串结束符。
    return subch;       //返回分配的字符数组地址。
}

BOOL MainWindow::ExcudeCmd(char* szOutPutBuf, char* szCmdLine)
{
    SetConsoleOutputCP(65001); // chcp 65001

    SECURITY_ATTRIBUTES sa;

    HANDLE hRead, hWrite;

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);

    sa.lpSecurityDescriptor = NULL;

    sa.bInheritHandle = TRUE;    //输出重定向

    if (!CreatePipe(&hRead, &hWrite, &sa, 0))
    {
        return FALSE;
    }

    STARTUPINFO si;

    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(STARTUPINFO));

    si.cb = sizeof(STARTUPINFO);

    si.hStdInput = hRead;

    si.hStdError = GetStdHandle(
                STD_ERROR_HANDLE);    //把创建进程的标准错误输出重定向到管道输入

    si.hStdOutput = hWrite;    //把创建进程的标准输出重定向到管道输入

    si.wShowWindow = SW_HIDE;

    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

    if (!CreateProcess(NULL,szCmdLine , NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
    {
        CloseHandle(hWrite);

        CloseHandle(hRead);

        return FALSE;
    }
    else
    {
        CloseHandle(pi.hProcess);

        CloseHandle(pi.hThread);
    }

    DWORD bytesRead;

    ReadFile(hRead, szOutPutBuf, BUFFERZ_SIZE, &bytesRead, NULL);
    qDebug()<<"bytesRead:"<<bytesRead;

    ui->ConsoleOutput->setText(ui->ConsoleOutput->toPlainText()+QString(substring(szOutPutBuf,0,bytesRead)));


    CloseHandle(hRead);

    return TRUE;
}

void MainWindow::on_pushButton_6_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    QString::fromLocal8Bit("文件对话框！"),
                                                    "C:",
                                                    QString::fromLocal8Bit("图片文件(*png *jpg);"));
    qDebug()<<"filename : "<<fileName;
}

bool MainWindow::check_status(){
    if(ui->inputFilePath->text()!=""){
        parameter.inputFilePath=ui->inputFilePath->text();
        if(ui->outputDir->text()!=""){
            parameter.outputDirectory=ui->outputDir->text();
            if(ui->outputName->text()!=""){
                parameter.outputName=ui->outputName->text();
                if(ui->waterMarkText->text()!=""){
                    parameter.waterMarkText=ui->waterMarkText->text();
                    if(ui->position->text()!=""){
                        parameter.position=ui->position->text().toDouble();
                        if(ui->frameSize->text()!=""){
                            parameter.frameSize=ui->frameSize->text().toDouble();

                            if(ui->YUVMode->isChecked()==false){
                                parameter.isYUV=true;
                            }else{
                                parameter.isYUV=false;
                            }

                            if(ui->sequence->isChecked()==false){
                                parameter.ifDump=false;
                            }else{
                                parameter.ifDump=true;
                            }

                            if(ui->videoMode->isChecked()==false){
                                parameter.isVideo=false;
                            }else{
                                parameter.isVideo=true;
                            }
                        }
                    }
                }

            }
        }
    }else{
        QMessageBox message(QMessageBox::Warning,QString::fromLocal8Bit("错误！"),QString::fromLocal8Bit("输入文件不能为空"),QMessageBox::Ok,NULL);
        message.exec();
        return false;
    }
}

void MainWindow::on_EncodeBtn_clicked()
{
    check_status();
}

