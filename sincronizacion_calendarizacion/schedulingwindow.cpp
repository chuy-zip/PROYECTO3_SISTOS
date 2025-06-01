#include "schedulingwindow.h"
#include "ui_schedulingwindow.h"

#include "schedulingwindow.h"
#include "ui_schedulingwindow.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QGraphicsTextItem>  // Para QGraphicsTextItem
#include <QGraphicsRectItem>  // Para QGraphicsRectItem
#include <QPen>               // Para QPen
#include <QBrush>

SchedulingWindow::SchedulingWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::SchedulingWindow),
    timerFIFO(new QTimer(this)), cicloActual(0), xActual(0), indiceProcesoFIFO(0), tiempoEjecutadoProcesoActual(0), colorIndex(0)
{
    ui->setupUi(this);
    setWindowTitle("Simulación de Algoritmos de Calendarización");
    //resize(900, 700);

    //colores 20

    coloresProcesos = {
        QColor(255, 0, 0),     QColor(0, 255, 0),     QColor(0, 0, 255),
        QColor(255, 255, 0),   QColor(255, 0, 255),   QColor(0, 255, 255),
        QColor(128, 0, 0),     QColor(0, 128, 0),     QColor(0, 0, 128),
        QColor(128, 128, 0),   QColor(128, 0, 128),   QColor(0, 128, 128),
        QColor(192, 192, 192), QColor(128, 128, 128), QColor(153, 102, 204),
        QColor(255, 128, 0),   QColor(102, 204, 255), QColor(153, 204, 0),
        QColor(255, 153, 153), QColor(204, 153, 255)
    };

    // Configurar la escena para el diagrama de Gantt
    escenaGantt = new QGraphicsScene(this);
    ui->graphicsView->setScene(escenaGantt);

    // Conectar botones a sus slots
    connect(ui->btnCargarArchivo, &QPushButton::clicked, this, &SchedulingWindow::onCargarArchivoClicked);
    connect(ui->btnEjecutarSimulacion, &QPushButton::clicked, this, &SchedulingWindow::onEjecutarSimulacionClicked);
    connect(timerFIFO, &QTimer::timeout, this, &SchedulingWindow::ejecutarCicloFIFO);
}

//parseo del archivo txt separado por comas
void SchedulingWindow::parsearArchivo(const QString &contenido) {
    procesos.clear();
    QStringList lineas = contenido.split('\n', Qt::SkipEmptyParts);

    for (const QString &linea : lineas) {
        QStringList partes = linea.split(',', Qt::SkipEmptyParts);
        if (partes.size() == 4) {
            Proceso p;
            p.PID = partes[0].trimmed();
            p.BT = partes[1].trimmed().toInt();
            p.AT = partes[2].trimmed().toInt();
            p.priority = partes[3].trimmed().toInt();
            procesos.append(p);
        }
    }
}

void SchedulingWindow::onCargarArchivoClicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "Abrir archivo de procesos", "", "Archivos de texto (*.txt)");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error al abrir el archivo";
        return;
    }

    QTextStream in(&file);
    contenidoArchivo = in.readAll();
    ui->txtContenidoArchivo->setPlainText(contenidoArchivo); // Mostrar contenido en el QTextEdit
    file.close();
}

void SchedulingWindow::onEjecutarSimulacionClicked() {
    qDebug() << "Contenido del archivo:\n" << contenidoArchivo;

    // Verificar qué algoritmos están seleccionados
    if (ui->checkBoxFIFO->isChecked()) {
        parsearArchivo(contenidoArchivo);
        dibujarDiagramaFIFO();

    }
    if (ui->checkBoxSJF->isChecked()) qDebug() << "Algoritmo SJF seleccionado";
    if (ui->checkBoxRR->isChecked()) qDebug() << "Algoritmo SRT seleccionado";
    if (ui->checkBoxSRT->isChecked()) qDebug() << "Algoritmo Round Robin seleccionado";
    if (ui->checkBoxPriority->isChecked()) qDebug() << "Algoritmo FIFO seleccionado";

}

void SchedulingWindow::dibujarDiagramaFIFO() {
    limpiarEscena();
    if (procesos.isEmpty()) return;

    std::sort(procesos.begin(), procesos.end(), [](const Proceso &a, const Proceso &b) {
        return a.AT < b.AT;
    });

    coloresAsignados.clear();
    colorIndex = 0;
    cicloActual = 0;
    xActual = 0;
    indiceProcesoFIFO = 0;
    tiempoEjecutadoProcesoActual = 0;

    escenaGantt->addLine(0, 30, 1000, 30, QPen(Qt::black));

    timerFIFO->start(300);
}

void SchedulingWindow::ejecutarCicloFIFO() {
    if (indiceProcesoFIFO >= procesos.size()) {
        timerFIFO->stop();
        // Dibujar línea de tiempo
        for (int i = 0; i < cicloActual; ++i) {
            QGraphicsTextItem *cicloText = escenaGantt->addText(QString::number(i));
            cicloText->setPos(i * 30, 40);
        }
        return;
    }

    const Proceso &p = procesos[indiceProcesoFIFO];

    // Si aún no ha llegado el proceso
    if (cicloActual < p.AT) {
        QGraphicsRectItem *idle = escenaGantt->addRect(xActual, 0, 30, 30, QPen(Qt::black), QBrush(Qt::lightGray));
        QGraphicsTextItem *text = escenaGantt->addText("IDLE");
        text->setPos(xActual + 5, 5);
        xActual += 30;
        cicloActual++;
        return;
    }

    // Asignar color si aún no lo tenía
    if (!coloresAsignados.contains(p.PID)) {
        coloresAsignados[p.PID] = coloresProcesos[colorIndex % coloresProcesos.size()];
        colorIndex++;
    }

    QColor color = coloresAsignados[p.PID];

    // Dibujar un ciclo de este proceso
    QGraphicsRectItem *rect = escenaGantt->addRect(xActual, 0, 30, 30, QPen(Qt::black), QBrush(color));
    QGraphicsTextItem *text = escenaGantt->addText(p.PID);
    text->setPos(xActual + 10, 5);

    xActual += 30;
    cicloActual++;
    tiempoEjecutadoProcesoActual++;

    if (tiempoEjecutadoProcesoActual >= p.BT) {
        indiceProcesoFIFO++;
        tiempoEjecutadoProcesoActual = 0;
    }
}


// aca voy a poner los destructores
void SchedulingWindow::limpiarEscena() {
    escenaGantt->clear();
}

SchedulingWindow::~SchedulingWindow()
{
    delete ui;
}
