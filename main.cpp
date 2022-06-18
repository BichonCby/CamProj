#include "mainfenetre.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("Fusion");
    MainFenetre w;
    w.show();

    return a.exec();
}
