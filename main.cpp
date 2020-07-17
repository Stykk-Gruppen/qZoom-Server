#include <QCoreApplication>
#include "sockethandler.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    qDebug() << "qZoom-Server running Qt Version: " << QT_VERSION_STR;

    SocketHandler* sh = new SocketHandler();




    return a.exec();
}
