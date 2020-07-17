#include <QCoreApplication>
#include <QTextStream>
#include "sockethandler.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QTextStream stream(stdout);

    stream << "qZoom-Server running Qt Version: " << QT_VERSION_STR << Qt::endl;

    SocketHandler* sh = new SocketHandler();




    return a.exec();
}
