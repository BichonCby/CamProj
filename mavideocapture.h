#ifndef MAVIDEOCAPTURE_H
#define MAVIDEOCAPTURE_H
#include <QPixmap>
#include <QImage>
#include <opencv2/opencv.hpp>
#include "mainfenetre.h"
#include <opencv2/core/types.hpp>

#define ID_CAMERA 0
#define KFIL_LUMINOSITY 0.9
#define KDIFF_NEW_LUMINOSITY 20

struct tListParam
{
    int blur;
    int seuil;
    int typevisu;
    int timeMvt;
    int sizeMvt;
};


class MaVideoCapture : public QThread
{
    Q_OBJECT
public:
    MaVideoCapture(QObject *parent = nullptr);
    QPixmap pixmap() const
    {
        return mPixmap;
    }
    float luminosity() const
    {
        return mLuminosity;
    }
    void loopTreatment();

    // accesseurs simples
    int stateCamera() const  {return mStateCamera;}
    bool recording() const {return mRecordRequest;}
    bool mouvement() const {return mMouvement;}
    void setCalibration(struct tListParam s);

signals : // les signaux qui lancent des fonctions dans la classe principale
    void NewPixmapCaptured(int a);
private :
    QPixmap mPixmap; // image de QT
    cv::Mat mFrame; //image de OpenCV
    cv::Mat mFrameBlurred; // image foutée
    cv::Mat mFrameBlurredPrev; // ancienne image floutée pour motion detection
    cv::Mat grayMat;
    cv::Mat mFrameDiff; //difference de frame pour motion detection
    cv::VideoCapture mVideoCapture; // Image de capture
    cv::VideoWriter mVideoWriter; // pour l'enregistrement
    QImage  cvMatToQImage( const cv::Mat &inMat );
    QPixmap cvMatToQPixmap( const cv::Mat &inMat );

    void getLuminosity();
    void filterLuminosity();
    void detectMotion();
    void ManageRecording();
    float mLuminosity;
    float mLuminosityFiltered;
    float mLuminosityMem=0;

    int mStateCamera = CAM_OFF;
    int mCptMvt=0;
    bool mRecording = false;
    bool mRecordRequest = false;
    bool mMouvement = false;
    double mBlurVal;
    double mThresholdVal;
    int mTypeVisu=0;
    int mTimeMvt;
    int mSizeMvt;
    QString filename1;
};

#endif // MAVIDEOCAPTURE_H
