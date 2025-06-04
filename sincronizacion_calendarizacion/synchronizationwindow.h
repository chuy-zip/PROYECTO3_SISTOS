#ifndef SYNCHRONIZATIONWINDOW_H
#define SYNCHRONIZATIONWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QVector>
#include <QMap>
#include <QTimer>

namespace Ui {
class SynchronizationWindow;
}

// clase para los procesos
struct Process {
    QString PID;
    int BT;  // burst Time
    int AT;  // arrival Time
    int priority;
};

// clase para los recursos
struct Resource {
    QString name;
    int count;
    int available;
};

// clase para las acciones de cada proceso y con que recurso en que ciclo
struct Action {
    QString PID;
    QString action; // "READ" or "WRITE"
    QString resource;
    int cycle;
};

// autoexplicatorio
struct SimulationStep {
    QString PID;
    QString resource;
    QString action;
    int cycle;
    bool success;
};

class SynchronizationWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SynchronizationWindow(QWidget *parent = nullptr);
    ~SynchronizationWindow();

private:
    Ui::SynchronizationWindow *ui;
};

#endif // SYNCHRONIZATIONWINDOW_H
