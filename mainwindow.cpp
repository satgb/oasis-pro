#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    currentTimerCount = -1;

    db = new DBManager();

    profile = db->getProfile(1);

    groups.append(new Group({0, 2, 4, 6}));
    groups.append(new Group({1, 2, 3, 4}));

    deviceOn = false;
    sessionOn = false;

    groupIndex = -1;
    typeIndex = -1;

    timer = new QTimer(this);
    timer->setSingleShot(true);

    sessionTimer = new QTimer(this);

    currentSession = nullptr;

    connect(ui->powerButton, &QPushButton::pressed, this, [this]()
    {
        if(deviceOn)
        {
            timer->start(1000);
            if(!sessionOn)
                switchGroup();
            else
            {
                endSession();
            }
        }
        else
            timer->start(1000);
    });

    connect(ui->powerButton, &QPushButton::released, this, [this]()
    {
        timer->stop();
    });

    connect(timer, &QTimer::timeout, this, &MainWindow::powerChange);
    connect(ui->upButton, &QPushButton::pressed, this, &MainWindow::pressUp);
    connect(ui->downButton, &QPushButton::pressed, this, &MainWindow::pressDown);

    connect(ui->selectButton, &QPushButton::pressed, this, &MainWindow::selectSession);

    connect(ui->batteryLevelSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::changeBatteryLevel);

    connect(sessionTimer, &QTimer::timeout, this, [this]()
    {
        drainBattery();

        ui->console->append(QString::number(currentTimerCount));

        if(currentTimerCount > 0)
            currentTimerCount--;

        if(currentTimerCount == 0)
            endSession();
    });

    connect(ui->connectComboBox, QOverload<int>::of(&QComboBox::activated), this, &MainWindow::startSession);
    ui->connectComboBox->blockSignals(true);

    connect(ui->recordButton, &QPushButton::pressed, this, &MainWindow::recordSession);
    ui->recordButton->blockSignals(true);

    connect(ui->replayButton, &QPushButton::pressed, this, &MainWindow::replaySession);

    connect(ui->clearButton, &QPushButton::pressed, this, [this]()
    {
        db->deleteSessions(profile->id);

        allSessions.clear();

        for (int x = 0; x < dbSessions.size(); x++)
            delete dbSessions[x];

        dbSessions.clear();
        updateRecordView(allSessions);
    });

    ui->sessionWidget->setEnabled(false);
    ui->graphWidget->setEnabled(false);
    ui->addWidget->hide();

    ui->batteryLevelSpinBox->setValue(profile->batteryLvl);
    ui->batteryLevelSpinBox->setSingleStep(10);
    //ui->batteryLevelSpinBox->findChild<QLineEdit*>();

    dbSessions = db->getSessions(profile->id);
    for (int x = 0; x < dbSessions.size(); x++)
        allSessions += dbSessions[x]->toString();
}

MainWindow::~MainWindow()
{
    db->addProfile(profile->id, profile->batteryLvl);

    for (int i = 0; i < dbSessions.size(); i++)
        delete dbSessions[i];

    delete db;
    delete profile;
    delete ui;
}

/*
 * Function: recordSession
 * Purpose: Stores the settings of the current session to the database
 * Parameters: none
 * Return: void
*/
void MainWindow::recordSession()
{
    if(currentSession != nullptr && sessionOn)
    {
        //create new Session pointer - do not use currentSession
        Session *s = new Session(currentSession->type, currentSession->duration, currentSession->intensity);

        db->addSession(profile->id, currentSession->type, currentSession->duration, currentSession->intensity);
        dbSessions.append(s);
        allSessions += s->toString();
    }

    updateRecordView(allSessions);
}

/*
 * Function: updateRecordView
 * Purpose: Updates the list of recordings on the ui
 * Parameters: recordItems
 * Return: void
*/
void MainWindow::updateRecordView(QStringList recordItems)
{
    ui->recordList->clear();
    ui->recordList->addItems(recordItems);
    ui->recordList->setCurrentRow(0);
}

/*
 * Function: powerChange
 * Purpose: Prepares device to power on/off
 * Parameters: none
 * Return: void
*/
void MainWindow::powerChange()
{
    if(deviceOn)
    {
        deviceOn = !deviceOn;
        ui->console->append("device is OFF");
        if(groupIndex > -1)
        {
            if(typeIndex > -1)
            {
                ui->sessions->itemAt(groups.at(groupIndex)->sessions.at(typeIndex))->widget()->setStyleSheet("");
                typeIndex = -1;
            }
            ui->groups->itemAt(groupIndex)->widget()->setStyleSheet("");
            groupIndex = -1;
        }

        sessionTimer->stop();
        currentTimerCount = -1;
        sessionOn = false;
        ui->selectButton->blockSignals(false);
        ui->powerButton->blockSignals(false);

        if(currentSession != nullptr)
        {
            ui->graphWidget->findChild<QLabel*>("graphLabel" + QString::number(currentSession->intensity))->setStyleSheet("");
            currentSession = nullptr;
        }
        ui->sessionWidget->setEnabled(false);
        ui->graphWidget->setEnabled(false);
        ui->powerLED->setStyleSheet("");
        ui->recordList->clear();
    }
    else
    {
        if(profile->batteryLvl > 0)
        {
            deviceOn = !deviceOn;
            ui->console->append("device is ON");

            for(int i = 1; i <= qCeil(profile->batteryLvl*8/100); i++)      //determines how many bars of battery are left
                blink(i);

            ui->graphWidget->setEnabled(true);
            ui->powerLED->setStyleSheet("background-color:green;");
            updateRecordView(allSessions);
            ui->replayButton->blockSignals(false);
        }
    }
}

/*
 * Function: endSession
 * Purpose: Prepares device to terminate a session
 * Parameters: none
 * Return: void
*/
void MainWindow::endSession()
{
    ui->console->append("session ended");

    sessionTimer->stop();

    currentSession = nullptr;

    ui->upButton->blockSignals(true);
    ui->downButton->blockSignals(true);

    //soft off
    QTimer::singleShot(0, this, [this](){blink(8);});
    QTimer::singleShot(500, this, [this](){blink(7);});
    QTimer::singleShot(1000, this, [this](){blink(6);});
    QTimer::singleShot(1500, this, [this](){blink(5);});
    QTimer::singleShot(2000, this, [this](){blink(4);});
    QTimer::singleShot(2500, this, [this](){blink(3);});
    QTimer::singleShot(3000, this, [this](){blink(2);});
    QTimer::singleShot(3500, this, [this](){blink(1);});

    QTimer::singleShot(4000, this, [this]()
    {
        ui->upButton->blockSignals(false);
        ui->downButton->blockSignals(false);
        powerChange();
    });
}

/*
 * Function: changeBatteryLevel
 * Purpose: Updates battery level on the ui
 * Parameters: newLevel
 * Return: void
*/
void MainWindow::changeBatteryLevel(double newLevel)
{
    int barsToFlash = 0;
//    if(currentSession != nullptr && ui->connectComboBox->currentIndex() > 0)
//    {

    //displays battery when 1 full bar is depleted
    if(profile->batteryLvl > 87.5 && newLevel <= 87.5)
        barsToFlash = 7;
    else if(profile->batteryLvl > 75.0 && newLevel <= 75.0)
        barsToFlash = 6;
    else if(profile->batteryLvl > 62.5 && newLevel <= 62.5)
        barsToFlash = 5;
    else if(profile->batteryLvl > 50.0 && newLevel <= 50.0)
        barsToFlash = 4;
    else if(profile->batteryLvl > 37.5 && newLevel <= 37.5)
        barsToFlash = 3;
    else if(profile->batteryLvl > 25.0 && newLevel <= 25.0)
        barsToFlash = 2;
    else if(profile->batteryLvl > 12.5 && newLevel <= 12.5)
        barsToFlash = 1;

    for(int i = 1; i <= barsToFlash; i++)
        blink(i);
//    }


    if (newLevel >= 0.0 && newLevel <= 100.0)
    {
        if (newLevel == 0.0 && deviceOn)
        {
            profile->batteryLvl = 0;
            powerChange();
        }
        else
            profile->batteryLvl = newLevel;

        ui->batteryLevelSpinBox->setValue(newLevel);
        int newLevelInt = int(newLevel);
        ui->batteryLevelBar->setValue(newLevelInt);
    }
    else
    {
        ui->batteryLevelSpinBox->setValue(0);
        ui->batteryLevelBar->setValue(0);
        powerChange();
    }
}

/*
 * Function: startSession
 * Purpose: Controls the session timer
 * Parameters: none
 * Return: void
*/
void MainWindow::startSession()
{
    if(currentSession != nullptr)       //session timer starts when session is selected
    {
        if(ui->connectComboBox->currentIndex() > 0)
        {
            ui->console->append("session started");
            sessionTimer->start(1000);
        }
        else
        {
            ui->console->append("waiting for connection");
            sessionTimer->stop();
        }
    }
}

/*
 * Function: initSession
 * Purpose: Initializes the selected recording
 * Parameters: none
 * Return: void
*/
void MainWindow::replaySession()
{
    if(deviceOn)
    {
        if(sessionOn == false)
        {
            int recordIndex = ui->recordList->currentRow();

            ui->console->append(QString::number(recordIndex));

            if(recordIndex > -1)
            {
                Session* s = dbSessions.at(recordIndex);

                //new Session prevents the current session from terminating when the recording list is cleared
                initSession(new Session(s->type, s->duration, s->intensity));
            }
        }
    }
}

/*
 * Function: initSession
 * Purpose: Prepares the device to start a new session
 * Parameters: s
 * Return: void
*/
void MainWindow::initSession(Session* s)        //DOUBLE CHECK THIS FUNCTION
{
    currentSession = s;

    ui->console->append(currentSession->type + " for " + QString::number(currentSession->duration));

    ui->selectButton->blockSignals(true);
    ui->connectComboBox->blockSignals(false);
    ui->recordButton->blockSignals(false);
    ui->replayButton->blockSignals(true);
//    ui->upButton->blockSignals(true);
//    ui->downButton->blockSignals(true);

//    ui->groups->itemAt(groupIndex)->widget()->setStyleSheet("");
//    ui->sessions->itemAt(groups.at(groupIndex)->sessions.at(typeIndex))->widget()->setStyleSheet("");

    ui->sessionWidget->setEnabled(false);
    ui->graphWidget->findChild<QLabel*>("graphLabel" + QString::number(currentSession->intensity))->setStyleSheet("background-color:yellow;");

    sessionOn = true;
    currentTimerCount = currentSession->duration * 6;   //convert duration (min) to sec and divide by 10 to speed up simulation
    startSession();
}

/*
 * Function: drainBattery
 * Purpose: Calculates the battery level after 1 second of usage
 * Parameters: none
 * Return: void
*/
void MainWindow::drainBattery()
{
    //battery lasts for 5 minutes - longest session is 4.5 minutes
    double batteryLevel = profile->batteryLvl - 0.33;
    //double batteryLevel = profile->batteryLvl - (0.0002 + (currentSession->intensity/100000.00) + (ui->connectComboBox->currentIndex() == 1 ? 0.00001 : 0.00002));

    changeBatteryLevel(batteryLevel);
}

/*
 * Function: addSession
 * Purpose: Creates a user defined session
 * Parameters: none
 * Return: void
*/
void MainWindow::addSession()
{
    ui->addWidget->show();
    ui->powerButton->blockSignals(true);

    connect(ui->closeButton, &QPushButton::pressed, this, [this]()
    {
        ui->addWidget->hide();
        ui->powerButton->blockSignals(false);
    });

    connect(ui->runButton, &QPushButton::pressed, this, [this]()
    {
        int typeIndex = ui->typeComboBox->currentIndex();
        int durationIndex = ui->durationComboBox->currentIndex();

        if(typeIndex > 0 && durationIndex > -1)
        {
            QString type = ui->typeComboBox->currentText();

            int duration = ui->durationComboBox->currentText().toInt();
            int intensity = ui->intensitySpinBox->value();

            //ui->console->append(type + " " + QString::number(duration) + " " + QString::number(intensity));

            ui->addWidget->hide();
            ui->powerButton->blockSignals(false);

            initSession(new Session(type, duration, intensity));
        }
    });
}

/*
 * Function: selectSession
 * Purpose: Gathers information to initialize a predefined session
 * Parameters: none
 * Return: void
*/
void MainWindow::selectSession()
{
    if(groupIndex > -1 && groupIndex < 2 && typeIndex > -1)
    {
        QString g = ui->groups->itemAt(groupIndex)->widget()->objectName().remove(0,5);

        QString s = ui->sessions->itemAt(groups.at(groupIndex)->sessions.at(typeIndex))->widget()->objectName().remove(0,5);

        //blink((groups.at(groupIndex)->sessions.at(typeIndex))+1);
        initSession(new Session(s, g.toInt(), 1));
    }
    else if(groupIndex == 2)
    {
        addSession();
    }
}

void MainWindow::blink(int graphLabelNum)
{
    QTimer::singleShot(0, this, [this, graphLabelNum]()
    {
        ui->graphWidget->findChild<QLabel*>("graphLabel" + QString::number(graphLabelNum))->setStyleSheet("background-color:orange;");
        ui->upButton->blockSignals(true);
        ui->downButton->blockSignals(true);
    });

    QTimer::singleShot(500, this, [this, graphLabelNum]()
    {
        ui->graphWidget->findChild<QLabel*>("graphLabel" + QString::number(graphLabelNum))->setStyleSheet("");

        if(currentSession != nullptr)
        {
            ui->graphWidget->findChild<QLabel*>("graphLabel" + QString::number(currentSession->intensity))->setStyleSheet("background-color:yellow;");
        }

        ui->upButton->blockSignals(false);
        ui->downButton->blockSignals(false);
    });
}

/*
 * Function: pressUp
 * Purpose: Handles upButton press event depending on the context
 * Parameters: none
 * Return: void
*/
void MainWindow::pressUp()
{
    if(deviceOn)
    {
        if(sessionOn)
        {
            if(currentSession->intensity < 8)
            {
                ui->graphWidget->findChild<QLabel*>("graphLabel" + QString::number(currentSession->intensity))->setStyleSheet("");
                currentSession->intensity++;
                ui->graphWidget->findChild<QLabel*>("graphLabel" + QString::number(currentSession->intensity))->setStyleSheet("background-color:yellow;");
            }
        }
        else
        {
            if(groupIndex > -1 && groupIndex < 2)
            {
                QList<int> sessions = groups.at(groupIndex)->sessions;

                if(typeIndex > -1)
                    ui->sessions->itemAt(sessions.at(typeIndex))->widget()->setStyleSheet("");

                typeIndex++;
                if(typeIndex > sessions.size() - 1)
                    typeIndex = 0;

                ui->sessions->itemAt(sessions.at(typeIndex))->widget()->setStyleSheet("background-color:yellow;");
            }
        }
    }
}

/*
 * Function: pressDown
 * Purpose: Handles downButton press event depending on the context
 * Parameters: none
 * Return: void
*/
void MainWindow::pressDown()
{
    if(deviceOn)
    {
        if(sessionOn)
        {
            if(currentSession->intensity > 1)
            {
                ui->graphWidget->findChild<QLabel*>("graphLabel" + QString::number(currentSession->intensity))->setStyleSheet("");
                currentSession->intensity--;
                ui->graphWidget->findChild<QLabel*>("graphLabel" + QString::number(currentSession->intensity))->setStyleSheet("background-color:yellow;");
            }
        }
        else
        {
            if(groupIndex > -1 && groupIndex < 2)
            {
                QList<int> sessions = groups.at(groupIndex)->sessions;

                if(typeIndex > -1)
                    ui->sessions->itemAt(sessions.at(typeIndex))->widget()->setStyleSheet("");

                typeIndex--;
                if(typeIndex < 0)
                    typeIndex = sessions.size() - 1;

                ui->sessions->itemAt(sessions.at(typeIndex))->widget()->setStyleSheet("background-color:yellow;");
            }
        }
    }
}

/*
 * Function: switchGroup
 * Purpose: Highlights the selected session group
 * Parameters: none
 * Return: void
*/
void MainWindow::switchGroup()
{
    if(deviceOn)
    {
        ui->sessionWidget->setEnabled(true);

        if(groupIndex > -1)
        {
            ui->groups->itemAt(groupIndex)->widget()->setStyleSheet("");

            if(typeIndex > -1 && groupIndex < 2)
            {
                ui->sessions->itemAt(groups.at(groupIndex)->sessions.at(typeIndex))->widget()->setStyleSheet("");
                typeIndex = -1;
            }
        }

        groupIndex = (groupIndex + 1) % 3;
        ui->groups->itemAt(groupIndex)->widget()->setStyleSheet("background-color:yellow;");
    }
}



