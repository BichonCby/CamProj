#ifndef MONREVEIL_H
#define MONREVEIL_H
#include <opencv2/opencv.hpp>
#include "mainfenetre.h"
#include <opencv2/core/types.hpp>

#define MODE_HORLOGE 0
#define MODE_ALARME_AFFICHAGE 1
#define ALARME_ACTIVATION 2
#define MODE_ALARME_REGL_H 3
#define MODE_ALARME_REGL_M 4
#define MODE_TYPE_SONNERIE 5

#define BUTTON_MODE 1
#define BUTTON_PLUS 2
#define BUTTON_MOINS 3
#define BUTTON_SECRET 4

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
};

#endif // MONREVEIL_H
