#ifndef MAINFENETRE_H
#define MAINFENETRE_H

#include <QMainWindow>
#include <QTimer>

namespace Ui {
class MainFenetre;
}

class MaVideoCapture;

class MainFenetre : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainFenetre(QWidget *parent = 0);
    ~MainFenetre();

private slots:
    void on_InitOpenCV_button_clicked();

private:
    void ticHorloge();
    Ui::MainFenetre *ui;
    MaVideoCapture *mOpenCV_videoCapture;
    void DisplayVideo(int a = 0);
    bool mCamOn = false;
    QTimer *trigger;
};

#endif // MAINFENETRE_H
