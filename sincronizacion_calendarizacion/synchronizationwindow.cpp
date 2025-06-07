#include "synchronizationwindow.h"
#include "ui_synchronizationwindow.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QPen>
#include <QBrush>
#include <algorithm>

SynchronizationWindow::SynchronizationWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SynchronizationWindow)
{
    ui->setupUi(this);
    setWindowTitle("Simulador de Mecanismos de Sincronización");

    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);

    connect(ui->btnLoadResources, &QPushButton::clicked, this, &SynchronizationWindow::onLoadResourcesClicked);
    connect(ui->btnLoadActions, &QPushButton::clicked, this, &SynchronizationWindow::onLoadActionsClicked);
    connect(ui->btnRunSimulation, &QPushButton::clicked, this, &SynchronizationWindow::onRunSimulationClicked);
    connect(ui->cbSyncType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SynchronizationWindow::onSyncTypeChanged);

    simulationTimer = new QTimer(this);
    connect(simulationTimer, &QTimer::timeout, this, &SynchronizationWindow::runSimulationStep);

    useSemaphore = false;
    currentCycle = 0;
    maxCycles = 0;
    simulationRunning = false;

    displayTimer = new QTimer(this);
    connect(displayTimer, &QTimer::timeout, this, &SynchronizationWindow::showNextCycle);

    displayCycle = -1;

    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setRenderHint(QPainter::TextAntialiasing);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing, true);
}

SynchronizationWindow::~SynchronizationWindow()
{
    delete ui;
}

void SynchronizationWindow::onSyncTypeChanged(int index)
{
    useSemaphore = (index == 1);
    ui->btnLoadResources->setEnabled(useSemaphore);
    resetSimulation();
    logMessage(QString("Modo cambiado a: %1").arg(useSemaphore ? "Semáforo" : "Mutex"));
}

void SynchronizationWindow::onLoadResourcesClicked()
{
    if (!useSemaphore) return;

    QString filePath = QFileDialog::getOpenFileName(this, "Abrir archivo de recursos", "", "Archivos de texto (*.txt)");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        logMessage("Error al abrir el archivo de recursos");
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    ui->txtResources->setPlainText(content);
    parseResourceFile(content);
    file.close();
}

void SynchronizationWindow::onLoadActionsClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Abrir archivo de acciones", "", "Archivos de texto (*.txt)");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        logMessage("Error al abrir el archivo de acciones");
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    ui->txtActions->setPlainText(content);
    parseActionFile(content);
    file.close();
}

void SynchronizationWindow::parseResourceFile(const QString &content)
{
    semaphoreCounts.clear();
    QStringList lines = content.split('\n', Qt::SkipEmptyParts);

    for (const QString &line : lines) {
        QStringList parts = line.split(',', Qt::SkipEmptyParts);
        if (parts.size() == 2) {
            QString resourceName = parts[0].trimmed();
            int count = parts[1].trimmed().toInt();
            semaphoreCounts[resourceName] = count;
        }
    }

    logMessage(QString("Cargados %1 recursos").arg(semaphoreCounts.size()));
}

void SynchronizationWindow::parseActionFile(const QString &content)
{
    actions.clear();
    processColors.clear();
    QStringList lines = content.split('\n', Qt::SkipEmptyParts);

    for (const QString &line : lines) {
        QStringList parts = line.split(',', Qt::SkipEmptyParts);
        if (parts.size() == 4) {
            Action a;
            a.PID = parts[0].trimmed();
            a.action = parts[1].trimmed().toUpper();
            a.resource = parts[2].trimmed();
            a.cycle = parts[3].trimmed().toInt();
            a.completed = false;
            a.completionCycle = -1;
            a.waitingSince = -1;
            actions.append(a);

            if (!processColors.contains(a.PID)) {
                uint hash = qHash(a.PID);
                processColors[a.PID] = QColor::fromHsv(hash % 360, 255, 200);
            }

            if (a.cycle > maxCycles) {
                maxCycles = a.cycle;
            }

            if (!resourceInUse.contains(a.resource)) {
                resourceInUse[a.resource] = false;
            }
            if (!semaphoreCounts.contains(a.resource) && useSemaphore) {
                semaphoreCounts[a.resource] = 1;
            }
        }
    }

    logMessage(QString("Cargadas %1 acciones").arg(actions.size()));
}

void SynchronizationWindow::onRunSimulationClicked()
{
    if (actions.isEmpty()) {
        logMessage("Error: No hay acciones para ejecutar la simulación");
        return;
    }

    if (useSemaphore && semaphoreCounts.isEmpty()) {
        logMessage("Error: Modo semáforo requiere recursos");
        return;
    }

    resetSimulation();
    prepareSimulation();

    scene->clear();
    ui->txtLog->clear();
    logMessage(QString("Iniciando simulación en modo %1").arg(useSemaphore ? "Semáforo" : "Mutex"));

    simulationRunning = true;
    simulationTimer->start(1000);
}

void SynchronizationWindow::prepareSimulation()
{
    for (auto it = resourceInUse.begin(); it != resourceInUse.end(); ++it) {
        it.value() = false;
        waitingQueues[it.key()] = std::queue<Action*>();
    }

    if (useSemaphore) {
        for (auto it = semaphoreCounts.begin(); it != semaphoreCounts.end(); ++it) {
            resourceInUse[it.key()] = false;
        }
    }

    std::sort(actions.begin(), actions.end(), [](const Action &a, const Action &b) {
        if (a.cycle == b.cycle) return a.PID < b.PID;
        return a.cycle < b.cycle;
    });

    currentCycle = 0;
}

bool SynchronizationWindow::tryAccessResource(Action* action)
{
    if (useSemaphore) {
        if (semaphoreCounts.contains(action->resource)) {
            if (semaphoreCounts[action->resource] > 0) {
                semaphoreCounts[action->resource]--;
                return true;
            }
        }
    } else {
        if (!resourceInUse[action->resource]) {
            resourceInUse[action->resource] = true;
            return true;
        }
    }
    return false;
}

void SynchronizationWindow::releaseResource(const QString &resource)
{
    if (useSemaphore) {
        if (semaphoreCounts.contains(resource)) {
            semaphoreCounts[resource]++;
        }
    } else {
        resourceInUse[resource] = false;
    }
}

bool SynchronizationWindow::waitingQueuesEmpty() const
{
    for (const auto &queue : waitingQueues) {
        if (!queue.empty()) {
            return false;
        }
    }
    return true;
}

void SynchronizationWindow::processWaitingActions()
{
    for (auto it = waitingQueues.begin(); it != waitingQueues.end(); ++it) {
        std::queue<Action*>& queue = it.value();
        if (queue.empty()) continue;

        Action* action = queue.front();
        if (tryAccessResource(action)) {
            action->completed = true;
            action->completionCycle = currentCycle;
            queue.pop();

            logMessage(QString("Ciclo %1: Proceso %2 %3 %4 (ACCESSED desde cola)")
                           .arg(currentCycle)
                           .arg(action->PID)
                           .arg(action->action)
                           .arg(action->resource));
        }
    }
}

void SynchronizationWindow::storeCurrentCycleState()
{
    CycleState state;

    // Guardar acciones completadas en este ciclo
    for (Action &action : actions) {
        if (action.completionCycle == currentCycle) {
            state.accessedActions.append(action);
        }
    }

    // Guardar acciones en espera en este ciclo
    for (Action &action : actions) {
        if ((action.waitingSince == currentCycle) ||
            (action.waitingSince != -1 && !action.completed && action.waitingSince <= currentCycle)) {
            state.waitingActions.append(action);
        }
    }

    cycleStates.append(state);
}

void SynchronizationWindow::runSimulationStep()
{
    if (!simulationRunning) return;

    // Liberar recursos de acciones completadas en el ciclo anterior
    for (Action &action : actions) {
        if (action.completed && action.completionCycle == currentCycle - 1) {
            releaseResource(action.resource);
        }
    }

    processWaitingActions();

    // Procesar acciones del ciclo actual
    QVector<Action*> currentActions;
    for (Action &action : actions) {
        if (action.cycle == currentCycle && !action.completed) {
            currentActions.append(&action);
        }
    }

    for (Action* action : currentActions) {
        if (tryAccessResource(action)) {
            action->completed = true;
            action->completionCycle = currentCycle;
            logMessage(QString("Ciclo %1: Proceso %2 %3 %4 (ACCESSED)")
                           .arg(currentCycle)
                           .arg(action->PID)
                           .arg(action->action)
                           .arg(action->resource));
        } else {
            waitingQueues[action->resource].push(action);
            action->waitingSince = currentCycle;
            logMessage(QString("Ciclo %1: Proceso %2 %3 %4 (WAITING)")
                           .arg(currentCycle)
                           .arg(action->PID)
                           .arg(action->action)
                           .arg(action->resource));
        }
    }

    // Almacenar el estado actual antes de avanzar
    storeCurrentCycleState();

    // Verificar si la simulación ha terminado
    bool allCompleted = true;
    bool allProcessed = true;

    for (const Action &action : actions) {
        if (!action.completed) {
            allCompleted = false;
            if (action.cycle > currentCycle) {
                allProcessed = false;
            }
        }
    }

    if (allCompleted || (allProcessed && waitingQueuesEmpty())) {
        if (!allCompleted) {
            logMessage("Simulación terminada (procesos pendientes no pueden completarse)");
        } else {
            logMessage("Simulación completada");
        }
        simulationTimer->stop();
        simulationRunning = false;

        // Iniciar la visualización paso a paso
        displayCycle = -1;
        displayTimer->start(1000);
        return;
    }

    currentCycle++;
}

void SynchronizationWindow::drawAccumulatedCycles(int upToCycle)
{
    scene->clear();

    const int blockWidth = 100;
    const int blockHeight = 70;
    const int verticalSpacing = 15;
    const int horizontalSpacing = 30;
    const int startX = 20;
    const int startY = 50;

    // Calcular el ancho total necesario
    int totalWidth = startX + (upToCycle + 1) * (blockWidth + horizontalSpacing) + 20;
    int totalHeight = 800;  // Altura fija suficiente

    scene->setSceneRect(0, 0, totalWidth, totalHeight);

    // Dibujar encabezados de ciclo
    for (int cycle = 0; cycle <= upToCycle; cycle++) {
        QGraphicsTextItem *cycleText = scene->addText(QString::number(cycle));
        cycleText->setPos(startX + cycle * (blockWidth + horizontalSpacing) + blockWidth/2 - 10, startY - 30);

        // Resaltar el ciclo más reciente
        if (cycle == upToCycle) {
            QGraphicsRectItem *highlight = scene->addRect(
                startX + cycle * (blockWidth + horizontalSpacing) - 5, startY - 35,
                blockWidth + 10, 25,
                QPen(Qt::red, 2), Qt::NoBrush);
        }
    }

    // Dibujar todos los ciclos hasta upToCycle
    for (int cycle = 0; cycle <= upToCycle; cycle++) {
        int verticalOffset = 0;

        // Dibujar acciones accedidas en este ciclo
        for (Action &action : actions) {
            if (action.completionCycle == cycle) {
                int x = startX + cycle * (blockWidth + horizontalSpacing);
                int y = startY + verticalOffset;

                QGraphicsRectItem *rect = scene->addRect(x, y, blockWidth, blockHeight,
                                                         QPen(Qt::black), QBrush(Qt::green));

                QGraphicsTextItem *pidText = scene->addText(QString("%1 - %2").arg(action.PID).arg(action.resource));
                pidText->setPos(x + 5, y + 5);

                QGraphicsTextItem *actionText = scene->addText(action.action);
                actionText->setPos(x + 5, y + 25);

                QGraphicsTextItem *statusText = scene->addText("ACCESSED");
                statusText->setPos(x + 5, y + 45);

                verticalOffset += blockHeight + verticalSpacing;
            }
        }

        // Dibujar acciones en espera en este ciclo
        for (Action &action : actions) {
            bool isWaiting = (action.waitingSince != -1) &&
                             (!action.completed ||
                              (action.completed && action.completionCycle > cycle));

            if (isWaiting && action.waitingSince <= cycle) {
                int x = startX + cycle * (blockWidth + horizontalSpacing);
                int y = startY + verticalOffset;

                QGraphicsRectItem *rect = scene->addRect(x, y, blockWidth, blockHeight,
                                                         QPen(Qt::black), QBrush(QColor(255, 165, 0)));

                QGraphicsTextItem *pidText = scene->addText(QString("%1 - %2").arg(action.PID).arg(action.resource));
                pidText->setPos(x + 5, y + 5);

                QGraphicsTextItem *actionText = scene->addText(action.action);
                actionText->setPos(x + 5, y + 25);

                QGraphicsTextItem *statusText = scene->addText("WAITING");
                statusText->setPos(x + 5, y + 45);

                verticalOffset += blockHeight + verticalSpacing;
            }
        }
    }

    ui->graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setRenderHint(QPainter::TextAntialiasing);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
}

void SynchronizationWindow::showNextCycle()
{
    displayCycle++;

    if (displayCycle > currentCycle) {
        displayTimer->stop();
        return;
    }

    drawAccumulatedCycles(displayCycle);
}

void SynchronizationWindow::drawSingleCycle(int cycleToDraw)
{
    scene->clear();

    const int blockWidth = 100;
    const int blockHeight = 70;
    const int verticalSpacing = 15;
    const int horizontalSpacing = 30;
    const int startX = 20;
    const int startY = 50;

    // Dibujar encabezados de todos los ciclos (para referencia)
    for (int cycle = 0; cycle < cycleStates.size(); cycle++) {
        QGraphicsTextItem *cycleText = scene->addText(QString::number(cycle));
        cycleText->setPos(startX + cycle * (blockWidth + horizontalSpacing) + blockWidth/2 - 10, startY - 30);

        // Resaltar el ciclo actual
        if (cycle == cycleToDraw) {
            QGraphicsRectItem *highlight = scene->addRect(
                startX + cycle * (blockWidth + horizontalSpacing) - 5, startY - 35,
                blockWidth + 10, 25,
                QPen(Qt::red, 2), Qt::NoBrush);
        }
    }

    const CycleState &state = cycleStates[cycleToDraw];
    int verticalOffset = 0;

    // Dibujar acciones accedidas en este ciclo
    for (const Action &action : state.accessedActions) {
        int x = startX + cycleToDraw * (blockWidth + horizontalSpacing);
        int y = startY + verticalOffset;

        QGraphicsRectItem *rect = scene->addRect(x, y, blockWidth, blockHeight,
                                                 QPen(Qt::black), QBrush(Qt::green));

        QGraphicsTextItem *pidText = scene->addText(QString("%1 - %2").arg(action.PID).arg(action.resource));
        pidText->setPos(x + 5, y + 5);

        QGraphicsTextItem *actionText = scene->addText(action.action);
        actionText->setPos(x + 5, y + 25);

        QGraphicsTextItem *statusText = scene->addText("ACCESSED");
        statusText->setPos(x + 5, y + 45);

        verticalOffset += blockHeight + verticalSpacing;
    }

    // Dibujar acciones en espera en este ciclo
    for (const Action &action : state.waitingActions) {
        int x = startX + cycleToDraw * (blockWidth + horizontalSpacing);
        int y = startY + verticalOffset;

        QGraphicsRectItem *rect = scene->addRect(x, y, blockWidth, blockHeight,
                                                 QPen(Qt::black), QBrush(QColor(255, 165, 0)));

        QGraphicsTextItem *pidText = scene->addText(QString("%1 - %2").arg(action.PID).arg(action.resource));
        pidText->setPos(x + 5, y + 5);

        QGraphicsTextItem *actionText = scene->addText(action.action);
        actionText->setPos(x + 5, y + 25);

        QGraphicsTextItem *statusText = scene->addText("WAITING");
        statusText->setPos(x + 5, y + 45);

        verticalOffset += blockHeight + verticalSpacing;
    }

    ui->graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void SynchronizationWindow::drawCompleteTimeline()
{
    scene->clear();

    const int blockWidth = 100;
    const int blockHeight = 70;
    const int verticalSpacing = 15;
    const int horizontalSpacing = 30;
    const int startX = 20;
    const int startY = 50;

    // Dibujar encabezados de ciclo
    for (int cycle = 0; cycle < cycleStates.size(); cycle++) {
        QGraphicsTextItem *cycleText = scene->addText(QString::number(cycle));
        cycleText->setPos(startX + cycle * (blockWidth + horizontalSpacing) + blockWidth/2 - 10, startY - 30);
    }

    // Dibujar cada ciclo
    for (int cycle = 0; cycle < cycleStates.size(); cycle++) {
        const CycleState &state = cycleStates[cycle];
        int verticalOffset = 0;

        // Dibujar acciones accedidas en este ciclo
        for (const Action &action : state.accessedActions) {
            int x = startX + cycle * (blockWidth + horizontalSpacing);
            int y = startY + verticalOffset;

            QGraphicsRectItem *rect = scene->addRect(x, y, blockWidth, blockHeight,
                                                     QPen(Qt::black), QBrush(Qt::green));

            QGraphicsTextItem *pidText = scene->addText(QString("%1 - %2").arg(action.PID).arg(action.resource));
            pidText->setPos(x + 5, y + 5);

            QGraphicsTextItem *actionText = scene->addText(action.action);
            actionText->setPos(x + 5, y + 25);

            QGraphicsTextItem *statusText = scene->addText("ACCESSED");
            statusText->setPos(x + 5, y + 45);

            verticalOffset += blockHeight + verticalSpacing;
        }

        // Dibujar acciones en espera en este ciclo
        for (const Action &action : state.waitingActions) {
            int x = startX + cycle * (blockWidth + horizontalSpacing);
            int y = startY + verticalOffset;

            QGraphicsRectItem *rect = scene->addRect(x, y, blockWidth, blockHeight,
                                                     QPen(Qt::black), QBrush(QColor(255, 165, 0)));

            QGraphicsTextItem *pidText = scene->addText(QString("%1 - %2").arg(action.PID).arg(action.resource));
            pidText->setPos(x + 5, y + 5);

            QGraphicsTextItem *actionText = scene->addText(action.action);
            actionText->setPos(x + 5, y + 25);

            QGraphicsTextItem *statusText = scene->addText("WAITING");
            statusText->setPos(x + 5, y + 45);

            verticalOffset += blockHeight + verticalSpacing;
        }
    }

    ui->graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void SynchronizationWindow::drawTimeline()
{
    scene->clear();

    const int blockWidth = 100;
    const int blockHeight = 70;
    const int verticalSpacing = 15;
    const int horizontalSpacing = 30;
    const int startX = 20;
    const int startY = 50;

    for (int cycle = 0; cycle <= currentCycle; cycle++) {
        QGraphicsTextItem *cycleText = scene->addText(QString::number(cycle));
        cycleText->setPos(startX + cycle * (blockWidth + horizontalSpacing) + blockWidth/2 - 10, startY - 30);
    }

    for (int cycle = 0; cycle <= currentCycle; cycle++) {
        int verticalOffset = 0;

        for (Action &action : actions) {
            if (action.completed && action.completionCycle == cycle) {
                int x = startX + cycle * (blockWidth + horizontalSpacing);
                int y = startY + verticalOffset;

                QGraphicsRectItem *rect = scene->addRect(x, y, blockWidth, blockHeight,
                                                         QPen(Qt::black), QBrush(Qt::green));

                QGraphicsTextItem *pidText = scene->addText(QString("%1 - %2").arg(action.PID).arg(action.resource));
                pidText->setPos(x + 5, y + 5);

                QGraphicsTextItem *actionText = scene->addText(action.action);
                actionText->setPos(x + 5, y + 25);

                QGraphicsTextItem *statusText = scene->addText("ACCESSED");
                statusText->setPos(x + 5, y + 45);

                verticalOffset += blockHeight + verticalSpacing;
            }
        }

        for (Action &action : actions) {
            if (action.waitingSince == cycle ||
                (action.waitingSince < cycle && !action.completed)) {
                int x = startX + cycle * (blockWidth + horizontalSpacing);
                int y = startY + verticalOffset;

                QGraphicsRectItem *rect = scene->addRect(x, y, blockWidth, blockHeight,
                                                         QPen(Qt::black), QBrush(QColor(255, 165, 0)));

                QGraphicsTextItem *pidText = scene->addText(QString("%1 - %2").arg(action.PID).arg(action.resource));
                pidText->setPos(x + 5, y + 5);

                QGraphicsTextItem *actionText = scene->addText(action.action);
                actionText->setPos(x + 5, y + 25);

                QGraphicsTextItem *statusText = scene->addText("WAITING");
                statusText->setPos(x + 5, y + 45);

                verticalOffset += blockHeight + verticalSpacing;
            }
        }
    }

    ui->graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void SynchronizationWindow::logMessage(const QString &message)
{
    ui->txtLog->append(message);
    qDebug() << message;
}

void SynchronizationWindow::resetSimulation()
{
    simulationTimer->stop();
    scene->clear();
    currentCycle = 0;
    simulationRunning = false;
    cycleStates.clear(); // Limpiar estados anteriores

    for (Action &a : actions) {
        a.completed = false;
        a.completionCycle = -1;
        a.waitingSince = -1;
    }

    for (auto it = waitingQueues.begin(); it != waitingQueues.end(); ++it) {
        while (!it.value().empty()) {
            it.value().pop();
        }
    }

    for (auto it = resourceInUse.begin(); it != resourceInUse.end(); ++it) {
        it.value() = false;
    }

    if (useSemaphore) {
        for (auto it = semaphoreCounts.begin(); it != semaphoreCounts.end(); ++it) {
            it.value() = 1;
        }
    }

    displayTimer->stop();
    displayCycle = -1;
    cycleStates.clear();
}

QColor SynchronizationWindow::getProcessColor(const QString &pid)
{
    if (!processColors.contains(pid)) {
        uint hash = qHash(pid);
        processColors[pid] = QColor::fromHsv(hash % 360, 255, 200);
    }
    return processColors[pid];
}
