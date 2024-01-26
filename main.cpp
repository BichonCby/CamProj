#include "mainfenetre.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("Fusion");
    MainFenetre w;
    w.showMinimized();

    return a.exec();
}

// **********************************************************
// ****************   TODO  LIST ****************************
// **********************************************************

/* gérer le snooze
 * gérer le niveau sonore
 * Voir quel bouton active le snooze et lequel arrête l'alarme
 * bouton mode/+/- pour snooze et on rajoute un bouton d'activation de l'alarme
 * Faire une machine a etat pour l'affichage et la gestion alarme/heure radio
 */
