#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QList>
#include <QListWidget>

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
    QListWidget* activeQListWidget;
    QList<Group*> groups;
    Session* currentSession;
    bool isOn;
    int groupsIndex;
    int sessionsIndex;

public slots:
    void startTimer();
    void selectGroup();
    void stopTimer();
    void powerChange();
    void upPress();
    void downPress();
    void selectSession();
    void blink();
};
#endif // MAINWINDOW_H
