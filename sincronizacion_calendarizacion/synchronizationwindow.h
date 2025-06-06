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

struct Action {
    QString PID;
    QString action; // "READ" or "WRITE"
    QString resource;
    int cycle;
};

struct Resource {
    QString name;
    int count;
    int available;
};

class SynchronizationWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SynchronizationWindow(QWidget *parent = nullptr);
    ~SynchronizationWindow();

private slots:
    void onLoadResourcesClicked();
    void onLoadActionsClicked();
    void onRunSimulationClicked();
    void runSimulationStep();
    void onSyncTypeChanged(int index);

private:
    Ui::SynchronizationWindow *ui;
    QGraphicsScene *scene;
    QVector<Resource> resources;
    QVector<Action> actions;
    QMap<QString, QColor> processColors;
    QTimer *simulationTimer;
    int currentCycle;
    int maxCycles;
    bool useSemaphore;

    void parseResourceFile(const QString &content);
    void parseActionFile(const QString &content);
    void prepareSimulation();
    void drawTimeline();
    void logMessage(const QString &message);
    void resetSimulation();
    QColor getProcessColor(const QString &pid);
};

#endif // SYNCHRONIZATIONWINDOW_H
