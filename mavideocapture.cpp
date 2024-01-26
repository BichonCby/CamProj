#include "mavideocapture.h"



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
        ManageRecording();

        switch (mTypeVisu)
        {
        case 0:
            mPixmap = cvMatToQPixmap(mFrame);break;
        case 1:
            mPixmap = cvMatToQPixmap(mFrameBlurred);break;
        case 2:
            mPixmap = cvMatToQPixmap(mFrameDiff);break;
        case 3:
        default:
            mPixmap = cvMatToQPixmap(mFrameNull);break;
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
        QString fname="/tmp/luminosity.txt";
        QFile file(fname);
        if (file.open(QIODevice::ReadWrite))
        {
            QTextStream stream (&file);
            stream << mLuminosityMem;
            file.close();
        }
    }
}
#define MAX_TIME_DETECT 500

// fonction de détection du mouvement par différence
void MaVideoCapture::detectMotion()
{
    // on floutte l'image pour limiter les écarts
    cv::GaussianBlur(grayMat,mFrameBlurred,cv::Size(mBlurVal,mBlurVal),0);
    if (!mFrameBlurredPrev.empty())
    {
        // si on a fait déjà un premier pas, on fait la différence absolue d'images successives
        mFrameDiff = cv::abs(mFrameBlurred-mFrameBlurredPrev);
        // on met en blanc ce qui est vraiment différent (au dessus d'un seuil)
        cv::threshold(mFrameDiff,mFrameDiff,mThresholdVal,255,cv::THRESH_BINARY);
        mMouvement = false;
        if (cv::countNonZero(mFrameDiff)> mSizeMvt)
        {
            // si on a beaucoup de pixels différents, il y a mouvement
            // on incrémente le compteur
            mCptMvt = std::min(mCptMvt+1,MAX_TIME_DETECT);
            mMouvement = true;
        }
        else
        {
            // sinon on décrémente le compteur
            mCptMvt = std::max(mCptMvt-1,0);
        }
        mRecordRequest = false;
        if (mCptMvt >= mTimeMvt)
        {
            // longue détection, on commence l'enregistrement
            mRecordRequest = true;
        }
    }
    else
    {
        qDebug() << "premier pas";
    }
}

// gestion de l'enregistrement
// ouverture du fichie
// enregistrement de l'image en cours
// fermeture du fichier

void MaVideoCapture::ManageRecording()
{
    if (mRecordRequest & !mRecording) // on va démarrer une nouvelle video
    {
        QDateTime date=QDateTime::currentDateTime();
        qDebug() << date.time().hour() << date.time().minute();
        QDir dir(".");
        const QString year_t = QString::number(date.date().year());
        QString month_t=formatNumber(date.date().month());
        QString day_t=formatNumber(date.date().day());
        QString hour_t=formatNumber(date.time().hour());
        QString minute_t=formatNumber(date.time().minute());
        QString second_t=formatNumber(date.time().second());

        filename1.clear();filename1.append(year_t);
        filename1.append("/");filename1.append(month_t);filename1.append("/");filename1.append(day_t);
        qDebug() << filename1;
        if (!dir.exists(filename1))
        {
            qDebug()<< "creation répertoire";
            qDebug()<< dir.mkpath(filename1);
        }
        filename1.append("/rec_");
        filename1.append(year_t);
        filename1.append(month_t);
        filename1.append(day_t);
        filename1.append("_");
        filename1.append(hour_t);
        filename1.append(minute_t);
        filename1.append(second_t);
        filename1.append(".avi");

        mVideoWriter.open(filename1.toStdString(),CV_FOURCC('D','I','V','X'),10.0,cv::Size(640,480),true) ;
        mRecording = true;
        qDebug() << "fichier créé" << filename1;
    }
    else if (!mRecordRequest && mRecording) // fin d'enregistremeent
    {
        qDebug() <<  "fin d'enregistremeent";
        mVideoWriter.release();
        mRecording = false;
        // vérification de la taille du fichier
        QFile fic(filename1);
        if (fic.size()<1000000) //taille en bit
            fic.remove();
        //qDebug() << QString::number(fic.size());

    }
    else if (mRecordRequest) // enregistremeent en cours
    {
        //qDebug() << "encours";
        mVideoWriter.write(mFrame);
    }
    // sinon on fait rien.
}
void MaVideoCapture::setCalibration(struct tListParam s)
{
    mBlurVal=s.blur;
    mThresholdVal=s.seuil;
    mTypeVisu = s.typevisu;
    mTimeMvt = s.timeMvt;
    mSizeMvt = s.sizeMvt;
    mVideoCapture.set(cv::CAP_PROP_BRIGHTNESS,((double)s.brightness)/100.0);
    mVideoCapture.set(cv::CAP_PROP_CONTRAST,((double)s.contrast)/100.0);
    mVideoCapture.set(cv::CAP_PROP_SATURATION,((double)s.saturation)/100.0);
    mVideoCapture.set(cv::CAP_PROP_EXPOSURE,s.exposure);
    qDebug() << "parametre cam CAP_PROP_BRIGHTNESS " << QString::number(mVideoCapture.get(cv::CAP_PROP_BRIGHTNESS));
    qDebug() << "parametre cam CAP_PROP_CONTRAST " << QString::number(mVideoCapture.get(cv::CAP_PROP_CONTRAST));
    qDebug() << "parametre cam CAP_PROP_SATURATION " << QString::number(mVideoCapture.get(cv::CAP_PROP_SATURATION));
//    qDebug() << "parametre cam CAP_PROP_HUE " << QString::number(mVideoCapture.get(cv::CAP_PROP_HUE));
//    qDebug() << "parametre cam CAP_PROP_GAIN " << QString::number(mVideoCapture.get(cv::CAP_PROP_GAIN));
    qDebug() << "parametre cam CAP_PROP_EXPOSURE " << QString::number(mVideoCapture.get(cv::CAP_PROP_EXPOSURE));
//    qDebug() << "parametre cam CAP_PROP_WHITE_BALANCE_BLUE_U " << QString::number(mVideoCapture.get(cv::CAP_PROP_WHITE_BALANCE_BLUE_U));
//    qDebug() << "parametre cam CAP_PROP_SHARPNESS " << QString::number(mVideoCapture.get(cv::CAP_PROP_SHARPNESS));
//    qDebug() << "parametre cam CAP_PROP_AUTO_EXPOSURE " << QString::number(mVideoCapture.get(cv::CAP_PROP_AUTO_EXPOSURE));
//    qDebug() << "parametre cam CAP_PROP_ZOOM " << QString::number(mVideoCapture.get(cv::CAP_PROP_ZOOM));
//    qDebug() << "parametre cam CAP_PROP_FOCUS " << QString::number(mVideoCapture.get(cv::CAP_PROP_FOCUS));
//    qDebug() << "parametre cam CAP_PROP_AUTOFOCUS " << QString::number(mVideoCapture.get(cv::CAP_PROP_AUTOFOCUS));
    qDebug() << "parametre cam CAP_PROP_FRAME_WIDTH " << QString::number(mVideoCapture.get(cv::CAP_PROP_FRAME_WIDTH));


}

// fonctions de transformation, copier coller
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
