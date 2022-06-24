#include "mavideocapture.h"
#include <QDebug>
#include <opencv2/core/types.hpp>

struct tListParam sListParamCam;

MaVideoCapture::MaVideoCapture(QObject *parent)
    : QThread {parent}
    , mVideoCapture {ID_CAMERA}
{

}

void MaVideoCapture::loopTreatment()
{
    mVideoCapture >> mFrame;
    if (!mFrame.empty())
    {
        cv::cvtColor(mFrame, grayMat, CV_BGR2GRAY);
        getLuminosity();
        filterLuminosity();
        detectMotion();

        switch (mTypeVisu)
        {
        case 0:
        default:
            mPixmap = cvMatToQPixmap(mFrame);break;
        case 1:
            mPixmap = cvMatToQPixmap(mFrameBlurred);break;
        case 2:
            mPixmap = cvMatToQPixmap(mFrameDiff);break;
        }
        //mPixmap = cvMatToQPixmap(mFrameDiff);//mFrame);
        mFrameBlurredPrev = mFrameBlurred.clone();
        mStateCamera = CAM_OK;
    }
    else
    {
        mStateCamera = CAM_HS;
    }
    emit NewPixmapCaptured(mStateCamera);

}
void MaVideoCapture::getLuminosity()
{

    int Totalintensity = 0;
    for (int i=0; i < grayMat.rows; ++i){
        for (int j=0; j < grayMat.cols; ++j){
            Totalintensity += (int)grayMat.at<uchar>(i, j);
        }
    }

    // Find avg lum of frame
    float avgLum = Totalintensity/(grayMat.rows * grayMat.cols);
    mLuminosity = avgLum;
    return;
}
void MaVideoCapture::filterLuminosity()
{
    mLuminosityFiltered = KFIL_LUMINOSITY*mLuminosity+(1-KFIL_LUMINOSITY)*mLuminosity;
    if (abs(mLuminosityFiltered-mLuminosityMem)>KDIFF_NEW_LUMINOSITY)
    {
        mLuminosityMem = mLuminosityFiltered;
        qDebug() << "nouvelle luminosité " << mLuminosityMem;
        // ajouter l'ecriture dans un fichier de la correspondance avec l'éclairage de l'heure
    }
}
#define MAX_TIME_DETECT 500
void MaVideoCapture::detectMotion()
{
    cv::GaussianBlur(grayMat,mFrameBlurred,cv::Size(mBlurVal,mBlurVal),0);
    if (!mFrameBlurredPrev.empty())
    {
//        cv::absdiff(mFrameBlurred,mFrameBlurredPrev,mFrameDiff);
        mFrameDiff = cv::abs(mFrameBlurred-mFrameBlurredPrev);
//        if (cv::countNonZero(mFrameBlurred != mFrameBlurredPrev)==0)
//            qDebug() << "identique";
        cv::threshold(mFrameDiff,mFrameDiff,mThresholdVal,255,cv::THRESH_BINARY);
        if (cv::countNonZero(mFrameDiff)> mSizeMvt)
        {
            mCptMvt = std::min(mCptMvt+1,MAX_TIME_DETECT);
            qDebug() << "mouvement";
        }
        else
        {
            mCptMvt = std::max(mCptMvt-1,0);
        }

        if (mCptMvt >= mTimeMvt)
        {
            qDebug() << "enregistremeent";
        }
    }
    else
    {
        qDebug() << "premier pas";
    }
}

void MaVideoCapture::ManageRecording()
{

}
void MaVideoCapture::setCalibration(struct tListParam s)
{
    mBlurVal=s.blur;
    mThresholdVal=s.seuil;
    mTypeVisu = s.typevisu;
    mTimeMvt = s.timeMvt;
    mSizeMvt = s.sizeMvt;

}
void MaVideoCapture::setVisu(int index)
{
    mTypeVisu = index;
}
QImage  MaVideoCapture::cvMatToQImage( const cv::Mat &inMat )
{
    switch ( inMat.type() )
    {
    // 8-bit, 4 channel
    case CV_8UC4:
    {
        QImage image( inMat.data,
                      inMat.cols, inMat.rows,
                      static_cast<int>(inMat.step),
                      QImage::Format_ARGB32 );

        return image;
    }

        // 8-bit, 3 channel
    case CV_8UC3:
    {
        QImage image( inMat.data,
                      inMat.cols, inMat.rows,
                      static_cast<int>(inMat.step),
                      QImage::Format_RGB888 );

        return image.rgbSwapped();
    }

        // 8-bit, 1 channel
    case CV_8UC1:
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
        QImage image( inMat.data,
                      inMat.cols, inMat.rows,
                      static_cast<int>(inMat.step),
                      QImage::Format_Grayscale8 );
#else
        static QVector<QRgb>  sColorTable;

        // only create our color table the first time
        if ( sColorTable.isEmpty() )
        {
            sColorTable.resize( 256 );

            for ( int i = 0; i < 256; ++i )
            {
                sColorTable[i] = qRgb( i, i, i );
            }
        }

        QImage image( inMat.data,
                      inMat.cols, inMat.rows,
                      static_cast<int>(inMat.step),
                      QImage::Format_Indexed8 );

        image.setColorTable( sColorTable );
#endif

        return image;
    }

    default:
        qWarning() << "ASM::cvMatToQImage() - cv::Mat image type not handled in switch:" << inMat.type();
        break;
    }

    return QImage();
}

QPixmap MaVideoCapture::cvMatToQPixmap( const cv::Mat &inMat )
{
    return QPixmap::fromImage( cvMatToQImage( inMat ) );
}
