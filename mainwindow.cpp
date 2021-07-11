#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    char ptr[]="python --version";

    ExcudeCmd(ptr);

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

void MainWindow::CreateChildProcess(char* szCmdlinein)
// Create a child process that uses the previously created pipes for STDIN and STDOUT.
{
    LPTSTR szCmdline = szCmdlinein;//这是子进程可执行文件的路径
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    BOOL bSuccess = FALSE;

    // Set up members of the PROCESS_INFORMATION structure.

    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    // Set up members of the STARTUPINFO structure.
    // This structure specifies the STDIN and STDOUT handles for redirection.

    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = g_hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
    siStartInfo.hStdInput = g_hChildStd_IN_Rd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
    siStartInfo.wShowWindow=SW_HIDE;

    // Create the child process.

    bSuccess = CreateProcess(NULL,
                             szCmdline,     // command line
                             NULL,          // process security attributes
                             NULL,          // primary thread security attributes
                             TRUE,          // handles are inherited
                             0,             // creation flags
                             NULL,          // use parent's environment
                             NULL,          // use parent's current directory
                             &siStartInfo,  // STARTUPINFO pointer
                             &piProcInfo);  // receives PROCESS_INFORMATION

    // If an error occurs, exit the application.
    if (!bSuccess)
        ErrorExit(TEXT("CreateProcess"));
    else
    {
        DWORD dwRet = WaitForSingleObject(piProcInfo.hProcess,INFINITE);
        // Close handles to the child process and its primary thread.
        // Some applications might keep these handles to monitor the status
        // of the child process, for example.

        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);
    }
}

void MainWindow::WriteToPipe(void)

// Read from a file and write its contents to the pipe for the child's STDIN.
// Stop when there is no more data.
{
    DWORD dwRead, dwWritten;
    CHAR chBuf[BUFSIZE];
    BOOL bSuccess = FALSE;

    for (;;)
    {
        bSuccess = ReadFile(g_hInputFile, chBuf, BUFSIZE, &dwRead, NULL);
        if (!bSuccess || dwRead == 0) break;

        bSuccess = WriteFile(g_hChildStd_IN_Wr, chBuf, dwRead, &dwWritten, NULL);
        if (!bSuccess) break;
    }

    // Close the pipe handle so the child process stops reading.

    if (!CloseHandle(g_hChildStd_IN_Wr))
        ErrorExit(TEXT("StdInWr CloseHandle"));
}

void MainWindow::ReadFromPipe(void)

// Read output from the child process's pipe for STDOUT
// and write to the parent process's pipe for STDOUT.
// Stop when there is no more data.
{
    BOOL bSuccess = FALSE;
    HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    if (!CloseHandle(g_hChildStd_OUT_Wr))
        ErrorExit(TEXT("StdInWr CloseHandle"));

    while(true){
        memset(chBuf,0,BUFSIZE);
        dwRead=0;
        bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
        if(!bSuccess || dwRead==0){
            break;
        }
        qDebug()<<chBuf;
        ui->ConsoleOutput->setText(ui->ConsoleOutput->toPlainText()+QString(chBuf));
    }
}

void MainWindow::ErrorExit(PTSTR lpszFunction)
// Format a readable error message, display a message box,
// and exit from the application.
{
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                dw,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&lpMsgBuf,
                0, NULL);

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
                                      (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
                    LocalSize(lpDisplayBuf) / sizeof(TCHAR),
                    TEXT("%s failed with error %d: %s"),
                    lpszFunction, dw, lpMsgBuf);
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(1);
}

BOOL MainWindow::ExcudeCmd( char* szCmdLine)
{
    SetConsoleOutputCP(65001); // chcp 65001

    SECURITY_ATTRIBUTES saAttr;
    // Set the bInheritHandle flag so pipe handles are inherited.

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT.

    if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
        ErrorExit(TEXT("StdoutRd CreatePipe"));

    // Ensure the read handle to the pipe for STDOUT is not inherited.

    if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
        ErrorExit(TEXT("Stdout SetHandleInformation"));

    // Create a pipe for the child process's STDIN.

    if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0))
        ErrorExit(TEXT("Stdin CreatePipe"));

    // Ensure the write handle to the pipe for STDIN is not inherited.

    if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
        ErrorExit(TEXT("Stdin SetHandleInformation"));

    CreateChildProcess(szCmdLine);

    ReadFromPipe();

    return TRUE;
}

void MainWindow::on_pushButton_6_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    QString::fromLocal8Bit("文件对话框！"),
                                                    "C:",
                                                    QString::fromLocal8Bit("图片文件(*png *jpg);"));
    ui->inputFilePath->setText(fileName);
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
                        }
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

                        return true;
                    }else{
                        QMessageBox message(QMessageBox::Warning,QString::fromLocal8Bit("错误！"),QString::fromLocal8Bit("水印位置不能为空！"),QMessageBox::Ok,NULL);
                        message.exec();
                        return false;
                    }
                }else{
                    QMessageBox message(QMessageBox::Warning,QString::fromLocal8Bit("错误！"),QString::fromLocal8Bit("水印内容不能为空！"),QMessageBox::Ok,NULL);
                    message.exec();
                    return false;
                }

            }else{
                QMessageBox message(QMessageBox::Warning,QString::fromLocal8Bit("错误！"),QString::fromLocal8Bit("输出文件名不能为空！"),QMessageBox::Ok,NULL);
                message.exec();
                return false;
            }
        }
    }else{
        QMessageBox message(QMessageBox::Warning,QString::fromLocal8Bit("错误！"),QString::fromLocal8Bit("输入文件不能为空！"),QMessageBox::Ok,NULL);
        message.exec();
        return false;
    }
}

QString MainWindow::constructParameter(QString mode)
{
    QString executableFilePath="SimpleWaterMark.exe";

    QString cmdCommand=executableFilePath+" -i "+parameter.inputFilePath+" -m "+mode+" -w "+parameter.waterMarkText+" -p "+QString::fromStdString(std::to_string(parameter.position));

    if(parameter.outputDirectory!=""){
        cmdCommand+=" -o "+parameter.outputDirectory+"/"+parameter.outputName;
    }else{
        cmdCommand+=" -o "+parameter.outputName;
    }

    if(parameter.isVideo){
        cmdCommand+=" -t video";
    }else{
        cmdCommand+=" -t pic";
    }

    if(parameter.isYUV){
        cmdCommand+=" -v ";
    }

    if(parameter.ifDump){
        cmdCommand+=" -d ";
    }

    if(parameter.frameSize!=0){
        cmdCommand+=" -f ";
    }
    qDebug()<<cmdCommand;
    return cmdCommand;
}


void MainWindow::on_pushButton_3_clicked()
{
    QString PathName;
    PathName = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                 "C:",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    if (PathName!="") {
        ui->outputDir->setText(PathName);
    }
}

void MainWindow::on_EncodeBtn_clicked()
{
    if(check_status()){
        QString cmdCommand=constructParameter("encode");
        int size=strlen(cmdCommand.toStdString().c_str());
        char* buf=new char[size];
        memset(buf,0,size);
        strcpy(buf,cmdCommand.toStdString().c_str());
        ExcudeCmd((char*)buf);
    }
}

void MainWindow::on_DecodeBtn_clicked()
{
    if(check_status()){
        QString cmdCommand=constructParameter("decode");
        int size=strlen(cmdCommand.toStdString().c_str());
        char* buf=new char[size];
        memset(buf,0,size);
        strcpy(buf,cmdCommand.toStdString().c_str());
        ExcudeCmd((char*)buf);
    }
}

