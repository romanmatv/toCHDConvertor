#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDirIterator>
#include <QDebug>

#define CHDMAN_EXE QString("chdman.exe")

//convert string: chdman.exe createcd -i "file.cue" -o "file.chd"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->restoreGeometry(settings.value("window").toByteArray());

    ui->chkbMoveToUpFolder->setChecked(settings.value("chkbMoveToUpFolder").toBool());
    ui->chkbSkipExist->setChecked(settings.value("chkbSkipExist").toBool());

    proc = new QProcess();

    connect(proc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onProcFinished(int,QProcess::ExitStatus)));
    connect(proc, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(onProcError(QProcess::ProcessError)));
    connect(proc, SIGNAL(readyReadStandardOutput()), this, SLOT(onProcText()));
    connect(proc, SIGNAL(readyReadStandardError()), this, SLOT(onProcText()));
    connect(proc, SIGNAL(channelReadyRead(int)), this, SLOT(onProcessChannelReadyRead(int)));

    QTimer::singleShot(100, [&](){
        if (!searchCHDMAN()){
            QMessageBox::critical(this, "chdman", "chdman not found! Place chdman.exe near programm.");
            qApp->exit();
        }
    });
}

MainWindow::~MainWindow()
{
    settings.setValue("window", this->geometry());
    settings.setValue("chkbMoveToUpFolder", ui->chkbMoveToUpFolder->isChecked());
    settings.setValue("chkbSkipExist", ui->chkbSkipExist->isChecked());

    delete ui;
    delete proc;
}

bool MainWindow::searchCHDMAN(){
    if (QFile::exists(CHDMAN_EXE)){
        chdmanPath = CHDMAN_EXE;
        return true;
    }else if (QFile::exists("chdman/"+CHDMAN_EXE)){
        chdmanPath = "chdman/"+CHDMAN_EXE;
        return true;
    }
    return false;
}

void MainWindow::scanFiles(){
    QDirIterator dirit(inputPath, {"*.cue","*.gdi"}, QDir::Files, QDirIterator::Subdirectories);

    fileList.clear();
    ui->listFiles->clear();
    while (dirit.hasNext()){
        auto path = dirit.next();
        fileList.append(path);
        ui->listFiles->addItem(path);
    }

    ui->btnConvert->setEnabled(fileList.size()>0);
}

void MainWindow::on_btnSelectInput_clicked()
{
    QFileDialog *dlg = new QFileDialog;
    dlg->setFileMode(QFileDialog::Directory);
    dlg->setDirectory(settings.value("lastDir").toString());
    if (dlg->exec()){
        settings.setValue("lastDir", dlg->directory().absolutePath());

        inputPath = dlg->directory().absolutePath();
        ui->lineInputPath->setText(inputPath);

        scanFiles();
    }
}

void MainWindow::on_btnConvert_clicked()
{
    startConvert();
}

void MainWindow::startConvert(){
    qDebug() << "=========START CONVERT========";
    queList.clear();
    queList.append(fileList);

    moveUp = ui->chkbMoveToUpFolder->isChecked();
    skipExist = ui->chkbSkipExist->isChecked();

    ui->btnConvert->setEnabled(false);
    ui->btnSelectInput->setEnabled(false);
    ui->chkbMoveToUpFolder->setEnabled(false);
    ui->chkbSkipExist->setEnabled(false);

    ui->btn_abort->setEnabled(true);

    stepProgress = 100.0 / fileList.size();

    nextConvert();
}

void MainWindow::nextConvert(){
    qDebug() << Q_FUNC_INFO;
    if (queList.size()==0){
        //Закончена очередь, финишируем
        ui->btnConvert->setEnabled(true);
        ui->btnSelectInput->setEnabled(true);
        ui->chkbMoveToUpFolder->setEnabled(true);
        ui->chkbSkipExist->setEnabled(true);
        ui->btn_abort->setEnabled(false);

        qApp->beep();

        ui->lbl_status->setText("All converted");
        ui->progressBar->setValue(100);

        qDebug() << "=========ALL CONVERTED========";

        return;
    }
    ui->progressBar->setValue(ui->progressBar->value()+stepProgress);

    QString currentFile = queList.takeFirst();
    QFileInfo file(currentFile);
    QString outputFile;
    QString filename = file.completeBaseName();

    qDebug() << "input:" << currentFile;

    ui->lbl_status->setText(QString("Converting: %1").arg(filename));

    if (moveUp){
        //Переместить файл в каталог выше
        QDir fold(QString("%1/..").arg(file.absoluteDir().absolutePath()));

        outputFile = QString("%1/%2.chd").arg(fold.absolutePath(), filename);
    }else{
        outputFile = QString("%1/%2.chd").arg(file.absoluteDir().absolutePath(), filename);
    }

    qDebug() << "output:"<<outputFile;

    if (skipExist){
        if (QFile::exists(outputFile)){
            qDebug() << "output file exist, skip";
            nextConvert();
            return;
        }
    }

    ui->txtEdit_console->clear();

    //convert string: chdman.exe createcd -i "file.cue" -o "file.chd"
    proc->start(chdmanPath, {"createcd","-i",currentFile,"-o",outputFile});
    qDebug() << "started process";
}

void MainWindow::onProcFinished(int exitCode, QProcess::ExitStatus exitStatus){
    qDebug() << Q_FUNC_INFO << exitCode << exitStatus;
    nextConvert();
}

void MainWindow::onProcError(QProcess::ProcessError error){
    qDebug() << Q_FUNC_INFO << error;
    nextConvert();
}

void MainWindow::onProcessChannelReadyRead(int channel){
    proc->setCurrentReadChannel(channel);
}

void MainWindow::onProcText(){
    QString text = proc->readAllStandardOutput();
    text.removeLast();
    if (!text.isEmpty()) ui->txtEdit_console->append(text);
    text = proc->readAllStandardError();
    text.removeLast();
    if (!text.isEmpty()) ui->txtEdit_console->append(text);
}


void MainWindow::on_btn_abort_clicked()
{
    queList.clear();
    proc->kill();
    proc->waitForFinished();
    QTimer::singleShot(300, [&](){
        ui->lbl_status->setText("Aborted");
    });
}

