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

// **********************************************************
// ****************   TODO  LIST ****************************
// **********************************************************

/* gérer le snooze
 * gérer le niveau sonore
 * Voir quel bouton active le snooze et lequel arrête l'alarme
 * bouton mode/+/- pour snooze et on rajoute un bouton d'activation de l'alarme
 * 
 * gérer la suppression si le fichier video est trop court
 */
