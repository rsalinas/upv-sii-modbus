#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QPalette pal = app.palette();
    pal.setColor(QPalette::Window, Qt::white);
    pal.setColor(QPalette::WindowText, Qt::black);
    pal.setColor(QPalette::Base, Qt::white);
    pal.setColor(QPalette::Text, Qt::black);
    pal.setColor(QPalette::Button, Qt::lightGray);
    pal.setColor(QPalette::ButtonText, Qt::black);
    app.setPalette(pal);
    MainWindow w;
    w.show();
    return app.exec();
}
