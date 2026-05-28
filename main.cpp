#include "mainwindow.h"

#define _STR(x) #x
#define STRINGIFY(x)  _STR(x)
#define APPLICATIONVERSION STRINGIFY(APP_VERSION)

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    qApp->setApplicationName("toCHDConvertor");
    qApp->setOrganizationName("romanmatv");
    qApp->setOrganizationDomain("romanmatv");
    qApp->setApplicationVersion(APPLICATIONVERSION);

    qApp->setStyle(QStyleFactory::create("Fusion"));

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return QApplication::exec();
}
