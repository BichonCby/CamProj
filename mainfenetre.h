#ifndef MAINFENETRE_H
#define MAINFENETRE_H

#include <QMainWindow>
#include <QTimer>
#include "qled.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <string>
#include <QDateTime>
#include <QThread>

#ifdef RASPBERRY_PI
    #include <wiringPi.h>
#endif

#define EVENT_PROCESS 0

#define CAM_OFF 0
#define CAM_OK 1
#define CAM_HS -1

#define NB_PARAM 20 // nombre max de paramètres dans le fichier de conf
#define CONFIG_FILE "config.ini"

#define IDX_CONF_BLUR 0
#define IDX_CONF_SEUIL 1
#define IDX_CONF_TYPEVISU 2
#define IDX_CONF_SIZEMVT 3
#define IDX_CONF_TIMEMVT 4
#define IDX_CONF_AUTO_ON 5
#define IDX_CONF_ALARME_HOUR 6
#define IDX_CONF_ALARME_MINUTE 7
#define IDX_CONF_ALARME_ON 8
#define IDX_CONF_ALARME_TYPE 9

#define GPIO_BUTTON_PLUS 26
#define GPIO_BUTTON_MOINS 11
#define GPIO_BUTTON_MODE 11
#define GPIO_BUTTON_ALARME 11

namespace Ui {
class MainFenetre;
}

struct tConf
{
    QString name;
    float val;
    QString module;
};

QString formatNumber(int n);

class MonReveil;
class MaVideoCapture;

class MainFenetre : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainFenetre(QWidget *parent = 0);
    ~MainFenetre();

private slots:
    void on_InitOpenCV_button_clicked();

    void on_BlurValue_valueChanged(int arg1);

    void on_ThresholdValue_valueChanged(int arg1);

    void on_typeVisu_currentIndexChanged(int index);

    void on_SizeMvtValue_valueChanged(int arg1);

    void on_TimeMvtValue_valueChanged(int arg1);

    void on_AutoONCheck_stateChanged(int arg1);

    void on_Mode_button_clicked();

    void on_Plus_button_clicked();

    void on_Minus_button_clicked();

    void on_Alarm_button_clicked();

private:
    void ticHorloge();
    void checkButton();
    void readAllParam();
    void writeAllParam();
    void sendCalibration();
    void sendAlarmParameters();
    void configGPIO();
    bool readGPIO(int button);
    void writeGPIO(int led,bool state);
    Ui::MainFenetre *ui;
    MaVideoCapture *mOpenCV_videoCapture;
    MonReveil *mMonReveil;
    void DisplayVideo(int a = 0);
    void DisplayClock(QString t);
    void DisplayAlarm(bool a);
    bool mCamOn = false;
    QTimer *trigger;
    QTimer *buttonTimer;
    struct tConf sConf[NB_PARAM];
    int mCptButtonMode=0;
    int mCptButtonPlus=0;
    int mCptButtonMoins=0;
    int mCptButtonAlarme=0;
};

#endif // MAINFENETRE_H
