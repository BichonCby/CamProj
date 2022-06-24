#include "mainfenetre.h"
#include "ui_mainfenetre.h"
#include "mavideocapture.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>

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
    sConf[0]={"blursize",5,"Interface"};
    sConf[1]={"seuildiff",35,"Interface"};
    sConf[2]={"typevisu",0,"Interface"};
    sConf[3]={"tailleMvt",10,"Motion"};
    sConf[4]={"timeMvt",5,"Motion"};
    readAllParam();
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
    }
    else if (state == CAM_HS)
    {
        ui->opencvFrame->setText("Pas de caméra");
    }
    return;
}

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
    sendCalibration();
    sConf[0].val=arg1;
}

void MainFenetre::on_ThresholdValue_valueChanged(int arg1)
{
    sendCalibration();
    sConf[1].val=arg1;
}

void MainFenetre::on_typeVisu_currentIndexChanged(int index)
{
    sendCalibration();
    sConf[2].val=index;
}

void MainFenetre::on_SizeMvtValue_valueChanged(int arg1)
{
    sendCalibration();
    sConf[3].val=arg1;
}

void MainFenetre::on_TimeMvtValue_valueChanged(int arg1)
{
    sendCalibration();
    sConf[4].val=arg1;
}

void MainFenetre::sendCalibration()
{
    sListParamUi.blur = sConf[0].val;
    sListParamUi.seuil = sConf[1].val;
    sListParamUi.typevisu = sConf[2].val;
    sListParamUi.sizeMvt = sConf[3].val;
    sListParamUi.timeMvt = sConf[4].val;
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
    //mOpenCV_videoCapture->setVisu((sConf[2].val));// utile?
    ui->BlurValue->setValue((int)(sConf[0].val));
    ui->ThresholdValue->setValue((int)(sConf[1].val));
    ui->typeVisu->setCurrentIndex((int)(sConf[2].val));
    ui->SizeMvtValue->setValue((int)(sConf[3].val));
    ui->TimeMvtValue->setValue((int)(sConf[4].val));
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


