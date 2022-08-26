#include "mainfenetre.h"
#include "ui_mainfenetre.h"
#include "mavideocapture.h"
#include "monreveil.h"


struct tListParam sListParamUi;
struct tListAlarmParam sListAlarmParm;


MainFenetre::MainFenetre(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainFenetre)
{
    ui->setupUi(this);
    mOpenCV_videoCapture = new MaVideoCapture(this);
    mMonReveil = new MonReveil(this);
    connect(mOpenCV_videoCapture,&MaVideoCapture::NewPixmapCaptured,this,[&]()
        {
            DisplayVideo(mOpenCV_videoCapture->stateCamera());
        });
    connect(mMonReveil,&MonReveil::NewDisplay,this,[&]()
        {
            DisplayClock(mMonReveil->textDisplay());
            DisplayAlarm(mMonReveil->alarmState());
            writeTM(mMonReveil->TMval(),mMonReveil->TMtype());
        });
    // ************************* triggers ****
    trigger = new QTimer(this);
    connect (trigger, &QTimer::timeout, this, &MainFenetre::ticHorloge);
    buttonTimer = new QTimer(this);
    connect (buttonTimer, &QTimer::timeout,this,&MainFenetre::checkButton);
    buttonTimer->start(100);

    ui->typeVisu->addItem("Brute");
    ui->typeVisu->addItem("Blurred");
    ui->typeVisu->addItem("Diff");
    ui->typeVisu->addItem("autre");
    sConf[IDX_CONF_BLUR]={"blursize",5,"Interface"};
    sConf[IDX_CONF_SEUIL]={"seuildiff",35,"Interface"};
    sConf[IDX_CONF_TYPEVISU]={"typevisu",0,"Interface"};
    sConf[IDX_CONF_SIZEMVT]={"tailleMvt",10,"Motion"};
    sConf[IDX_CONF_TIMEMVT]={"timeMvt",5,"Motion"};
    sConf[IDX_CONF_AUTO_ON]={"AutoON",0,"Interface"};
    sConf[IDX_CONF_ALARME_HOUR]={"AlarmHour",0,"Alarm"};
    sConf[IDX_CONF_ALARME_MINUTE]={"AlarmMinute",0,"Alarm"};
    sConf[IDX_CONF_ALARME_ON]={"AlarmOn",0,"Alarm"};
    sConf[IDX_CONF_ALARME_TYPE]={"AlarmType",0,"Alarm"};

    readAllParam();
    // si Auto ON est configuré, on lance directement le processus
    if (sConf[IDX_CONF_AUTO_ON].val)
        on_InitOpenCV_button_clicked();
    mTM1637 = new tm1637();
    configGPIO();

}

MainFenetre::~MainFenetre()
{
    writeAllParam();
    delete ui;
    if (mOpenCV_videoCapture->isRunning())
    {
        mOpenCV_videoCapture->terminate();
    }
    //qDebug() << "destruction";
    mOpenCV_videoCapture->quit();
}

void MainFenetre::ticHorloge()
{
    mOpenCV_videoCapture->loopTreatment();
}
void MainFenetre::DisplayVideo(int state)
{
    //qDebug() << "appel au display" << state;
    if (state == CAM_OK)
    {
        ui->opencvFrame->setPixmap(mOpenCV_videoCapture->pixmap().scaled(500,500));
        ui->Luminosity->setText(QString::number(mOpenCV_videoCapture->luminosity()));
        ui->RecordLed->setValue(mOpenCV_videoCapture->recording());
        ui->MouvementLed->setValue(mOpenCV_videoCapture->mouvement());
    }
    else if (state == CAM_HS)
    {
        ui->opencvFrame->setText("Pas de caméra");
    }
    return;
}

void MainFenetre::DisplayClock(QString t)
{
    ui->Time_display->setText(t);
    return;
}
void MainFenetre::DisplayAlarm(bool a)
{
    ui->Alarm_led->setValue(a);
    return;
}


void MainFenetre::sendCalibration()
{
    sListParamUi.blur = sConf[IDX_CONF_BLUR].val;
    sListParamUi.seuil = sConf[IDX_CONF_SEUIL].val;
    sListParamUi.typevisu = sConf[IDX_CONF_TYPEVISU].val;
    sListParamUi.sizeMvt = sConf[IDX_CONF_SIZEMVT].val;
    sListParamUi.timeMvt = sConf[IDX_CONF_TIMEMVT].val;
    mOpenCV_videoCapture->setCalibration(sListParamUi);
}
void MainFenetre::sendAlarmParameters()
{
    sListAlarmParm.hour = sConf[IDX_CONF_ALARME_HOUR].val;
    sListAlarmParm.minute = sConf[IDX_CONF_ALARME_MINUTE].val;
    sListAlarmParm.alarmON = sConf[IDX_CONF_ALARME_ON].val;
    sListAlarmParm.type = sConf[IDX_CONF_ALARME_TYPE].val;
    mMonReveil->setAlarmParameters(sListAlarmParm);
}
void MainFenetre::readAllParam()
{
    QFile mfile(CONFIG_FILE);
    QByteArray ligne;
    if (mfile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        while (!mfile.atEnd())
        {
            ligne = mfile.readLine();
            ligne.resize(ligne.size()-1);// pour supprimer le caractère de fin de ligne
            if (ligne.contains("="))
            {
                QList<QByteArray> champs =ligne.split('=');
                if (champs.count()>=2)
                {
                    for (int i=0;i<NB_PARAM;i++) // on essaye de trouver le param correspondant à la ligne
                        if (champs[0] == sConf[i].name)
                        {
                            sConf[i].val=champs[1].toFloat();
                          //  qDebug() << "trouvé " << champs[1] << champs[1].toFloat();
                        }
                }
            }
        }
    }

    mfile.close();
    sendCalibration();
    sendAlarmParameters();
    ui->BlurValue->setValue((int)(sConf[IDX_CONF_BLUR].val));
    ui->ThresholdValue->setValue((int)(sConf[IDX_CONF_SEUIL].val));
    ui->typeVisu->setCurrentIndex((int)(sConf[IDX_CONF_TYPEVISU].val));
    ui->SizeMvtValue->setValue((int)(sConf[IDX_CONF_SIZEMVT].val));
    ui->TimeMvtValue->setValue((int)(sConf[IDX_CONF_TIMEMVT].val));
    ui->AutoONCheck->setChecked(sConf[IDX_CONF_AUTO_ON].val>0.0);
}

void MainFenetre::writeAllParam()
{
    QFile mfile(CONFIG_FILE);
    //QByteArray ligne;
    struct tListAlarmParam s=mMonReveil->getAlarmParameters();
    sConf[IDX_CONF_ALARME_HOUR].val=s.hour;
    sConf[IDX_CONF_ALARME_MINUTE].val=s.minute;
    sConf[IDX_CONF_ALARME_ON].val=s.alarmON;
    sConf[IDX_CONF_ALARME_TYPE].val=s.type;

    if (mfile.open(QIODevice::WriteOnly | QIODevice::Text))
    {

        QTextStream flux(&mfile);
        for (int i=0;i<NB_PARAM;i++)
        {
            if (!sConf[i].name.isNull())
            {
                flux << sConf[i].name << "=" << sConf[i].val <<endl;
            }
        }
        flux << "fin" << endl;
    }
    mfile.close();
}

QString formatNumber(int number)
{
    QString result("");
    if (number >99 || number <0)
        return QString("NN");
    if (number <10)
    {
        result.append("0");
        result.append(QString::number(number));
        return result;
    }
    result.append(QString::number(number));
    return result;
}

// ***********************************************
// ***********   buttons *************************
// ***********************************************

void MainFenetre::checkButton()
{
#if (!EVENT_PROCESS) // on va scruter régulièrement
    if (ui->Mode_button->isDown() || readGPIO(GPIO_BUTTON_MODE))
    {
        if (++mCptButtonMode == 1)
        {
           // qDebug()<<"Mode";
            mMonReveil->ButtonPushed(BUTTON_MODE);
        }
    }
    else
        mCptButtonMode=0;

    if (ui->Plus_button->isDown()|| readGPIO(GPIO_BUTTON_PLUS))
    {
        if ((++mCptButtonPlus)%5 == 1  || (mCptButtonPlus > 15 && mCptButtonPlus%2 ==1))
        {
            mMonReveil->ButtonPushed(BUTTON_PLUS);
        }
    }
    else mCptButtonPlus=0;
    if (ui->Minus_button->isDown() || readGPIO(GPIO_BUTTON_MOINS))
    {
        if ((++mCptButtonMoins)%5 == 1 || (mCptButtonMoins > 15 && mCptButtonMoins%2 ==1))
        {
            mMonReveil->ButtonPushed(BUTTON_MOINS);
        }
    }
    else mCptButtonMoins=0;
    if ((ui->Alarm_button->isDown() || readGPIO(GPIO_BUTTON_ALARME)) && mCptButtonAlarme == 0)
    {
        mCptButtonAlarme = 5; // anti rebond
        mMonReveil->ButtonPushed(BUTTON_ALARME);
    }
    else mCptButtonAlarme=std::max(0,mCptButtonAlarme-1);
#endif
}
// slot appelés depuis l'interface
void MainFenetre::on_InitOpenCV_button_clicked()
{
    sendCalibration();
    if (mCamOn == false)
    {
        mCamOn = true;
        ui->InitOpenCV_button->setText(QString("Stop Camera"));
        trigger->start(100);
    }
    else
    {
        mCamOn = false;
        ui->InitOpenCV_button->setText(QString("Start Camera"));
        trigger->stop();
    }
}

void MainFenetre::on_BlurValue_valueChanged(int arg1)
{
    sConf[IDX_CONF_BLUR].val=arg1;
    sendCalibration();
}

void MainFenetre::on_ThresholdValue_valueChanged(int arg1)
{
    sConf[IDX_CONF_SEUIL].val=arg1;
    sendCalibration();

}

void MainFenetre::on_typeVisu_currentIndexChanged(int index)
{
    sConf[IDX_CONF_TYPEVISU].val=index;
    sendCalibration();
    qDebug() << index;
}

void MainFenetre::on_SizeMvtValue_valueChanged(int arg1)
{
    sConf[IDX_CONF_SIZEMVT].val=arg1;
    sendCalibration();
}

void MainFenetre::on_TimeMvtValue_valueChanged(int arg1)
{
    sConf[IDX_CONF_TIMEMVT].val=arg1;
    sendCalibration();
}


void MainFenetre::on_AutoONCheck_stateChanged(int arg1)
{
    sConf[IDX_CONF_AUTO_ON].val = arg1;
}


void MainFenetre::on_Mode_button_clicked()
{
#if (EVENT_PROCESS)
    mMonReveil->ButtonPushed(BUTTON_MODE);
#endif
}

void MainFenetre::on_Plus_button_clicked()
{
#if (EVENT_PROCESS)
    mMonReveil->ButtonPushed(BUTTON_PLUS);
#endif
}

void MainFenetre::on_Minus_button_clicked()
{
#if (EVENT_PROCESS)
    mMonReveil->ButtonPushed(BUTTON_MOINS);
#endif
}

void MainFenetre::on_Alarm_button_clicked()
{
#if (EVENT_PROCESS)
    mMonReveil->ButtonPushed(BUTTON_ALARME);
#endif

}

void MainFenetre::configGPIO()
{
//#ifdef RASPBERRY_PI
    wiringPiSetupGpio(); // Utilise la numérotation de pin BCM
// bouton plus
    pinMode(GPIO_BUTTON_PLUS, INPUT);
    pullUpDnControl(GPIO_BUTTON_PLUS, PUD_UP);
    pinMode(GPIO_BUTTON_MOINS, INPUT);
    pullUpDnControl(GPIO_BUTTON_MOINS, PUD_UP);
    pinMode(GPIO_BUTTON_MODE, INPUT);
    pullUpDnControl(GPIO_BUTTON_MODE, PUD_UP);
    mTM1637->TMsetup(24,23);
    mTM1637->TMsetBrightness(0);
    mTM1637->TMclear();
    qDebug() << "GPIO et TM OK";
//#endif

}
bool MainFenetre::readGPIO(int button)
{
#ifdef RASPBERRY_PI
    return !digitalRead(button); // on inverse car on est en pull up
#endif
    if (button >0) // pour éviter le warning
        return false;
    else
        return false;

}
void MainFenetre::writeGPIO(int led,bool state)
{
#ifdef RASPBERRY_PI
    digitalWrite(led, state);
#endif
    if (state) // pour éviter le warning
        led=led+0;
}

void MainFenetre::writeTM(int val,int type)
{
    uint8_t tWord[4]={0b01101101,0b01011100,0b01010100,0};
    uint8_t tWordVol[4]={0b00111111,0b01011100,0,0};
//#ifdef RASPBERRY_PI
    switch (type)
    {
    case TM_TYPE_TIME_M:
        mTM1637->TMshowNumber(val,0,true,4,0);
        qDebug() << "affichage";
        break;
    case TM_TYPE_TIME_P:
        mTM1637->TMshowNumber(val,64,true,4,0);
        break;
    case TM_TYPE_REGL_HOUR:
        mTM1637->TMclear();
        mTM1637->TMshowNumber(val,64,true,2,0);
        break;
    case TM_TYPE_REGL_MIN:
        mTM1637->TMclear();
        mTM1637->TMshowNumber(val,64,true,2,2);
        break;
    case TM_TYPE_TYPE:
        mTM1637->TMsetSegments(tWord,4,0);
        break;
    case TM_TYPE_VIDE:
        mTM1637->TMclear();
        break;
    case TM_TYPE_VOL:
//        mTM1637->TMclear();
        tWordVol[3]=val;
        mTM1637->TMsetSegments(tWordVol,4,0);
        break;
    }
//#endif
val=type;type=val; //pour éviter les warnings
return;
}
