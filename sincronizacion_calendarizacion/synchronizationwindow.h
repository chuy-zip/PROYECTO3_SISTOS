#ifndef SYNCHRONIZATIONWINDOW_H
#define SYNCHRONIZATIONWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QVector>
#include <QMap>
#include <QTimer>
#include <queue>

namespace Ui {
class SynchronizationWindow;
}

struct Action {
    QString PID;
    QString action;
    QString resource;
    int cycle;
    bool completed;
    int completionCycle;
    int waitingSince;
};

struct CycleState {
    QVector<Action> accessedActions;
    QVector<Action> waitingActions;
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
    void showNextCycle();

private:
    Ui::SynchronizationWindow *ui;
    QGraphicsScene *scene;
    QVector<Action> actions;
    QMap<QString, QColor> processColors;
    QMap<QString, bool> resourceInUse;
    QMap<QString, int> semaphoreCounts;
    QMap<QString, std::queue<Action*>> waitingQueues;
    QTimer *simulationTimer;
    int currentCycle;
    int maxCycles;
    bool useSemaphore;
    bool simulationRunning;
    QVector<CycleState> cycleStates;
    void drawCompleteTimeline();
    void storeCurrentCycleState();
    void parseResourceFile(const QString &content);
    void parseActionFile(const QString &content);
    void prepareSimulation();
    void drawTimeline();
    void logMessage(const QString &message);
    void resetSimulation();
    bool waitingQueuesEmpty() const;
    QColor getProcessColor(const QString &pid);
    void processWaitingActions();
    bool tryAccessResource(Action* action);
    void releaseResource(const QString &resource);
    int displayCycle;
    QTimer *displayTimer;
    void drawSingleCycle(int cycleToDraw);
    void drawAccumulatedCycles(int upToCycle);
};

#endif // SYNCHRONIZATIONWINDOW_H
