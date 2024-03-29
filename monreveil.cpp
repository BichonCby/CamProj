#include "monreveil.h"
#include <QSound>

MonReveil::MonReveil(QObject *parent)
    : QThread {parent}
{

    timer1s = new QTimer(this);
    connect(timer1s,&QTimer::timeout,this,&MonReveil::AfficheHeure);
    timer1s->start(1000);

    timer500ms = new QTimer(this);
    connect(timer500ms,&QTimer::timeout,this,&MonReveil::tic500ms);
    timer500ms->start(500);

    timerBackTime = new QTimer(this);
    timerBackTime->setSingleShot(true);
    timerBackTime->setInterval(15000);
    connect(timerBackTime,&QTimer::timeout,this,&MonReveil::BackToTimeDisplay);

    mMusic = new QMediaPlayer();
    mMusic->setMedia(QUrl::fromLocalFile("./son1.wav")); // a voir quand on aura différentes sources

}

void MonReveil::BackToTimeDisplay()
{
    modeDisplay=MODE_HORLOGE;
    StateMachine();

}
void MonReveil::tic500ms()
{
    wink500 = !wink500;
    //qDebug() << wink500;
    if (modeDisplay == MODE_ALARME_AFFICHAGE)
    {
        if (wink500)
            AfficheVide();
        else
            AfficheAlarmeTime();
    }
    if (modeDisplay == MODE_ALARME_REGL_H )
    {
        if (wink500)
            AfficheVide();
        else
            AfficheAlarmeReglageHeure();
    }
    if (modeDisplay == MODE_ALARME_REGL_M )
    {
        if (wink500)
            AfficheVide();
        else
            AfficheAlarmeReglageMinute();
    }
    return;
}
void MonReveil::setAlarmParameters(struct tListAlarmParam s)
{
    mAlarmHour = s.hour;
    mAlarmMinut = s.minute;
    mAlarmActivated = s.alarmON;
    mAlarmType = s.type;
}

struct tListAlarmParam MonReveil::getAlarmParameters()
{
    struct tListAlarmParam s;
    s.hour= mAlarmHour;
    s.minute =mAlarmMinut;
    s.alarmON = mAlarmActivated;
    s.type = mAlarmType;
    return s;
}

void MonReveil::AfficheVide()
{
    mTMtype = TM_TYPE_VIDE;
    textToDisplay.clear();
    emit NewDisplay();

}
void MonReveil::AfficheHeure()
{
    QDateTime date=QDateTime::currentDateTime();
    if (modeDisplay == MODE_HORLOGE)
    {
        textToDisplay.clear();
        textToDisplay.append(formatNumber(date.time().hour()));
        if (wink) // on affiche les deux points
            textToDisplay.append(":");
        else
            textToDisplay.append(" ");
        //    textToDisplay.append(QString::number(date.time().minute()));
        textToDisplay.append(formatNumber(date.time().minute()));
        mTMtype = wink?TM_TYPE_TIME_M:TM_TYPE_TIME_P;
        mTMval = 100 * date.time().hour() + date.time().minute();
        wink=!wink;

        emit NewDisplay();
        // qDebug() << textToDisplay;
    }
    // vérification de l'alarme
    if (date.time().second()==0)
    {
        if ((mAlarmActivated) && (mAlarmHour == date.time().hour()) && (mAlarmMinut == date.time().minute()))
        {
            LaunchAlarm();
        }
    }
}

void MonReveil::AfficheAlarmeTime()
{
    textToDisplay.clear();
    textToDisplay.append(formatNumber(mAlarmHour));
    textToDisplay.append(":");
    textToDisplay.append(formatNumber(mAlarmMinut));
    // pour l'afficheur
    mTMtype = TM_TYPE_TIME_P;
    mTMval = 100*mAlarmHour+mAlarmMinut;
    emit NewDisplay();
    return;
}

void MonReveil::AfficheAlarmeActivation()
{
/*    textToDisplay.clear();
    textToDisplay.append("AL ");
    if (mAlarmActivated)
        textToDisplay.append("1");
    else
        textToDisplay.append("0");*/
    emit NewDisplay();
}
void MonReveil::AfficheAlarmeReglageHeure()
{
    textToDisplay.clear();
    textToDisplay.append(formatNumber(mAlarmHour));
    textToDisplay.append(":  ");
    mTMtype = TM_TYPE_REGL_HOUR;
    mTMval = mAlarmHour;
    emit NewDisplay();
}
void MonReveil::AfficheAlarmeReglageMinute()
{
    textToDisplay.clear();
    textToDisplay.append("  :");
    textToDisplay.append(formatNumber(mAlarmMinut));
    mTMtype = TM_TYPE_REGL_MIN;
    mTMval = mAlarmMinut;
    emit NewDisplay();

}
void MonReveil::AfficheAlarmeTypeSonnerie()
{
    textToDisplay.clear();
    textToDisplay.append("SON");
    mTMtype = TM_TYPE_TYPE;
    emit NewDisplay();
}

void MonReveil::AfficheAlarmeVolume()
{
    textToDisplay.clear();
    textToDisplay.append("oO: ");
    textToDisplay.append(QString::number(mAlarmVolume));
    mTMtype = TM_TYPE_VOL;
    mTMval = mAlarmVolume;
    emit NewDisplay();

}
void MonReveil::StateMachine()
{
    switch (modeDisplay)
    {
    case MODE_HORLOGE :
        if (mPushedButton == BUTTON_MODE)
        {
            modeDisplay = MODE_ALARME_AFFICHAGE;
           // qDebug() << "chagt mode";
        }
        mPushedButton = 0; // réinit du push
        break;
    case MODE_ALARME_AFFICHAGE :
        AfficheAlarmeTime();
        if (mPushedButton == BUTTON_MODE)
            modeDisplay = MODE_ALARME_REGL_H;
        mPushedButton = 0; // réinit du push
        break;
/*    case ALARME_ACTIVATION :
        AfficheAlarmeActivation();
        if (mPushedButton == BUTTON_PLUS || mPushedButton == BUTTON_MOINS)
        {
            mAlarmActivated = !mAlarmActivated;
            AfficheAlarmeActivation();
        }
        if(mPushedButton == BUTTON_MODE)
        {
            modeDisplay = MODE_ALARME_REGL_H;
            AfficheAlarmeReglageHeure();
        }
        mPushedButton = 0;*
        break;*/
    case MODE_ALARME_REGL_H :
        AfficheAlarmeReglageHeure();
        if (mPushedButton == BUTTON_PLUS)
        {
            mAlarmHour = (mAlarmHour==23)?0:mAlarmHour+1;
            AfficheAlarmeReglageHeure();
        }
        if (mPushedButton == BUTTON_MOINS)
        {
            mAlarmHour = (mAlarmHour==0)?23:mAlarmHour-1;
            AfficheAlarmeReglageHeure();
        }
        if(mPushedButton == BUTTON_MODE)
        {
            modeDisplay = MODE_ALARME_REGL_M;
            AfficheAlarmeReglageMinute();
        }
        mPushedButton = 0;
        break;
    case MODE_ALARME_REGL_M :
        AfficheAlarmeReglageMinute();
        if (mPushedButton == BUTTON_PLUS)
        {
            mAlarmMinut = (mAlarmMinut==59)?0:mAlarmMinut+1;
            AfficheAlarmeReglageMinute();
        }
        if (mPushedButton == BUTTON_MOINS)
        {
            mAlarmMinut = (mAlarmMinut==0)?59:mAlarmMinut-1;
            AfficheAlarmeReglageMinute();
        }
        if(mPushedButton == BUTTON_MODE)
        {
            modeDisplay = MODE_TYPE_SONNERIE;
            AfficheAlarmeTypeSonnerie();
        }
        mPushedButton = 0;
        break;

    case MODE_TYPE_SONNERIE :
        AfficheAlarmeTypeSonnerie();
        if(mPushedButton == BUTTON_MODE)
        {
            modeDisplay = MODE_VOL_ALARME;
            AfficheAlarmeVolume();
        }
        mPushedButton = 0;
        break;

    case MODE_VOL_ALARME:
        AfficheAlarmeVolume();
        if (mPushedButton == BUTTON_PLUS)
        {
            mAlarmVolume = std::min(9,mAlarmVolume+1);
            AfficheAlarmeVolume();
        }
        if (mPushedButton == BUTTON_MOINS)
        {
            mAlarmVolume = std::max(0,mAlarmVolume-1);
            AfficheAlarmeVolume();
        }
        if(mPushedButton == BUTTON_MODE)
        {
            modeDisplay = MODE_HORLOGE;
            AfficheHeure();
        }
        mPushedButton = 0;
        break;

    }
}
void MonReveil::ButtonPushed(int b)
{
    mPushedButton = b; // un seul bouton à la fois
    if (b==BUTTON_ALARME)
    {
        mAlarmActivated = !mAlarmActivated;
        AfficheAlarmeActivation();
    }
    else
        timerBackTime->start();
    StateMachine();
    StateMachine();
}
// on utilisera alors un timer pour le snooze uniquement
// TODO
/*void MonReveil::LaunchTimerAlarm(bool s)
{
    if (!s)
        timerAlarm->stop();
    else
        if (!timerAlarm->isActive())
        {
            qDebug() << "activation de l'alarme";
            QDateTime dateN=QDateTime::currentDateTime();
            int t=mAlarmHour*60+mAlarmMinut-dateN.time().hour()*60-dateN.time().minute();
            if (t<0) t+=1440;
            timerAlarm->start(t*60*1000);
        }
}*/
void MonReveil::LaunchAlarm()
{
    qDebug() << "ALARME!!!!";
    modeDisplay = MODE_HORLOGE; // au cas où, mais normalement on devrait déjà y être
    StateMachine();
    mMusic->setVolume(mAlarmVolume*10);
    mMusic->play();
}
