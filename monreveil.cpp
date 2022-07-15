#include "monreveil.h"
QTimer *timer1s;
MonReveil::MonReveil(QObject *parent)
    : QThread {parent}
{

    timer1s = new QTimer(this);
    connect(timer1s,&QTimer::timeout,this,&MonReveil::AfficheHeure);
    timer1s->start(1000);

}

void MonReveil::AfficheHeure()
{
    if (modeDisplay == MODE_HORLOGE)
    {
        QDateTime date=QDateTime::currentDateTime();
        textToDisplay.clear();
        textToDisplay.append(formatNumber(date.time().hour()));
        if (wink) // on affiche les deux points
            textToDisplay.append(":");
        else
            textToDisplay.append(" ");
        //    textToDisplay.append(QString::number(date.time().minute()));
        textToDisplay.append(formatNumber(date.time().minute()));
        wink=!wink;
        emit NewDisplay();
        // qDebug() << textToDisplay;
    }
}

void MonReveil::AfficheAlarmeTime()
{
    textToDisplay.clear();
    textToDisplay.append(formatNumber(mAlarmHour));
    textToDisplay.append(":");
    textToDisplay.append(formatNumber(mAlarmMinut));
    emit NewDisplay();
    return;
}

void MonReveil::AfficheAlarmeActivation()
{
    textToDisplay.clear();
    textToDisplay.append("AL ");
    if (mAlarmActivated)
        textToDisplay.append("1");
    else
        textToDisplay.append("0");
    emit NewDisplay();

}
void MonReveil::AfficheAlarmeReglageHeure()
{
    textToDisplay.clear();
    textToDisplay.append(formatNumber(mAlarmHour));
    textToDisplay.append(":  ");
    emit NewDisplay();
}
void MonReveil::AfficheAlarmeReglageMinute()
{
    textToDisplay.clear();
    textToDisplay.append("  :");
    textToDisplay.append(formatNumber(mAlarmMinut));
    emit NewDisplay();

}
void MonReveil::AfficheAlarmeTypeSonnerie()
{
    textToDisplay.clear();
    textToDisplay.append("SON");
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
            qDebug() << "chagt mode";
        }
        mPushedButton = 0; // réinit du push
        break;
    case MODE_ALARME_AFFICHAGE :
        AfficheAlarmeTime();
        if (mPushedButton == BUTTON_MODE)
            modeDisplay = ALARME_ACTIVATION;
        mPushedButton = 0; // réinit du push
        break;
    case ALARME_ACTIVATION :
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
        mPushedButton = 0;
        break;
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
            modeDisplay = MODE_HORLOGE;
            //AfficheAlarme();
        }
        mPushedButton = 0;
        break;

    }
}
void MonReveil::ButtonPushed(int b)
{
    mPushedButton = b; // un seul bouton à la fois
    qDebug() <<  "bouton" << QString::number(b);
    StateMachine();
    StateMachine();
}
