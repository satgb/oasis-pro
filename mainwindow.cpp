#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    db = new DBManager();

    profile = db->getProfile(1);

    groups.append(new Group({0, 2, 4, 6}));
    groups.append(new Group({1, 2, 3, 4}));

    highlight = "background-color:yellow;";

    deviceOn = false;
    sessionOn = false;
    graphOn = false;

    blinkCount = 0;

    softOffNum = 0;

    groupIndex = -1;
    typeIndex = -1;

    timer = new QTimer(this);
    timer->setSingleShot(true);

    sessionTimer = new QTimer(this);

    batteryBlinkTimer = new QTimer(this);
    batteryBlinkCooldown = new QTimer(this);

    countdownTimer = new QTimer(this);

    currentSession = nullptr;

    connect(ui->powerButton, &QPushButton::pressed, this, [this]()
    {
        if(deviceOn)
        {
            timer->start(1000);
            if(!sessionOn)
            {
                switchGroup();
            }
        }
        else
            timer->start(1000);
    });

    connect(ui->powerButton, &QPushButton::released, this, [this]()
    {
        timer->stop();

        if (sessionOn){
            countdownTimer->start(1000);
        }
    });

    connect(timer, &QTimer::timeout, this, &MainWindow::powerChange);
    connect(ui->upButton, &QPushButton::pressed, this, &MainWindow::pressUp);
    connect(ui->downButton, &QPushButton::pressed, this, &MainWindow::pressDown);

    connect(ui->selectButton, &QPushButton::pressed, this, &MainWindow::selectSession);

    connect(ui->batteryLevelSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::changeBatteryLevel);

    connect(sessionTimer, &QTimer::timeout, this, [this]()
    {
        drainBattery();
    });

    connect(batteryBlinkTimer, &QTimer::timeout, this, &MainWindow::displayBatteryWarning);
    connect(batteryBlinkCooldown, &QTimer::timeout, this, &MainWindow::startBatteryWarning);

    connect(countdownTimer, &QTimer::timeout, this, &MainWindow::softOff);

    connect(ui->connectComboBox, QOverload<int>::of(&QComboBox::activated), this, &MainWindow::startSession);
    ui->connectComboBox->blockSignals(true);

    connect(ui->recordButton, &QPushButton::pressed, this, &MainWindow::recordSession);
    ui->recordButton->blockSignals(true);

    connect(ui->replayButton, &QPushButton::pressed, this, &MainWindow::replaySession);

    connect(ui->clearButton, &QPushButton::pressed, this, [this]()
    {
        db->deleteSessions(profile->id);

        allSessions.clear();

        for (int x = 0; x < dbSessions.size(); x++) {
            delete dbSessions[x];
        }

        dbSessions.clear();
        updateRecordView(allSessions);
    });

    ui->sessionWidget->setEnabled(false);
    ui->graphWidget->setEnabled(false);
    ui->addWidget->hide();

    ui->batteryLevelSpinBox->setValue(profile->batteryLvl);

    dbSessions = db->getSessions(profile->id);
    for (int x = 0; x < dbSessions.size(); x++)
    {
        allSessions += dbSessions[x]->toString();
    }
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

void MainWindow::recordSession()
{
    if(currentSession != nullptr && sessionOn)
    {
        Session *s = currentSession;

        db->addSession(profile->id, currentSession->type, currentSession->duration, currentSession->intensity);
        dbSessions.append(s);
        allSessions += s->toString();
    }

    updateRecordView(allSessions);
}

void MainWindow::updateRecordView(QStringList recordItems)
{
    ui->recordList->clear();
    ui->recordList->addItems(recordItems);
    ui->recordList->setCurrentRow(0);
}

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

        sessionOn = false;
        sessionTimer->stop();
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

            ui->graphWidget->setEnabled(true);
            ui->powerLED->setStyleSheet("background-color:green;");
            updateRecordView(allSessions);
            ui->replayButton->blockSignals(false);
            startBatteryWarning();
            if (profile->batteryLvl < 12.5){
                ui->console->append("The battery must be replaced before the unit can be used again.");
                powerChange();
            }
        }
    }
}

void MainWindow::endSession()
{
    ui->centralwidget->blockSignals(true);
/*
    for(int i = 1; i <= 8; i++)
    {
        ui->graphWidget->findChild<QLabel*>("graphLabel" + QString::number(i))->setStyleSheet("background-color:yellow");

        QTimer::singleShot(1000, this, [this]()
        {
            ui->graphWidget->findChild<QLabel*>("graphLabel" + QString::number(i))->setStyleSheet("");
        });
    }
*/
    ui->centralwidget->blockSignals(false);

    powerChange();
}

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



void MainWindow::startSession()
{
    if(ui->connectComboBox->currentIndex() > 0)
    {
        ui->console->append("session started");

        sessionTimer->start(1000);
        batteryBlinkCooldown->start(20000);
    }
    else
    {
        ui->console->append("waiting for connection");
        sessionTimer->stop();
    }
}

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

                initSession(new Session(s->type, s->duration, s->intensity));

                //QString s = ui->console->append(allSessions.at(recordIndex));
                //ui->console->append(ui->recordList->currentItem()->text());
                //QString type = s
            }
        }
    }
}

void MainWindow::initSession(Session* s)
{
    currentSession = s;
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
    currentTimerCount = currentSession->duration;
    startSession();
}

void MainWindow::drainBattery()
{
    double batteryLevel = profile->batteryLvl - (0.05 + currentSession->intensity/100.00);

    changeBatteryLevel(batteryLevel);
}

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
        int typeIndex1 = ui->typeComboBox1->currentIndex();
        int typeIndex2 = ui->typeComboBox2->currentIndex();
        int durationIndex = ui->durationComboBox->currentIndex();

        if(typeIndex1 > 0 && durationIndex > -1)
        {
            QString type = ui->typeComboBox1->currentText();

            if(typeIndex2 > 0 && typeIndex1 != typeIndex2)
            {
                type += "+" + ui->typeComboBox2->currentText();
            }

            int duration = ui->durationComboBox->currentText().toInt();
            int intensity = ui->intensitySpinBox->value();

            //ui->console->append(type + " " + QString::number(duration) + " " + QString::number(intensity));

            ui->addWidget->hide();
            ui->powerButton->blockSignals(false);

            initSession(new Session(type, duration, intensity));
        }
    });
}

void MainWindow::selectSession()
{
    if(groupIndex > -1 && groupIndex < 2 && typeIndex > -1)
    {
        //int session = groups.at(groupIndex)->sessions.at(typeIndex);

//        QString g = "-";
        QString g = ui->groups->itemAt(groupIndex)->widget()->objectName().remove(0,5);

        QString s = ui->sessions->itemAt(groups.at(groupIndex)->sessions.at(typeIndex))->widget()->objectName().remove(0,5);
/*
        if(groupIndex == 0)
            g = "20";
        else if(groupIndex == 1)
            g = "45";

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
*/
        ui->console->append(s + " for " + g);

        initSession(new Session(s, g.toInt(), 1));
/*
        QTimer::singleShot(1000, this, &MainWindow::blink);
        QTimer::singleShot(1200, this, &MainWindow::blink);
        QTimer::singleShot(1400, this, &MainWindow::blink);
        QTimer::singleShot(1600, this, &MainWindow::blink);
        QTimer::singleShot(1800, this, &MainWindow::blink);
        QTimer::singleShot(2000, this, &MainWindow::blink);*/
    }
    else if(groupIndex == 2)
    {
        addSession();
    }
}

void MainWindow::blink()
{
    int session = abs(groups.at(groupIndex)->sessions.at(typeIndex) - 7);

    QString highlight = "background-color:yellow;";

    if(ui->graph->itemAt(session)->widget()->styleSheet() == highlight)
        ui->graph->itemAt(session)->widget()->setStyleSheet("");
    else
        ui->graph->itemAt(session)->widget()->setStyleSheet(highlight);
}

void MainWindow::changeBatteryLevel(double newLevel)
{
    if (currentSession != nullptr){
        // if the battery level was just changed to 2 bars (less than 25)
        if (profile->batteryLvl < 25.0 && profile->batteryLvl + (0.05 + currentSession->intensity/100.00) >= 25.0){
            startBatteryWarning();
            ui->console->append("It is recommended to replace the battery before starting another session.");
        }

        if (profile->batteryLvl < 12.5 && profile->batteryLvl + (0.05 + currentSession->intensity/100.00) >= 12.5){
            startBatteryWarning();
            if (sessionOn){
                endSession();
            }
            ui->console->append("The battery must be replaced before the unit can be used again.");
            powerChange();
        }
    }


    if (newLevel >= 0.0 && newLevel <= 100.0)
    {
        if (newLevel == 0.0 && deviceOn)
        {
            powerChange();
            profile->batteryLvl = 0;
        }
        else
            profile->batteryLvl = newLevel;

        ui->batteryLevelSpinBox->setValue(newLevel);
        int newLevelInt = int(newLevel);
        ui->batteryLevelBar->setValue(newLevelInt);
    }
    else
    {
        ui->console->append("negative");
        ui->batteryLevelSpinBox->setValue(0);
        ui->batteryLevelBar->setValue(0);
        powerChange();
    }
}

/*
Function: startBatteryWarning
Purpose: Initiates the battery warning sequence by starting the blink timer
         (which calls displayBatteryWarning).
Parameters: none
returns: void
*/
void MainWindow::startBatteryWarning()
{
    batteryBlinkTimer->start(500);
}

/*
Function: displayBatteryWarning
Purpose: Toggles the light on the graph labels depending on their current state
         to achieve a blinking effect. It will turn on a certain number of
         lights on the graph proportionate to the battery's current level.
Parameters: none
returns: void
*/
void MainWindow::displayBatteryWarning()
{
    if (graphOn){
        // turn off light on all graph labels 1-8
        for (int i = 0; i < 8; ++i){
            ui->graph->itemAt(i)->widget()->setStyleSheet("");
        }

        // re-display intensity
        if (sessionOn){
            ui->graphWidget->findChild<QLabel*>("graphLabel" + QString::number(currentSession->intensity))->setStyleSheet(highlight);
        }


        // reset after 5 (arbitrary #) blinks
        if (blinkCount >= 5){
            batteryBlinkTimer->stop();
            blinkCount = 0;
        }

        graphOn = false;
    } else {
        // convert the battery level value to a 1-8 scale to know how many
        // lights to turn on
        int maxGraphNum = qCeil(profile->batteryLvl / (100.0 / 8.0));

        // turn on all lights up to corresponding battery level
        // itemAt() reads the labels backward so the for loop as to iterate backward as well
        for (int i = 7; i >= 8 - maxGraphNum; --i){
            ui->graph->itemAt(i)->widget()->setStyleSheet(highlight);
        }

        ++blinkCount;
        graphOn = true;
    }
}

void MainWindow::softOff()
{
    // turn off light on all graph labels 1-8
    for (int i = 0; i < 8; ++i){
        ui->graph->itemAt(i)->widget()->setStyleSheet("");
    }

    if (softOffNum == 8){
        endSession();
        softOffNum = 0;
        countdownTimer->stop();
        return;
    }

    // turn on number corresponding to the countdown
    ui->graph->itemAt(softOffNum)->widget()->setStyleSheet(highlight);
    ++softOffNum;
}
