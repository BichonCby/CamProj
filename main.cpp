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
 * revoir le state machine, c'est pas terrible pour l'alarme
 * on peut imaginer utiliser le timer 1s pour faire clignoter le réglage de l'heure alarme
 * Voir quel bouton active le snooze et lequel arrête l'alarme
 * bouton mode/+/- pour snooze et on rajoute un bouton d'activation de l'alarme
 * rajouter un timer pour revenir à l'affichage de l'heure après 15s d'inactivité
 */
