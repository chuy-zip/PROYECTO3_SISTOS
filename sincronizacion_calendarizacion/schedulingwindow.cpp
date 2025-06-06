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
#include <QQueue>
#include <QMessageBox>

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
        // todas las simulaciones han terminao
        disconnect(this, &SchedulingWindow::simulacionTerminada, nullptr, nullptr);
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
    QString formattedContent;
    contenidoArchivo.clear();

    // Leer línea por línea y formatear
    while (!in.atEnd()) {
        QString line = in.readLine();
        contenidoArchivo += line + "\n"; // Guardar el contenido original

        QStringList parts = line.split(",");
        if (parts.size() >= 4) {
            formattedContent += QString("PID: %1, BT: %2, AT: %3, Priority: %4\n")
                                    .arg(parts[0].trimmed())
                                    .arg(parts[1].trimmed())
                                    .arg(parts[2].trimmed())
                                    .arg(parts[3].trimmed());
        } else {
            formattedContent += line + "\n";
        }
    }

    ui->txtContenidoArchivo->setPlainText(formattedContent);
    file.close();
}

void SchedulingWindow::onEjecutarSimulacionClicked() {

    if (contenidoArchivo.isEmpty()) {
        QMessageBox::warning(this,
                             "Advertencia",
                             "No se ha cargado ningún archivo de procesos.\nPor favor, cargue un archivo primero.");
        return;
    }

    // al menos uno seleccionado
    if (!ui->checkBoxFIFO->isChecked() &&
        !ui->checkBoxSJF->isChecked() &&
        !ui->checkBoxSRT->isChecked() &&
        !ui->checkBoxRR->isChecked() &&
        !ui->checkBoxPriority->isChecked()) {
        QMessageBox::warning(this, "Advertencia",
                             "No se ha seleccionado ningún algoritmo.\nSeleccione al menos uno.");
        return;
    }

    limpiarEscena();
    qDebug() << "Contenido del archivo:\n" << contenidoArchivo;

    ui->metricsTextEdit->clear();
    parsearArchivo(contenidoArchivo);

    simulaciones.clear();
    simulacionActual = 0;

    int heightMul = 1;

    if (ui->checkBoxFIFO->isChecked()) {
        auto resultado = ejecutarFIFO(procesos);
        simulaciones.append([=]() {
            animarSimulacion(resultado, "FIFO", heightMul);
        });
    }

    if (ui->checkBoxSJF->isChecked()) {
        heightMul +=1;
        auto resultado = ejecutarSJF(procesos);
        simulaciones.append([=]() {
            animarSimulacion(resultado, "SJF", heightMul);
        });
    }

    if (ui->checkBoxSRT->isChecked()) {
        heightMul +=1;
        auto resultado = ejecutarSRT(procesos);
        simulaciones.append([=]() {
            animarSimulacion(resultado, "SRT", heightMul);
        });
    }

    if (ui->checkBoxRR->isChecked()) {
        heightMul +=1;
        int quantum = ui->quantumSpinBox->value(); // valor del spinbox
        auto resultado = ejecutarRR(procesos, quantum);
        simulaciones.append([=]() {
            animarSimulacion(resultado, "Round Robin (Q=" + QString::number(quantum) + ")", heightMul);
        });
    }

    if (ui->checkBoxPriority->isChecked()) {
        heightMul +=1;
        int intervaloAging = ui->spinBoxAging->value(); //valor del spinbox
        auto resultado = ejecutarPriorityAging(procesos, intervaloAging);
        simulaciones.append([=]() {
            animarSimulacion(resultado, "Priority Aging (T=" + QString::number(intervaloAging) + ")", heightMul);
        });
    }

    disconnect(this, &SchedulingWindow::simulacionTerminada, nullptr, nullptr);
    connect(this, &SchedulingWindow::simulacionTerminada, this, [=]() {
        QTimer::singleShot(1000, this, &SchedulingWindow::ejecutarProximaSimulacion);
    });

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

QVector<ResultadoSimulacion> SchedulingWindow::ejecutarSRT(const QVector<Proceso>& procesosOriginales) {
    QVector<ResultadoSimulacion> resultado;
    QVector<Proceso> procesosPendientes = procesosOriginales;

    int tiempoActual = 0;
    Proceso* procesoActual = nullptr;
    QMap<QString, int> tiempoRestante;
    QMap<QString, Proceso*> procesoMap;  // Mapa para búsqueda rápida

    // Inicializar estructuras
    for (auto& proceso : procesosPendientes) {
        tiempoRestante[proceso.PID] = proceso.BT;
        procesoMap[proceso.PID] = &proceso;
    }

    while (!tiempoRestante.isEmpty()) {
        // Recolectar procesos disponibles
        QVector<Proceso*> disponibles;
        for (auto it = tiempoRestante.begin(); it != tiempoRestante.end(); ++it) {
            Proceso* proc = procesoMap[it.key()];
            if (proc->AT <= tiempoActual && it.value() > 0) {
                disponibles.append(proc);
            }
        }

        if (!disponibles.isEmpty()) {
            // Ordenar por tiempo restante
            std::sort(disponibles.begin(), disponibles.end(), [&](Proceso* a, Proceso* b) {
                return tiempoRestante[a->PID] < tiempoRestante[b->PID];
            });

            Proceso* elegido = disponibles.first();

            // Manejar cambio de proceso
            if (!procesoActual || procesoActual->PID != elegido->PID) {
                resultado.append({elegido->PID, tiempoActual, 1});
            } else {
                resultado.last().duracion++;
            }

            procesoActual = elegido;
            tiempoRestante[elegido->PID]--;

            // Eliminar si terminó
            if (tiempoRestante[elegido->PID] == 0) {

                tiempoRestante.remove(elegido->PID);
            }

        }

        tiempoActual++;
    }

    return resultado;
}

QVector<ResultadoSimulacion> SchedulingWindow::ejecutarRR(const QVector<Proceso>& procesosOriginales, int quantum) {
    QVector<ResultadoSimulacion> resultado;
    QVector<Proceso> procesos = procesosOriginales;
    QMap<QString, int> tiempoRestante;
    QQueue<Proceso*> colaListos;
    int tiempoActual = 0;

    // Inicializar tiempo restante
    for (const auto& p : procesos) {
        tiempoRestante[p.PID] = p.BT;
    }

    // Ordenar procesos por tiempo de llegada
    std::sort(procesos.begin(), procesos.end(), [](const Proceso& a, const Proceso& b) {
        return a.AT < b.AT;
    });

    // Índice para nuevos procesos que llegan
    int nextProceso = 0;

    while (true) {
        // Agregar procesos que han llegado a la cola de listos
        while (nextProceso < procesos.size() && procesos[nextProceso].AT <= tiempoActual) {
            colaListos.enqueue(&procesos[nextProceso]);
            nextProceso++;
        }

        if (colaListos.isEmpty()) {
            if (nextProceso < procesos.size()) {
                // No hay procesos listos, avanzar al próximo tiempo de llegada
                tiempoActual = procesos[nextProceso].AT;
                continue;
            } else {
                // Todos los procesos completados
                break;
            }
        }

        Proceso* actual = colaListos.dequeue();
        int tiempoEjecucion = qMin(quantum, tiempoRestante[actual->PID]);

        // Registrar el segmento de ejecución
        resultado.append({actual->PID, tiempoActual, tiempoEjecucion});

        // Actualizar tiempo restante
        tiempoRestante[actual->PID] -= tiempoEjecucion;
        tiempoActual += tiempoEjecucion;

        // Agregar procesos que llegaron durante esta ejecución
        while (nextProceso < procesos.size() && procesos[nextProceso].AT < tiempoActual) {
            colaListos.enqueue(&procesos[nextProceso]);
            nextProceso++;
        }

        // Si el proceso no ha terminado, volver a colocarlo en la cola
        if (tiempoRestante[actual->PID] > 0) {
            colaListos.enqueue(actual);
        }
    }

    return resultado;
}

QVector<ResultadoSimulacion> SchedulingWindow::ejecutarPriorityAging(const QVector<Proceso>& procesosOriginales, int intervaloAging) {
    QVector<ResultadoSimulacion> resultado;
    QVector<Proceso> procesos = procesosOriginales;
    std::sort(procesos.begin(), procesos.end(), [](const Proceso& a, const Proceso& b) {
        return a.AT < b.AT; // Ordenar por tiempo de llegada inicial
    });

    int tiempoActual = 0;
    QVector<Proceso*> readyQueue;
    QMap<QString, int> tiempoRestante;
    QMap<QString, int> prioridadActual; // Prioridad dinámica con aging
    QMap<QString, int> ultimoAging;    // Cuando se aplicó último aging

    // Inicialización
    for (const auto& p : procesos) {
        tiempoRestante[p.PID] = p.BT;
        prioridadActual[p.PID] = p.priority;
        ultimoAging[p.PID] = p.AT;
    }

    while (true) {
        // llegado a la ready queue
        for (auto& p : procesos) {
            if (p.AT == tiempoActual) {
                readyQueue.push_back(&p);
            }
        }

        // envejecimiento: cada 5 ciclos aumentamos prioridad (disminuimos valor)
        for (auto& p : readyQueue) {
            if (tiempoActual - ultimoAging[p->PID] >= intervaloAging) {
                prioridadActual[p->PID] = std::max(1, prioridadActual[p->PID] - 1);
                ultimoAging[p->PID] = tiempoActual;
            }
        }

        //seleccionar proceso con mayor prioridad, osea el de menor valor. priority 1 es el mas alto

        Proceso* elegido = nullptr;
        if (!readyQueue.isEmpty()) {
            std::sort(readyQueue.begin(), readyQueue.end(), [&](Proceso* a, Proceso* b) {
                return prioridadActual[a->PID] < prioridadActual[b->PID]; // Menor valor = mayor prioridad
            });
            elegido = readyQueue.first();
        }

        // Ejecutar el proceso elegido o IDLE
        if (elegido && tiempoRestante[elegido->PID] > 0) {
            // Continuar o crear nuevo segmento
            if (!resultado.isEmpty() && resultado.last().PID == elegido->PID) {
                resultado.last().duracion++;
            } else {
                resultado.append({elegido->PID, tiempoActual, 1});
            }

            tiempoRestante[elegido->PID]--;

            // Eliminar si terminó
            if (tiempoRestante[elegido->PID] == 0) {
                readyQueue.removeAll(elegido);
            }
        }

        // verificar condición de término
        bool todosTerminados = true;
        for (const auto& p : procesos) {
            if (tiempoRestante[p.PID] > 0) {
                todosTerminados = false;
                break;
            }
        }
        if (todosTerminados) break;

        tiempoActual++;
    }

    return resultado;
}

void SchedulingWindow::calcularMetricas(const QVector<ResultadoSimulacion>& resultado) {
    QMap<QString, int> tiempoLlegada;
    QMap<QString, int> tiempoFinalizacion;
    QMap<QString, int> tiempoInicioEjecucion;
    QMap<QString, int> tiempoCPUUsado;

    // Inicializar con datos de los procesos
    for (const auto& p : procesosMap) {
        tiempoLlegada[p.PID] = p.AT;
        tiempoFinalizacion[p.PID] = 0;
        tiempoInicioEjecucion[p.PID] = -1;  // -1 indica que no ha iniciado
        tiempoCPUUsado[p.PID] = 0;
    }

    // Procesar todos los segmentos
    for (const auto& segmento : resultado) {
        // Registrar el primer inicio de ejecución
        if (tiempoInicioEjecucion[segmento.PID] == -1) {
            tiempoInicioEjecucion[segmento.PID] = segmento.inicio;
        }

        // El tiempo de finalización es el último segmento
        tiempoFinalizacion[segmento.PID] = segmento.inicio + segmento.duracion;
        tiempoCPUUsado[segmento.PID] += segmento.duracion;
    }

    // Calcular métricas según las definiciones corroboradas por el estimado Carlos Canteo
    //El completion time es el tiempo que pasa desde la submision hasta la finalizacion.
    //El turnaround es el que pasa desde el inicio de la ejecucion hasta la finalizacion
    //Y el response/wait es el tiempo desde submission hasta el inicio de ejecucion

    double totalCompletion = 0;
    double totalTurnaround = 0;
    double totalResponse = 0;
    int numProcesos = procesosMap.size();

    for (const auto& p : procesosMap) {
        // COMPLETION TIME: desde submission (AT) hasta finalización
        int completionTime = tiempoFinalizacion[p.PID] - tiempoLlegada[p.PID];

        // TURNAROUND TIME: desde inicio de ejecución hasta finalización
        int turnaroundTime = tiempoFinalizacion[p.PID] - tiempoInicioEjecucion[p.PID];

        // RESPONSE/WAIT TIME: desde submission (AT) hasta inicio de ejecución
        int responseTime = tiempoInicioEjecucion[p.PID] - tiempoLlegada[p.PID];

        totalCompletion += completionTime;
        totalTurnaround += turnaroundTime;
        totalResponse += responseTime;
    }

    // Mostrar promedios
    ui->metricsTextEdit->append("Avg Completion Time: " + QString::number(totalCompletion/numProcesos));
    ui->metricsTextEdit->append("Avg Turnaround Time: " + QString::number(totalTurnaround/numProcesos));
    ui->metricsTextEdit->append("Avg Waiting Time (Response): " + QString::number(totalResponse/numProcesos));
    ui->metricsTextEdit->append("--------------------------------");
}


void SchedulingWindow::animarSimulacion(const QVector<ResultadoSimulacion>& resultado, const QString& nombreAlgoritmo, int heightMult) {
    //limpiarEscena();
    ui->metricsTextEdit->append("Simulación: " + nombreAlgoritmo);

    // Configuración de espaciado
    const int BLOCK_HEIGHT = 30;
    const int VERTICAL_SPACING = 100;  // Espacio entre simulaciones
    const int BASE_Y_OFFSET = 20;     // Margen superior

    int yOffset = BASE_Y_OFFSET + (heightMult - 1) * VERTICAL_SPACING;

    // Inicializar variables de animación
    cicloAnimacion = 0;
    xAnimacion = 0;
    colorMapAnimacion.clear();
    colorIndexAnimacion = 0;
    tiemposEsperaAnimacion.clear();
    tiemposRespuestaAnimacion.clear();
    indexAnimacion = 0;
    bloqueActual = 0;  // contador para bloques dentro de un proceso
    resultadoActual = resultado;
    procesoActual = nullptr;  // Para rastrear el proceso actual
    enModoIdle = false;

    // Configuración inicial de la escena
    QPen pen(Qt::black);
    //escenaGantt->addLine(0, 30, 1000, 30, pen);
    ui->graphicsView->setScene(escenaGantt);

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]() {

        if (enModoIdle) {
            // Dibujar un solo bloque IDLE por tick del timer
            QGraphicsRectItem *idle = escenaGantt->addRect(xAnimacion, yOffset, 30, BLOCK_HEIGHT, pen, QBrush(Qt::lightGray));
            QGraphicsTextItem *text = escenaGantt->addText("IDLE");
            text->setPos(xAnimacion, yOffset + 5);
            xAnimacion += 30;
            cicloAnimacion++;

            // Verificar si hemos completado todos los bloques IDLE necesarios
            if (cicloAnimacion >= procesoActual->inicio) {
                enModoIdle = false;
                bloqueActual = 0;
            }
            return;
        }

        // Si no hay proceso actual, obtener el siguiente
        if (procesoActual == nullptr) {

            // esto es al final, cuando ya se pintaron todos
            if (indexAnimacion >= resultadoActual.size()) {
                // Dibujar números de ciclo
                for (int i = 0; i < cicloAnimacion; ++i) {
                    QGraphicsTextItem *cicloText = escenaGantt->addText(QString::number(i));
                    cicloText->setPos(i * 30, yOffset + BLOCK_HEIGHT + 5);
                }

                // Calcular métricas
                calcularMetricas(resultadoActual);

                timer->stop();
                timer->deleteLater();
                // Emitir señal después de limpiar
                QTimer::singleShot(0, this, [=]() {
                    emit simulacionTerminada();
                });
                return;
            }

            procesoActual = &resultadoActual[indexAnimacion];

            if (!globalColorMap.contains(procesoActual->PID)) {
                int colorIndex = globalColorMap.size() % coloresProcesos.size();
                globalColorMap[procesoActual->PID] = coloresProcesos[colorIndex];
            }

            // Entrar en modo IDLE si es necesario
            if (cicloAnimacion < procesoActual->inicio) {
                enModoIdle = true;
                return;  // Dejar que el próximo tick maneje el IDLE
            }

            bloqueActual = 0;  // Resetear contador de bloques
        }

        // Dibujar solo un bloque del proceso actual
        if (bloqueActual < procesoActual->duracion) {
            QColor procesoColor = globalColorMap[procesoActual->PID];
            QGraphicsRectItem *rect = escenaGantt->addRect(xAnimacion, yOffset, 30, BLOCK_HEIGHT, pen, QBrush(procesoColor));
            QGraphicsTextItem *text = escenaGantt->addText(procesoActual->PID);
            text->setPos(xAnimacion + 5, yOffset + 5);
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

    timer->start(400);  // Velocidad de animación (ms)
}


// aca voy a poner los destructores
void SchedulingWindow::limpiarEscena() {
    escenaGantt->clear();
    globalColorMap.clear();
}

SchedulingWindow::~SchedulingWindow()
{
    delete ui;
}
