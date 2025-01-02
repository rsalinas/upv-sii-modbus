#include <QApplication>
#include <QDebug>
#include "graphicsview.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    GraphicsView view;
    view.setRenderHint(QPainter::Antialiasing);
    view.setWindowTitle("Joystick emulator");

    view.setMinimumSize(250, 250);
    view.resize(400, 400);                   // Tamaño de la ventana
    view.setSceneRect(-100, -100, 200, 200); // Ajustar la vista para que no haya márgenes
    view.setAlignment(Qt::AlignCenter);      // Centrar la escena en la ventana

    view.show();

    QObject::connect(view.getPointer(), &MovableCircle::positionChanged, [](int x, int y) {
        qDebug() << "Posición actual - X:" << x << "Y:" << y;
    });

    return app.exec();
}
