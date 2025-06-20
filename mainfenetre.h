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
#include <opencv2/core/types.hpp>

#ifdef RASPBERRY_PI
    #include <wiringPi.h>
    #include "tm1637.h"
#endif
#include "tm1637.h"
#define EVENT_PROCESS 0

#define CAM_OFF 0
#define CAM_OK 1
#define CAM_HS -1

#define NB_PARAM 40 // nombre max de paramètres dans le fichier de conf
#define CONFIG_FILE "config.ini"

// index de configuration
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
#define IDX_CONF_SEUIL_LUM_TRES_FAIBLE 10
#define IDX_CONF_SEUIL_LUM_FAIBLE 11
#define IDX_CONF_SEUIL_LUM_MOYENNE 12
#define IDX_CONF_SEUIL_LUM_FORTE 13
#define IDX_CONF_SEUIL_LUM_TRES_FORTE 14
#define IDX_CONF_PIN_BTN_PLUS 15
#define IDX_CONF_PIN_BTN_MOINS 16
#define IDX_CONF_PIN_BTN_MODE 17
#define IDX_CONF_PIN_BTN_ALARME 18
#define IDX_CONF_PIN_LED_ALARME 19
#define IDX_CONF_PIN_LED_REC 20
#define IDX_CONF_PIN_TM_CLK 21
#define IDX_CONF_PIN_TM_DIO 22
#define IDX_CONF_CAM_BRIGHTNESS 23
#define IDX_CONF_CAM_SATURATION 24
#define IDX_CONF_CAM_CONTRAST 25
#define IDX_CONF_CAM_EXPOSURE 26
#define IDX_CONF_MODE_FILE 27

// pin GPIO
#define GPIO_BUTTON_PLUS 26
#define GPIO_BUTTON_MOINS 11
#define GPIO_BUTTON_MODE 11
#define GPIO_BUTTON_ALARME 11

#define TM_TYPE_TIME_P  0
#define TM_TYPE_TIME_M  1
#define TM_TYPE_REGL_HOUR   2
#define TM_TYPE_REGL_MIN    3
#define TM_TYPE_VOL 4
#define TM_TYPE_TYPE    5
#define TM_TYPE_VIDE 10
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

    void on_LumLowValue_valueChanged(int arg1);

    void on_LumMiddleValue_valueChanged(int arg1);

    void on_LumHighValue_valueChanged(int arg1);

    void on_LumVeryHighValue_valueChanged(int arg1);

    void on_BrightnessValue_valueChanged(int arg1);

    void on_ContrastValue_valueChanged(int arg1);

    void on_ContrastSlider_valueChanged(int value);

    void on_BrightnessSlider_valueChanged(int value);

    void on_SaturationValue_valueChanged(int arg1);

    void on_SaturationSlider_valueChanged(int value);

    void on_ExposureValue_valueChanged(int arg1);

    void on_ExposureSlider_valueChanged(int value);

    void on_FileONCheck_stateChanged(int arg1);

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
    void writeTM(int val,int type);
    Ui::MainFenetre *ui;
    MaVideoCapture *mOpenCV_videoCapture;
    MonReveil *mMonReveil;
    tm1637 *mTM1637;
    void DisplayVideo(int a = 0);
    void DisplayClock(QString t);
    void DisplayAlarm(bool a);
    bool mCamOn = false;
    bool mModeFichier = false; // mode fichier pour traiter une video et pas le flux camera
    QTimer *trigger;
    QTimer *buttonTimer;
    struct tConf sConf[NB_PARAM];
    int mCptButtonMode=0;
    int mCptButtonPlus=0;
    int mCptButtonMoins=0;
    int mCptButtonAlarme=0;
};

#endif // MAINFENETRE_H
