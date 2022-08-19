#ifndef MONREVEIL_H
#define MONREVEIL_H
#include <opencv2/opencv.hpp>
#include "mainfenetre.h"
#include <opencv2/core/types.hpp>
//#include <qsound.h>
#include <QMediaPlayer>


#define MODE_HORLOGE 0
#define MODE_ALARME_AFFICHAGE 1
#define MODE_ALARME_REGL_H 2
#define MODE_ALARME_REGL_M 3
#define MODE_TYPE_SONNERIE 4
#define MODE_VOL_ALARME 5

#define BUTTON_MODE 1
#define BUTTON_PLUS 2
#define BUTTON_MOINS 3
#define BUTTON_ALARME 4

struct tListAlarmParam
{
    int hour;
    int minute;
    int type;
    int alarmON;
};

class MonReveil  : public QThread
{
    Q_OBJECT
public:
    MonReveil(QObject *parent = nullptr);
    QString textDisplay() const
    {
        return textToDisplay;
    }
    bool alarmState() const
    {
        return mAlarmActivated;
    }
    void ButtonPushed(int b);
    void setAlarmParameters(struct tListAlarmParam s);
    struct tListAlarmParam getAlarmParameters();
signals:
    void NewDisplay();
private :
    void tic500ms();
    void AfficheVide();
    void AfficheHeure();
    void AfficheAlarmeTime();
    void AfficheAlarmeActivation();
    void AfficheAlarmeReglageHeure();
    void AfficheAlarmeReglageMinute();
    void AfficheAlarmeTypeSonnerie();
    void AfficheAlarmeVolume();

    void BackToTimeDisplay();

    void StateMachine();
    void LaunchAlarm();
    QString textToDisplay;
    bool wink=false;
    bool wink500 =false;
    int modeDisplay=MODE_HORLOGE;
    int mPushedButton=0;
    bool mAlarmActivated = false;
    int mAlarmHour=0;
    int mAlarmMinut=0;
    int mAlarmType=0;
    int mAlarmVolume=5;

    QTimer *timer1s; // timer 1s pour clignotement lent
    QTimer *timer500ms; // timer 500ms pour clignotement rapide
    QTimer *timerSnooze; // timer pour lancer le snooze (repeat alarm)
    QTimer *timerBackTime; // timer pour revenir à l'affichage heure après inactivité

    QMediaPlayer *mMusic;
};

#endif // MONREVEIL_H
