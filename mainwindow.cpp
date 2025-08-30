#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QTime>
#include <QDateTime>
#include <QDebug>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSoundEffect>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , wrongAttempts(0)
{
    ui->setupUi(this);
    correctPIN ="1234";
    isArmed = false;

    // Show "Enter PIN" placeholder, no dots initially
    ui->lineEdit_pin->setPlaceholderText("Enter PIN");
    ui->lineEdit_pin->setEchoMode(QLineEdit::Normal);

    // Switch to dots only when typing
    connect(ui->lineEdit_pin, &QLineEdit::textChanged, this, [=](const QString &text) {
        if (!text.isEmpty()) {
            ui->lineEdit_pin->setEchoMode(QLineEdit::Password);
        } else {
            ui->lineEdit_pin->setEchoMode(QLineEdit::Normal);
        }
    });



    qDebug() << "✅ correctPIN is set to:" << correctPIN;
    isArmed = false;
    ui->label_systemStatus_->setText("System Disarmed");
    ui->label_systemStatus_->setStyleSheet("color: blue; font-weight: bold;");
    ui->label_doorStatus->setText("Door Unlocked");
    alarmVisible = false;

    // (… rest of your existing code …)


    // Audio setup
    chimePlayer = new QMediaPlayer(this);
    chimeOutput = new QAudioOutput(this);
    chimeOutput->setVolume(100);
    chimePlayer->setAudioOutput(chimeOutput);
    chimePlayer->setSource(QUrl("qrc:/assets/sounds/chimesound.wav"));

    buzzerPlayer = new QMediaPlayer(this);
    buzzerOutput = new QAudioOutput(this);
    buzzerOutput->setVolume(100);
    buzzerPlayer->setAudioOutput(buzzerOutput);
    buzzerPlayer->setSource(QUrl("qrc:/assets/sounds/buzzersound.wav"));



    ui->lineEdit_pin->setPlaceholderText("Enter PIN");

    connect(ui->button_camera, &QPushButton::clicked, this, &MainWindow::on_button_camera_clicked);
    connect(ui->button_submit, &QPushButton::clicked, this, &MainWindow::on_button_submit_clicked);
    connect(ui->button_reset, &QPushButton::clicked, this, &MainWindow::on_button_reset_clicked);

    alarmTimer = new QTimer(this);
    connect(alarmTimer, &QTimer::timeout, this, &MainWindow::toggleAlarm);

    clockTimer = new QTimer(this);
    connect(clockTimer, &QTimer::timeout, this, &MainWindow::updateClock);
    clockTimer->start(1000);
    updateClock();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::lockInput()
{
    ui->lineEdit_pin->setEnabled(false);
    ui->button_submit->setEnabled(false);

    updateStatus("🚨 ALARM TRIGGERED!", "color: red; font-weight: bold;");
    alarmVisible = true;
    alarmTimer->start(500);  // blinking effect

    QTimer::singleShot(5000, this, [=]() {
        alarmTimer->stop();

        QString statusText = isArmed ? "🟢 System Armed" : "🔵 System Disarmed";
        QString statusColor = isArmed ? "color: green; font-weight: bold;" : "color: blue; font-weight: bold;";
        updateStatus(statusText, statusColor);

        ui->lineEdit_pin->setEnabled(true);
        ui->button_submit->setEnabled(true);
        ui->label_pinStatus->setText("🔓 You can try again.");
        ui->label_pinStatus->setStyleSheet("color: green; font-weight: bold;");
        wrongAttempts = 0;

        QTimer::singleShot(2000, this, &MainWindow::resetFields);
    });
}

void MainWindow::toggleAlarm()
{
    alarmVisible = !alarmVisible;
    if (alarmVisible) {
        ui->label_pinStatus->setText("🚨 INTRUDER ALERT 🚨");
        ui->label_pinStatus->setStyleSheet("color: red; font-weight: bold; font-size: 18px;");
    } else {
        ui->label_pinStatus->clear();
    }
}

void MainWindow::updateClock() {
    // Get current date and time
    QDateTime current = QDateTime::currentDateTime();

    // Format date and time separately
    QString currentDate = current.date().toString("dd-MM-yyyy");  // Example: 24-08-2025
    QString timeText = current.time().toString("hh:mm:ss AP");       // Example: 12:45:30

    // Update UI labels
    ui->label_date->setText("📅 " + currentDate);
    ui->label_clock->setText("⏰ " + timeText);
}




void MainWindow::on_button_submit_clicked()
{
    QString enteredPIN = ui->lineEdit_pin->text().trimmed();
    qDebug() << "Entered PIN:" << enteredPIN << "Length:" << enteredPIN.length();
    qDebug() << "Correct PIN:" << correctPIN << "Length:" << correctPIN.length();

    if (enteredPIN.isEmpty()) {
        qDebug() << "⚠️ Empty PIN entered, ignoring.";
        return;   // Prevents false "Incorrect PIN" after clearing
    }
    if (enteredPIN == correctPIN)
      {
        qDebug() << "✅ PIN matched. Arming/disarming system...";
        wrongAttempts = 0;
        chimePlayer->play();

        isArmed = !isArmed;

        qDebug() << "System armed status:" << isArmed;

        // Fade effect
        if (isArmed)
        {
            ui->label_systemStatus_->setText("System Armed");
            ui->label_systemStatus_->setStyleSheet("color: green; font-weight: bold;");
            ui->label_doorStatus->setStyleSheet("QLabel { color : green; font-weight: bold; }");
            ui->label_doorStatus->setText("Door Locked");
        }
        else
        {
            ui->label_systemStatus_->setText("System Disarmed");
            ui->label_systemStatus_->setStyleSheet("color: blue; font-weight: bold;");
            ui->label_doorStatus->setStyleSheet("QLabel { color : blue; font-weight: bold; }");
            ui->label_doorStatus->setText("Door Unlocked");

        }

        // Now apply fade animation after updating text
        auto* effect = new QGraphicsOpacityEffect(ui->label_systemStatus_);
        ui->label_systemStatus_->setGraphicsEffect(effect);
        effect->setOpacity(0);
        QPropertyAnimation* anim = new QPropertyAnimation(effect, "opacity");
        anim->setDuration(500);
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        anim->start(QAbstractAnimation::DeleteWhenStopped);


        ui->label_pinStatus->clear();
        ui->label_pinStatus->setText("✅ Access Granted");
        ui->label_pinStatus->setStyleSheet("color: green; font-weight: bold;");
    }
    else {
        qDebug() << "❌ PIN mismatch. Triggering alarm.";
        buzzerPlayer->play();
        wrongAttempts++;

        ui->label_pinStatus->clear();
        ui->label_pinStatus->setText("❌ Incorrect PIN");
        ui->label_pinStatus->setStyleSheet("color: red; font-weight: bold;");
        QPropertyAnimation* animation = new QPropertyAnimation(ui->lineEdit_pin, "pos");
        animation->setDuration(300);
        animation->setKeyValueAt(0, ui->lineEdit_pin->pos());
        animation->setKeyValueAt(0.25, ui->lineEdit_pin->pos() + QPoint(-10, 0));
        animation->setKeyValueAt(0.5, ui->lineEdit_pin->pos() + QPoint(10, 0));
        animation->setKeyValueAt(0.75, ui->lineEdit_pin->pos() + QPoint(-10, 0));
        animation->setKeyValueAt(1, ui->lineEdit_pin->pos());
        animation->start(QAbstractAnimation::DeleteWhenStopped);

        if (wrongAttempts >= 3) {
            ui->label_pinStatus->setText("🔒 Too many attempts! Locked for 5 seconds.");
            ui->label_pinStatus->setStyleSheet("color: red; font-weight: bold;");
            lockInput();
            return;
        }
        qDebug() << "🔚 End of submit click handler";

    }

    ui->lineEdit_pin->clear();

}

void MainWindow::on_button_reset_clicked()
{
    resetFields();
}

void MainWindow::resetFields()
{
    ui->lineEdit_pin->clear();
    ui->label_pinStatus->clear();
    ui->lineEdit_pin->setFocus();
}

void MainWindow::updateStatus(const QString &text, const QString &style)
{
    ui->label_systemStatus_->setText(text);
    ui->label_systemStatus_->setStyleSheet(style);
}

void MainWindow::on_button_camera_clicked()
{
    QPixmap pixmap(":/assets/images/home_view.png");
    QPixmap scaledPixmap = pixmap.scaled(400, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QLabel* imageLabel = new QLabel;
    imageLabel->setPixmap(scaledPixmap);
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Simulated Camera View");
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    layout->addWidget(imageLabel);
    dialog->setLayout(layout);
    dialog->exec();
}
void MainWindow::animateSystemStatus(bool armed)
{
    QString color = armed ? "#00FF00" : "#0000FF"; // Green or Blue
    QString style = QString("QLabel { background-color: %1; color: white; padding: 10px; border-radius: 10px; }").arg(color);

    ui->label_systemStatus_->setStyleSheet(style);
    ui->label_doorStatus->setStyleSheet(style);
}
