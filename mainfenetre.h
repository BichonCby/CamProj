#ifndef MAINFENETRE_H
#define MAINFENETRE_H

#include <QMainWindow>
#include <QTimer>

#define CAM_OFF 0
#define CAM_OK 1
#define CAM_HS -1

#define NB_PARAM 20 // nombre max de paramètres dans le fichier de conf
#define CONFIG_FILE "config.ini"

namespace Ui {
class MainFenetre;
}

struct tConf
{
    QString name;
    float val;
    QString module;
};

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

private:
    void ticHorloge();
    void readAllParam();
    void writeAllParam();
    void sendCalibration();

    Ui::MainFenetre *ui;
    MaVideoCapture *mOpenCV_videoCapture;
    void DisplayVideo(int a = 0);
    bool mCamOn = false;
    QTimer *trigger;
    struct tConf sConf[NB_PARAM];
};

#endif // MAINFENETRE_H
