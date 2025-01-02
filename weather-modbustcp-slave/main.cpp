#include <QApplication>
#include <QSplashScreen>
#include <QTimer>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QPixmap splashImage(":/images/images/splash.png");
    Q_ASSERT(!splashImage.isNull());

    QSplashScreen splash(splashImage);
    splash.showMessage("Loading...", Qt::AlignBottom | Qt::AlignHCenter, Qt::black);
    splash.show();
    MainWindow *mainWindow;
    QTimer::singleShot(2000, [&]() {
        splash.close();
        mainWindow = new MainWindow();
        mainWindow->show();
    });

    return app.exec();
}
