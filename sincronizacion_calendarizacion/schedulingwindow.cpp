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
    : QMainWindow(parent), ui(new Ui::SchedulingWindow)
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
}


void SchedulingWindow::ejecutarProximaSimulacion() {
    if (simulacionActual < simulaciones.size()) {
        simulaciones[simulacionActual]();
        simulacionActual++;
    } else {
        // Todas las simulaciones completadas
        disconnect(this, &SchedulingWindow::simulacionTerminada, this, &SchedulingWindow::ejecutarProximaSimulacion);
    }
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

    procesosMap.clear();
    for (const Proceso &p : procesos) {
        procesosMap[p.PID] = p;
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

    ui->metricsTextEdit->clear();
    parsearArchivo(contenidoArchivo);

    simulaciones.clear();
    simulacionActual = 0;

    if (ui->checkBoxFIFO->isChecked()) {
        auto resultado = ejecutarFIFO(procesos);
        simulaciones.append([=]() {
            animarSimulacion(resultado, "FIFO");
        });
    }

    if (ui->checkBoxSJF->isChecked()) {
        auto resultado = ejecutarSJF(procesos);
        simulaciones.append([=]() {
            animarSimulacion(resultado, "SJF");
        });
    }

    if (ui->checkBoxRR->isChecked()) qDebug() << "Algoritmo SRT seleccionado";
    if (ui->checkBoxSRT->isChecked()) qDebug() << "Algoritmo Round Robin seleccionado";
    if (ui->checkBoxPriority->isChecked()) qDebug() << "Algoritmo FIFO seleccionado";

    connect(this, &SchedulingWindow::simulacionTerminada, this, &SchedulingWindow::ejecutarProximaSimulacion);
    if (!simulaciones.isEmpty()) {
        ejecutarProximaSimulacion();
    }

}

QVector<ResultadoSimulacion> SchedulingWindow::ejecutarFIFO(const QVector<Proceso>& procesosOriginales) {
    QVector<Proceso> procesos = procesosOriginales;
    QVector<ResultadoSimulacion> resultado;

    std::sort(procesos.begin(), procesos.end(), [](const Proceso &a, const Proceso &b) {
        return a.AT < b.AT;
    });

    int tiempoActual = 0;
    for (const Proceso &p : procesos) {
        if (tiempoActual < p.AT)
            tiempoActual = p.AT;

        resultado.append({p.PID, tiempoActual, p.BT});
        tiempoActual += p.BT;
    }

    return resultado;
}

QVector<ResultadoSimulacion> SchedulingWindow::ejecutarSJF(const QVector<Proceso>& procesosOriginales) {
    QVector<Proceso> procesos = procesosOriginales;
    QVector<ResultadoSimulacion> resultado;

    int tiempoActual = 0;
    QVector<Proceso> procesosPendientes = procesos;
    QVector<Proceso> disponibles;

    while (!procesosPendientes.isEmpty()) {
        // Obtener todos los procesos que han llegado hasta el tiempo actual
        disponibles.clear();
        for (int i = 0; i < procesosPendientes.size(); ++i) {
            if (procesosPendientes[i].AT <= tiempoActual) {
                disponibles.append(procesosPendientes[i]);
            }
        }

        if (!disponibles.isEmpty()) {
            // Elegir el proceso con menor tiempo de burst
            std::sort(disponibles.begin(), disponibles.end(), [](const Proceso &a, const Proceso &b) {
                return a.BT < b.BT;
            });

            Proceso elegido = disponibles.first();

            // Agregar al resultado
            resultado.append({elegido.PID, tiempoActual, elegido.BT});
            tiempoActual += elegido.BT;

            // Eliminar el proceso elegido de los pendientes
            for (int i = 0; i < procesosPendientes.size(); ++i) {
                if (procesosPendientes[i].PID == elegido.PID) {
                    procesosPendientes.removeAt(i);
                    break;
                }
            }
        } else {
            // No hay procesos listos aún, avanzar el tiempo
            tiempoActual++;
        }
    }

    return resultado;
}

//aqui voy a poner las funciones de simulacion
void SchedulingWindow::animarSimulacion(const QVector<ResultadoSimulacion>& resultado, const QString& nombreAlgoritmo) {
    limpiarEscena();
    ui->metricsTextEdit->append("Simulación: " + nombreAlgoritmo);

    // Inicializar variables de animación
    cicloAnimacion = 0;
    xAnimacion = 0;
    colorMapAnimacion.clear();
    colorIndexAnimacion = 0;
    tiemposEsperaAnimacion.clear();
    tiemposRespuestaAnimacion.clear();
    indexAnimacion = 0;
    bloqueActual = 0;  // Nuevo contador para bloques dentro de un proceso
    resultadoActual = resultado;
    procesoActual = nullptr;  // Para rastrear el proceso actual

    // Configuración inicial de la escena
    QPen pen(Qt::black);
    //escenaGantt->addLine(0, 30, 1000, 30, pen);
    ui->graphicsView->setScene(escenaGantt);

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]() {
        // Si no hay proceso actual, obtener el siguiente
        if (procesoActual == nullptr) {

            // esto es al final, cuando ya se pintaron todos
            if (indexAnimacion >= resultadoActual.size()) {
                // Dibujar números de ciclo
                for (int i = 0; i < cicloAnimacion; ++i) {
                    QGraphicsTextItem *cicloText = escenaGantt->addText(QString::number(i));
                    cicloText->setPos(i * 30, 40);
                }

                // Calcular métricas
                double totalEspera = std::accumulate(tiemposEsperaAnimacion.begin(), tiemposEsperaAnimacion.end(), 0.0);
                double totalRespuesta = std::accumulate(tiemposRespuestaAnimacion.begin(), tiemposRespuestaAnimacion.end(), 0.0);
                double promedioEspera = totalEspera / tiemposEsperaAnimacion.size();
                double promedioRespuesta = totalRespuesta / tiemposRespuestaAnimacion.size();

                ui->metricsTextEdit->append("T. Espera Promedio: " + QString::number(promedioEspera));
                ui->metricsTextEdit->append("T. Respuesta Promedio: " + QString::number(promedioRespuesta));
                ui->metricsTextEdit->append("-------------\n");

                timer->stop();
                timer->deleteLater();
                emit simulacionTerminada();
                return;
            }

            procesoActual = &resultadoActual[indexAnimacion];

            // Asignar color si es nuevo proceso
            if (!colorMapAnimacion.contains(procesoActual->PID)) {
                colorMapAnimacion[procesoActual->PID] = coloresProcesos[colorIndexAnimacion++ % coloresProcesos.size()];
            }

            // Dibujar tiempo en el que no tiene proceso
            while (cicloAnimacion < procesoActual->inicio) {
                QGraphicsRectItem *idle = escenaGantt->addRect(xAnimacion, 0, 30, 30, pen, QBrush(Qt::lightGray));
                QGraphicsTextItem *text = escenaGantt->addText("IDLE");
                text->setPos(xAnimacion + 5, 5);
                xAnimacion += 30;
                cicloAnimacion++;
            }

            bloqueActual = 0;  // Resetear contador de bloques
        }

        // Dibujar solo un bloque del proceso actual
        if (bloqueActual < procesoActual->duracion) {
            QGraphicsRectItem *rect = escenaGantt->addRect(xAnimacion, 0, 30, 30, pen, QBrush(colorMapAnimacion[procesoActual->PID]));
            QGraphicsTextItem *text = escenaGantt->addText(procesoActual->PID);
            text->setPos(xAnimacion + 10, 5);
            xAnimacion += 30;
            cicloAnimacion++;
            bloqueActual++;
        } else {
            // Calcular métricas cuando terminamos con este proceso
            int espera = procesoActual->inicio - procesosMap[procesoActual->PID].AT;
            int respuesta = procesoActual->inicio + procesoActual->duracion - procesosMap[procesoActual->PID].AT;
            tiemposEsperaAnimacion.append(espera);
            tiemposRespuestaAnimacion.append(respuesta);

            // Pasar al siguiente proceso
            indexAnimacion++;
            procesoActual = nullptr;
        }
    });

    connect(timer, &QTimer::destroyed, this, [=]() {
        if (simulacionActual < simulaciones.size()) {
            QTimer::singleShot(1000, this, &SchedulingWindow::ejecutarProximaSimulacion);
        }
    });

    timer->start(500);  // Velocidad de animación (ms)
}


// aca voy a poner los destructores
void SchedulingWindow::limpiarEscena() {
    escenaGantt->clear();
}

SchedulingWindow::~SchedulingWindow()
{
    delete ui;
}
