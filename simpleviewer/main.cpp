#include <QApplication>
#include "oscilloscope.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Oscilloscope oscilloscope;

    oscilloscope.resize(800, 600);
    oscilloscope.show();
    QTimer::singleShot(00000, &a, &QApplication::quit);

    return a.exec();
}
