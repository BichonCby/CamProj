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
    sConf[IDX_CONF_SEUIL_LUM_TRES_FAIBLE]={"LumTresFaible",40,"Luminosity"};
    sConf[IDX_CONF_SEUIL_LUM_FAIBLE]={"LumFaible",80,"Luminosity"};
    sConf[IDX_CONF_SEUIL_LUM_MOYENNE]={"LumMoyenne",100,"Luminosity"};
    sConf[IDX_CONF_SEUIL_LUM_FORTE]={"LumForte",140,"Luminosity"};
    sConf[IDX_CONF_SEUIL_LUM_TRES_FORTE]={"LumTresForte",160,"Luminosity"};
    sConf[IDX_CONF_PIN_BTN_PLUS]={"PinBtnPlus",26,"Pinout"};
    sConf[IDX_CONF_PIN_BTN_MOINS]={"PinBtnMoins",11,"Pinout"};
    sConf[IDX_CONF_PIN_BTN_MODE]={"PinBtnMode",13,"Pinout"};
    sConf[IDX_CONF_PIN_BTN_ALARME]={"PinBtnAlarme",12,"Pinout"};
    sConf[IDX_CONF_PIN_LED_ALARME]={"PinLedAlarme",14,"Pinout"};
    sConf[IDX_CONF_PIN_LED_REC]={"PinLedRec",15,"Pinout"};
    sConf[IDX_CONF_PIN_TM_CLK]={"PinTMClk",24,"Pinout"};
    sConf[IDX_CONF_PIN_TM_DIO]={"PinTMDIO",23,"Pinout"};
    sConf[IDX_CONF_CAM_BRIGHTNESS]={"Brightness",50,"Camera"};
    sConf[IDX_CONF_CAM_SATURATION]={"Saturation",50,"Camera"};
    sConf[IDX_CONF_CAM_CONTRAST]={"Contrast",50,"Camera"};
    sConf[IDX_CONF_CAM_EXPOSURE]={"Exposure",1,"Camera"};


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
        writeGPIO(sConf[IDX_CONF_PIN_LED_REC].val,mOpenCV_videoCapture->recording());
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
    sListParamUi.lumLow = sConf[IDX_CONF_SEUIL_LUM_FAIBLE].val;
    sListParamUi.lumMiddle = sConf[IDX_CONF_SEUIL_LUM_MOYENNE].val;
    sListParamUi.lumHigh = sConf[IDX_CONF_SEUIL_LUM_FORTE].val;
    sListParamUi.lumVeryHigh = sConf[IDX_CONF_SEUIL_LUM_TRES_FORTE].val;
    sListParamUi.brightness = sConf[IDX_CONF_CAM_BRIGHTNESS].val;
    sListParamUi.saturation = sConf[IDX_CONF_CAM_SATURATION].val;
    sListParamUi.contrast = sConf[IDX_CONF_CAM_CONTRAST].val;
    sListParamUi.exposure= sConf[IDX_CONF_CAM_EXPOSURE].val;
    sListParamUi.modeFile= sConf[IDX_CONF_MODE_FILE].val;
   // qDebug() << "send calibration  mode file " << sConf[IDX_CONF_MODE_FILE].val;
   // qDebug() << "send calibration  mode file " << sListParamUi.modeFile;

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
    ui->LumLowValue->setValue((int)(sConf[IDX_CONF_SEUIL_LUM_FAIBLE].val));
    ui->LumMiddleValue->setValue((int)(sConf[IDX_CONF_SEUIL_LUM_MOYENNE].val));
    ui->LumHighValue->setValue((int)(sConf[IDX_CONF_SEUIL_LUM_FORTE].val));
    ui->LumVeryHighValue->setValue((int)(sConf[IDX_CONF_SEUIL_LUM_TRES_FORTE].val));
    ui->BrightnessValue->setValue((int)(sConf[IDX_CONF_CAM_BRIGHTNESS].val));
    ui->BrightnessSlider->setValue((int)(sConf[IDX_CONF_CAM_BRIGHTNESS].val));
    ui->SaturationValue->setValue((int)(sConf[IDX_CONF_CAM_SATURATION].val));
    ui->SaturationSlider->setValue((int)(sConf[IDX_CONF_CAM_SATURATION].val));
    ui->ContrastValue->setValue((int)(sConf[IDX_CONF_CAM_CONTRAST].val));
    ui->ContrastSlider->setValue((int)(sConf[IDX_CONF_CAM_CONTRAST].val));
    ui->ExposureValue->setValue((int)(sConf[IDX_CONF_CAM_EXPOSURE].val));
    ui->ExposureSlider->setValue((int)(sConf[IDX_CONF_CAM_EXPOSURE].val));

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
// slot appelés depuis l'interface, on appui sur le bouton start stop, au au demarrage si Auto ON
void MainFenetre::on_InitOpenCV_button_clicked()
{
    //qDebug() << "mode file : " << mModeFichier;
    //qDebug() << "mode cam : " << mCamOn;
    sendCalibration();
    if (mCamOn == false)
    {
        mCamOn = true;
        if (mModeFichier == true) // on est en mode fichier, pas besoin de camera
            ui->InitOpenCV_button->setText(QString("Stop File"));
          else
            ui->InitOpenCV_button->setText(QString("Stop Camera"));
        trigger->start(100);
    }
    else
    {
        mCamOn = false;
        if (mModeFichier == true) // on est en mode fichier, pas besoin de camera
            ui->InitOpenCV_button->setText(QString("Start File"));
        else
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

void MainFenetre::on_LumLowValue_valueChanged(int arg1)
{
    sConf[IDX_CONF_SEUIL_LUM_FAIBLE].val=arg1;
    sendCalibration();
}

void MainFenetre::on_LumMiddleValue_valueChanged(int arg1)
{
    sConf[IDX_CONF_SEUIL_LUM_MOYENNE].val=arg1;
    sendCalibration();
}

void MainFenetre::on_LumHighValue_valueChanged(int arg1)
{
    sConf[IDX_CONF_SEUIL_LUM_FORTE].val=arg1;
    sendCalibration();
}

void MainFenetre::on_LumVeryHighValue_valueChanged(int arg1)
{
    sConf[IDX_CONF_SEUIL_LUM_TRES_FORTE].val=arg1;
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
    pinMode(uint8_t(sConf[IDX_CONF_PIN_BTN_PLUS].val), INPUT);
    pullUpDnControl(GPIO_BUTTON_PLUS, PUD_UP);
    // bouton moins
    pinMode(uint8_t(sConf[IDX_CONF_PIN_BTN_MOINS].val), INPUT);
    pullUpDnControl(GPIO_BUTTON_MOINS, PUD_UP);
    // bouton mode
    pinMode(uint8_t(sConf[IDX_CONF_PIN_BTN_MODE].val), INPUT);
    pullUpDnControl(GPIO_BUTTON_MODE, PUD_UP);
// sorties LED
    pinMode(uint8_t(sConf[IDX_CONF_PIN_LED_ALARME].val), OUTPUT);
    pinMode(uint8_t(sConf[IDX_CONF_PIN_LED_REC].val), INPUT);

    mTM1637->TMsetup(uint8_t(sConf[IDX_CONF_PIN_TM_CLK].val),uint8_t(sConf[IDX_CONF_PIN_TM_DIO].val));
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
    uint8_t TMlum=0;
    float videolum=mOpenCV_videoCapture->luminosity();
    TMlum = 0; // la base, après on regarde les seuils
    if (videolum>uint8_t(sConf[IDX_CONF_SEUIL_LUM_FAIBLE].val)) TMlum=1;
    if (videolum>uint8_t(sConf[IDX_CONF_SEUIL_LUM_MOYENNE].val)) TMlum=3;
    if (videolum>uint8_t(sConf[IDX_CONF_SEUIL_LUM_FORTE].val)) TMlum=5;
    if (videolum>uint8_t(sConf[IDX_CONF_SEUIL_LUM_TRES_FORTE].val)) TMlum=7;
    mTM1637->TMsetBrightness(TMlum);
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
        mTM1637->TMclear();
        //tWordVol[3]=val;
        mTM1637->TMsetSegments(tWordVol,3,1);
        mTM1637->TMshowNumber(val,0,false,1,1);
        break;
    }
//#endif
val=type;type=val; //pour éviter les warnings
return;
}



void MainFenetre::on_BrightnessValue_valueChanged(int arg1)
{
    sConf[IDX_CONF_CAM_BRIGHTNESS].val=arg1;
//    ui->BrightnessValue->setValue((int)(sConf[IDX_CONF_CAM_BRIGHTNESS].val));
    ui->BrightnessSlider->setValue((int)(sConf[IDX_CONF_CAM_BRIGHTNESS].val));
    sendCalibration();
}

void MainFenetre::on_BrightnessSlider_valueChanged(int value)
{
    sConf[IDX_CONF_CAM_BRIGHTNESS].val=value;
    ui->BrightnessValue->setValue((int)(sConf[IDX_CONF_CAM_BRIGHTNESS].val));
    sendCalibration();

}

void MainFenetre::on_ContrastValue_valueChanged(int arg1)
{
    sConf[IDX_CONF_CAM_CONTRAST].val=arg1;
    ui->ContrastSlider->setValue((int)(sConf[IDX_CONF_CAM_CONTRAST].val));
    sendCalibration();

}

void MainFenetre::on_ContrastSlider_valueChanged(int value)
{
    sConf[IDX_CONF_CAM_CONTRAST].val=value;
    ui->ContrastValue->setValue((int)(sConf[IDX_CONF_CAM_CONTRAST].val));
    sendCalibration();

}


void MainFenetre::on_SaturationValue_valueChanged(int arg1)
{
    sConf[IDX_CONF_CAM_SATURATION].val=arg1;
    ui->SaturationSlider->setValue((int)(sConf[IDX_CONF_CAM_SATURATION].val));
    sendCalibration();

}

void MainFenetre::on_SaturationSlider_valueChanged(int value)
{
    sConf[IDX_CONF_CAM_SATURATION].val=value;
    ui->SaturationValue->setValue((int)(sConf[IDX_CONF_CAM_SATURATION].val));
    sendCalibration();

}

void MainFenetre::on_ExposureValue_valueChanged(int arg1)
{
    sConf[IDX_CONF_CAM_EXPOSURE].val=arg1;
    ui->ExposureSlider->setValue((int)(sConf[IDX_CONF_CAM_EXPOSURE].val));
    sendCalibration();

}

void MainFenetre::on_ExposureSlider_valueChanged(int value)
{
    sConf[IDX_CONF_CAM_EXPOSURE].val=value;
    ui->ExposureValue->setValue((int)(sConf[IDX_CONF_CAM_EXPOSURE].val));
    sendCalibration();

}

void MainFenetre::on_FileONCheck_stateChanged(int arg1)
{
    qDebug() << "File mode appui bouton start" << arg1;
    if (arg1 == 2) // on veut jouer le fichier
    {
        sConf[IDX_CONF_MODE_FILE].val=1;
        mCamOn = false;
        mModeFichier = 1;
        ui->InitOpenCV_button->setText(QString("Start File"));
    }
    else
    {
        sConf[IDX_CONF_MODE_FILE].val=0;
        mCamOn = false;
        mModeFichier = 0;
        ui->InitOpenCV_button->setText(QString("Start Camera"));
    }
    sendCalibration();
}

