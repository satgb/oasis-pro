#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QList>
#include <QtMath>

#include "dbmanager.h"
#include "profile.h"
#include "group.h"
#include "session.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTimer* timer;
    QTimer* sessionTimer;
    QTimer* inactivityTimer;
    QVector<Group*> groups;
    QVector<Session*> dbSessions;

    QStringList allSessions;

    DBManager* db;
    Profile* profile;
    Session* currentSession;

    bool deviceOn;
    int groupIndex;
    int typeIndex;
    int currentTimerCount;

    void blink(int, int = 500, QString = "orange");
    void updateRecordView(QStringList);

private slots:
    void switchGroup();
    void powerChange();
    void pressUp();
    void pressDown();
    void selectSession();
    void changeBatteryLevel(double);
    void initSession(Session*);
    void startSession();
    void drainBattery();
    void recordSession();
    void addSession();
    void replaySession();
    void endSession();
};
#endif // MAINWINDOW_H
