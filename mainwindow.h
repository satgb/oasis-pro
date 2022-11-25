#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QList>
#include <QListWidget>

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
    QListWidget* activeQListWidget;
    QList<Group*> groups;
    int currentTimerCount;
    QVector<Session*> dbSessions;

    QStringList allSessions;

    void updateRecordView(QStringList);

    DBManager* db;
    Profile* profile;
    Session* currentSession;

    bool deviceOn;
    bool sessionOn;
    int groupIndex;
    int typeIndex;

    void blink();

public slots:
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
