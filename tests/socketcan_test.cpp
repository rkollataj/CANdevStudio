//#define CATCH_CONFIG_RUNNER
//#include <fakeit.hpp>
#include <QCoreApplication>
#include <QTimer>
#include <QVariant>
#include <candevice.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTimer timer;
    CanDevice cd;
    int cnt = 0;

    cd.init("socketcan", "can0");
    cd.start();
    
}

