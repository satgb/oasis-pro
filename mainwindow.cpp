#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    groups.append(new Group(20, {0, 2, 4, 6}));
    groups.append(new Group(45, {1, 2, 3, 4}));
    groups.append(new Group(180, {1, 3, 5, 7}));

    isOn = false;

    groupsIndex = -1;
    sessionsIndex = -1;

    timer = new QTimer(this);
    timer->setSingleShot(true);

    connect(ui->powerButton, &QPushButton::pressed, this, &MainWindow::startTimer);
    //connect(ui->powerButton, &QPushButton::clicked, this, &MainWindow::selectGroup);
    connect(ui->powerButton, &QPushButton::released, this, &MainWindow::stopTimer);
    connect(ui->upButton, &QPushButton::pressed, this, &MainWindow::upPress);
    connect(ui->downButton, &QPushButton::pressed, this, &MainWindow::downPress);

    connect(ui->selectButton, &QPushButton::pressed, this, &MainWindow::selectSession);

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
        selectGroup();
    }
    else
    {
        timer->start(1000);
    }
}

void MainWindow::upPress()
{
    if(isOn)
    {
        if(groupsIndex > -1)
        {
            QList<int> sessions = groups.at(groupsIndex)->sessions;

            if(sessionsIndex > -1)
                ui->sessions->itemAt(sessions.at(sessionsIndex))->widget()->setStyleSheet("");

            sessionsIndex++;
            if(sessionsIndex > sessions.size() - 1)
                sessionsIndex = 0;

            ui->sessions->itemAt(sessions.at(sessionsIndex))->widget()->setStyleSheet("background-color:yellow;");
        }
    }
}

void MainWindow::downPress()
{
    if(isOn)
    {
        if(groupsIndex > -1)
        {
            QList<int> sessions = groups.at(groupsIndex)->sessions;

            if(sessionsIndex > -1)
                ui->sessions->itemAt(sessions.at(sessionsIndex))->widget()->setStyleSheet("");

            sessionsIndex--;
            if(sessionsIndex < 0)
                sessionsIndex = sessions.size() - 1;

            ui->sessions->itemAt(sessions.at(sessionsIndex))->widget()->setStyleSheet("background-color:yellow;");
        }
    }
}

void MainWindow::selectGroup()
{
    if(isOn)
    {
        if(groupsIndex > -1)
        {
            ui->groups->itemAt(groupsIndex)->widget()->setStyleSheet("");

            if(sessionsIndex > -1)
            {
                //ui->console->append("Group changed");
                ui->sessions->itemAt(groups.at(groupsIndex)->sessions.at(sessionsIndex))->widget()->setStyleSheet("");
                sessionsIndex = -1;
            }
        }

        groupsIndex = (groupsIndex + 1) % 3;
        ui->groups->itemAt(groupsIndex)->widget()->setStyleSheet("background-color:yellow;");
    }
}

void MainWindow::selectSession()
{
    if(groupsIndex > -1 && sessionsIndex > -1)
    {
        int session = groups.at(groupsIndex)->sessions.at(sessionsIndex);

        QString g = "n/a";
        QString s = "n/a";

        if(groupsIndex == 0)
            g = "20";
        else if(groupsIndex == 1)
            g = "45";
        else if(groupsIndex == 2)
            g = "3h";

        if(session == 0)
            s = "MET";
        else if(session == 1)
            s = "Sub-Delta";
        else if(session == 2)
            s = "Delta";
        else if(session == 3)
            s = "Theta";
        else if(session == 4)
            s = "Alpha";
        else if(session == 5)
            s = "SMR";
        else if(session == 6)
            s = "Beta";
        else if(session == 7)
            s = "100 Hz";

        ui->console->append(s + " for " + g);

        QTimer::singleShot(1000, this, &MainWindow::blink);
        QTimer::singleShot(1200, this, &MainWindow::blink);
        QTimer::singleShot(1400, this, &MainWindow::blink);
        QTimer::singleShot(1600, this, &MainWindow::blink);
        QTimer::singleShot(1800, this, &MainWindow::blink);
        QTimer::singleShot(2000, this, &MainWindow::blink);
    }
}

void MainWindow::blink()
{
    int session = abs(groups.at(groupsIndex)->sessions.at(sessionsIndex) - 7);

    QString highlight = "background-color:yellow;";

    if(ui->graph->itemAt(session)->widget()->styleSheet() == highlight)
        ui->graph->itemAt(session)->widget()->setStyleSheet("");
    else
        ui->graph->itemAt(session)->widget()->setStyleSheet(highlight);
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
        if(groupsIndex > -1)
        {
            ui->groups->itemAt(groupsIndex)->widget()->setStyleSheet("");
            groupsIndex = -1;
            sessionsIndex = -1;
        }
    }
   else
    {
        isOn = !isOn;
        ui->console->append("on");
    }
}
