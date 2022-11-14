#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    isOn = false;

    timesIndex = -1;

    timer = new QTimer(this);
    timer->setSingleShot(true);

    connect(ui->powerButton, &QPushButton::pressed, this, &MainWindow::startTimer);
    //connect(ui->powerButton, &QPushButton::clicked, this, &MainWindow::selectTime);
    connect(ui->powerButton, &QPushButton::released, this, &MainWindow::stopTimer);
    connect(timer, &QTimer::timeout, this, &MainWindow::powerChange);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startTimer()
{
    if(isOn)
    {
        timer->start(1000);
        selectTime();
    }
    else
    {
        timer->start(1000);
    }
}

void MainWindow::selectTime()
{
    if(isOn)
    {
        if(timesIndex > -1)
            ui->times->itemAt(timesIndex)->widget()->setStyleSheet("");
        timesIndex = (timesIndex + 1) % 4;
        ui->times->itemAt(timesIndex)->widget()->setStyleSheet("background-color:yellow;");

        ui->console->append(QString::number(ui->progressBar->value()));
//        ui->console->append(QString::number((5/8)*100));



        if(ui->progressBar->value() < int((2/8.0)*100))
        {
            ui->battery->itemAt(7)->widget()->setStyleSheet("background-color:red;");
            ui->battery->itemAt(6)->widget()->setStyleSheet("background-color:red;");
            ui->battery->itemAt(5)->widget()->setStyleSheet("");
            ui->battery->itemAt(4)->widget()->setStyleSheet("");
            ui->battery->itemAt(3)->widget()->setStyleSheet("");
            ui->battery->itemAt(2)->widget()->setStyleSheet("");
            ui->battery->itemAt(1)->widget()->setStyleSheet("");
            ui->battery->itemAt(0)->widget()->setStyleSheet("");
        }
        else if(ui->progressBar->value() < int((5/8.0)*100))
        {
            ui->battery->itemAt(7)->widget()->setStyleSheet("background-color:red;");
            ui->battery->itemAt(6)->widget()->setStyleSheet("background-color:red;");
            ui->battery->itemAt(5)->widget()->setStyleSheet("background-color:yellow;");
            ui->battery->itemAt(4)->widget()->setStyleSheet("background-color:yellow;");
            ui->battery->itemAt(3)->widget()->setStyleSheet("background-color:yellow;");
            ui->battery->itemAt(2)->widget()->setStyleSheet("");
            ui->battery->itemAt(1)->widget()->setStyleSheet("");
            ui->battery->itemAt(0)->widget()->setStyleSheet("");
        }
        else
        {
            ui->battery->itemAt(7)->widget()->setStyleSheet("background-color:red;");
            ui->battery->itemAt(6)->widget()->setStyleSheet("background-color:red;");
            ui->battery->itemAt(5)->widget()->setStyleSheet("background-color:yellow;");
            ui->battery->itemAt(4)->widget()->setStyleSheet("background-color:yellow;");
            ui->battery->itemAt(3)->widget()->setStyleSheet("background-color:yellow;");
            ui->battery->itemAt(2)->widget()->setStyleSheet("background-color:green;");
            ui->battery->itemAt(1)->widget()->setStyleSheet("background-color:green;");
            ui->battery->itemAt(0)->widget()->setStyleSheet("background-color:green;");
        }

    }
}

void MainWindow::stopTimer()
{
    timer->stop();
}

void MainWindow::powerChange()
{
    if(isOn)
    {
        isOn = !isOn;
        ui->console->append("off");
    }
   else
    {
        isOn = !isOn;
        ui->console->append("on");
    }
}
