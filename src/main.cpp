#include "lazy.h"
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    auto *a = new QApplication(argc, argv);
    QApplication::setOrganizationName("NamorNiradnug");
    QApplication::setOrganizationDomain("github.com/NamorNiradnug/");
    QApplication::setApplicationName("RayMarcher");
    MainWindow w(a);
    w.show();
    return QApplication::exec();
}
