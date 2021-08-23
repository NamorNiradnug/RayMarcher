#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setOrganizationName("NamorNiradnug");
    QApplication::setOrganizationDomain("github.com/NamorNiradnug/");
    QApplication::setApplicationName("RayMarcher");
    MainWindow w;
    w.show();
    return QApplication::exec();
}
