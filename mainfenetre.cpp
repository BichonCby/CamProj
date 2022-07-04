#include "mainfenetre.h"
#include "ui_mainfenetre.h"
#include "mavideocapture.h"


struct tListParam sListParamUi;

MainFenetre::MainFenetre(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainFenetre)
{
    ui->setupUi(this);
    mOpenCV_videoCapture = new MaVideoCapture(this);
    connect(mOpenCV_videoCapture,&MaVideoCapture::NewPixmapCaptured,this,[&]()
        {
            DisplayVideo(mOpenCV_videoCapture->stateCamera());
        });
    trigger = new QTimer(this);
    connect (trigger, &QTimer::timeout, this, &MainFenetre::ticHorloge);

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
    readAllParam();
    // si Auto ON est configuré, on lance directement le processus
    if (sConf[IDX_CONF_AUTO_ON].val)
        on_InitOpenCV_button_clicked();
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

void MainFenetre::sendCalibration()
{
    sListParamUi.blur = sConf[IDX_CONF_BLUR].val;
    sListParamUi.seuil = sConf[IDX_CONF_SEUIL].val;
    sListParamUi.typevisu = sConf[IDX_CONF_TYPEVISU].val;
    sListParamUi.sizeMvt = sConf[IDX_CONF_SIZEMVT].val;
    sListParamUi.timeMvt = sConf[IDX_CONF_TIMEMVT].val;
    mOpenCV_videoCapture->setCalibration(sListParamUi);
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
   // for (int i=0;i<2;i++)
   //     qDebug() << sConf[i].name <<  " " << sConf[i].val;
    mfile.close();
    sendCalibration();
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



