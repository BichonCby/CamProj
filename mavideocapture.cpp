#include "mavideocapture.h"
#include <QDebug>

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
        mPixmap = cvMatToQPixmap(mFrame);
        getLuminosity();
        filterLuminosity();
        emit NewPixmapCaptured(0);
    }
}
void MaVideoCapture::getLuminosity()
{
    cv::Mat grayMat;
    cv::cvtColor(mFrame, grayMat, CV_BGR2GRAY);

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
    }
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
