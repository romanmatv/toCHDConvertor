#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QTimer>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_btnSelectInput_clicked();

    void onProcFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcError(QProcess::ProcessError error);
    void onProcText();
    void onProcessChannelReadyRead(int chanel);

    void on_btnConvert_clicked();

    void on_btn_abort_clicked();

private:
    Ui::MainWindow *ui;
    QSettings settings;

    QString chdmanPath;
    QString inputPath;
    QStringList fileList, queList;

    QProcess *proc;

    bool moveUp, skipExist;

    qreal stepProgress;

    bool searchCHDMAN();
    void scanFiles();

    void startConvert();
    void nextConvert();
};
#endif // MAINWINDOW_H
