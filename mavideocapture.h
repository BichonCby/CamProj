#ifndef MAVIDEOCAPTURE_H
#define MAVIDEOCAPTURE_H
#include <QPixmap>
#include <QImage>
#include <QThread>
#include <opencv2/opencv.hpp>

#define ID_CAMERA 0
#define KFIL_LUMINOSITY 0.9
#define KDIFF_NEW_LUMINOSITY 20

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
signals : // les signaux qui lancent des fonctions dans la classe principale
    void NewPixmapCaptured(int a);
protected :
    //void run() override;
private :
    QPixmap mPixmap; // image de QT
    cv::Mat mFrame; //image de OpenCV
    cv::VideoCapture mVideoCapture; // Image de capture
    QImage  cvMatToQImage( const cv::Mat &inMat );
    QPixmap cvMatToQPixmap( const cv::Mat &inMat );

    void getLuminosity();
    void filterLuminosity();
    float mLuminosity;
    float mLuminosityFiltered;
    float mLuminosityMem=0;


};

#endif // MAVIDEOCAPTURE_H
