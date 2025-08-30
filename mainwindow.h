#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QPixmap>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSoundEffect>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateStatus(const QString &text, const QString &style);

private slots:
    void resetFields();
    void lockInput();
    void toggleAlarm();
    void updateClock();
    void on_button_camera_clicked();
    void on_button_submit_clicked();
    void on_button_reset_clicked();



private:
    Ui::MainWindow *ui;
    QString correctPIN ="1234";
    int wrongAttempts = 0;
    bool isArmed;
    bool alarmVisible = false;
    void animateColor(QWidget *widget, QColor color);
    void shakeWidget(QWidget *widget);



    QTimer *alarmTimer;
    QTimer *clockTimer;
    QMediaPlayer* chimePlayer;
    QAudioOutput* chimeOutput;

    QMediaPlayer* buzzerPlayer;
    QAudioOutput* buzzerOutput;
    QSoundEffect* soundEffect;


    QPixmap cameraFeed;

    void animateSystemStatus(bool isArmed);
};

#endif // MAINWINDOW_H
