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

    // Configuración inicial
    useSemaphore = false;
    currentCycle = 0;
    maxCycles = 0;
}

SynchronizationWindow::~SynchronizationWindow()
{
    delete ui;
}

void SynchronizationWindow::onSyncTypeChanged(int index)
{
    useSemaphore = (index == 1); // 0: Mutex, 1: Semáforo
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
    resources.clear();
    QStringList lines = content.split('\n', Qt::SkipEmptyParts);

    for (const QString &line : lines) {
        QStringList parts = line.split(',', Qt::SkipEmptyParts);
        if (parts.size() == 2) {
            Resource r;
            r.name = parts[0].trimmed();
            r.count = parts[1].trimmed().toInt();
            r.available = r.count;
            resources.append(r);
        }
    }

    logMessage(QString("Cargados %1 recursos").arg(resources.size()));
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
            actions.append(a);

            // Asignar color si es nuevo proceso
            if (!processColors.contains(a.PID)) {
                // Generar color basado en hash del PID para consistencia
                uint hash = qHash(a.PID);
                processColors[a.PID] = QColor::fromHsv(hash % 360, 255, 200);
            }

            if (a.cycle > maxCycles) {
                maxCycles = a.cycle;
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

    if (useSemaphore && resources.isEmpty()) {
        logMessage("Error: Modo semáforo requiere recursos");
        return;
    }

    resetSimulation();
    prepareSimulation();

    scene->clear();
    ui->txtLog->clear();
    logMessage(QString("Iniciando simulación en modo %1").arg(useSemaphore ? "Semáforo" : "Mutex"));

    simulationTimer->start(1000); // 1 segundo por paso
}

void SynchronizationWindow::prepareSimulation()
{
    // Resetear disponibilidad de recursos
    for (Resource &r : resources) {
        r.available = r.count;
    }

    // Ordenar acciones por ciclo
    std::sort(actions.begin(), actions.end(), [](const Action &a, const Action &b) {
        return a.cycle < b.cycle;
    });

    currentCycle = 0;
}

void SynchronizationWindow::runSimulationStep()
{
    if (currentCycle > maxCycles) {
        simulationTimer->stop();
        logMessage("Simulación completada");
        return;
    }

    // Procesar todas las acciones del ciclo actual
    QVector<Action> currentActions;
    for (const Action &action : actions) {
        if (action.cycle == currentCycle) {
            currentActions.append(action);
        }
    }

    if (currentActions.isEmpty()) {
        logMessage(QString("Ciclo %1: No hay acciones").arg(currentCycle));
    } else {
        // Procesar cada acción del ciclo actual
        for (const Action &action : currentActions) {
            bool success = false;

            if (useSemaphore) {
                // Lógica para semáforos
                for (Resource &r : resources) {
                    if (r.name == action.resource && r.available > 0) {
                        r.available--;
                        success = true;
                        break;
                    }
                }
            } else {
                // Lógica para mutex (solo un proceso puede acceder)
                for (Resource &r : resources) {
                    if (r.name == action.resource) {
                        if (r.available == r.count) { // Recurso disponible
                            r.available--;
                            success = true;
                        }
                        break;
                    }
                }
            }

            logMessage(QString("Ciclo %1: Proceso %2 %3 %4 (%5)")
                           .arg(currentCycle)
                           .arg(action.PID)
                           .arg(action.action)
                           .arg(action.resource)
                           .arg(success ? "Éxito" : "Espera"));
        }
    }

    drawTimeline();
    currentCycle++;
}

void SynchronizationWindow::drawTimeline()
{
    scene->clear();

    const int blockWidth = 80;
    const int blockHeight = 50;
    const int verticalSpacing = 10;
    const int horizontalSpacing = 20;
    const int startX = 20;
    const int startY = 50;

    // Dibujar números de ciclo
    for (int cycle = 0; cycle <= currentCycle; cycle++) {
        QGraphicsTextItem *cycleText = scene->addText(QString::number(cycle));
        cycleText->setPos(startX + cycle * (blockWidth + horizontalSpacing) + blockWidth/2 - 10, startY - 30);
    }

    // Dibujar bloques para cada ciclo
    for (int cycle = 0; cycle <= currentCycle; cycle++) {
        QVector<Action> cycleActions;
        for (const Action &action : actions) {
            if (action.cycle == cycle) {
                cycleActions.append(action);
            }
        }

        // Ordenar acciones verticalmente por PID
        std::sort(cycleActions.begin(), cycleActions.end(), [](const Action &a, const Action &b) {
            return a.PID < b.PID;
        });

        // Dibujar cada acción del ciclo
        for (int i = 0; i < cycleActions.size(); i++) {
            const Action &action = cycleActions[i];

            // Determinar si tuvo éxito (solo para el ciclo actual)
            bool success = false;
            if (cycle == currentCycle - 1) { // Solo para el ciclo que acaba de procesarse
                if (useSemaphore) {
                    for (const Resource &r : resources) {
                        if (r.name == action.resource) {
                            success = (r.available < r.count);
                            break;
                        }
                    }
                } else {
                    // Para mutex, solo el primero en el ciclo tuvo éxito
                    success = (i == 0);
                }
            } else if (cycle < currentCycle - 1) {
                // Para ciclos anteriores, asumimos éxito si fue procesado
                success = true;
            }

            QColor color = success ? Qt::green : QColor(255, 165, 0); // Verde o naranja

            int x = startX + cycle * (blockWidth + horizontalSpacing);
            int y = startY + i * (blockHeight + verticalSpacing);

            // Dibujar rectángulo
            QGraphicsRectItem *rect = scene->addRect(x, y, blockWidth, blockHeight, QPen(Qt::black), QBrush(color));

            // Dibujar texto
            QGraphicsTextItem *pidText = scene->addText(QString("%1 - %2").arg(action.PID).arg(action.resource));
            pidText->setPos(x + 5, y + 5);

            QGraphicsTextItem *actionText = scene->addText(action.action);
            actionText->setPos(x + 5, y + 25);
        }
    }

    // Ajustar la vista
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

    // Resetear recursos
    for (Resource &r : resources) {
        r.available = r.count;
    }
}

QColor SynchronizationWindow::getProcessColor(const QString &pid)
{
    if (!processColors.contains(pid)) {
        // Generar color basado en hash del PID para consistencia
        uint hash = qHash(pid);
        processColors[pid] = QColor::fromHsv(hash % 360, 255, 200);
    }
    return processColors[pid];
}
