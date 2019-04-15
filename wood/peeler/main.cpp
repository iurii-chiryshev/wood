#include "ui/mainwindow.h"
#include <QApplication>

/***
 * Начальная точка входа
 * */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
