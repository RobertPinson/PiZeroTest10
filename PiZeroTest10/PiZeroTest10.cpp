#include "MainWindow.h"
#include <QApplication>
#include "qtmosquitto.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	QtMosquittoApp mapp;
    MainWindow w;
    w.show();
    
    return a.exec();
}
