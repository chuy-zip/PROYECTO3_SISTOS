// schedulingwindow.h
#ifndef SCHEDULINGWINDOW_H
#define SCHEDULINGWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QVector>
#include <QTimer>

namespace Ui {
class SchedulingWindow;
}

struct Proceso {
    QString PID;
    int BT;  // Burst Time
    int AT;  // Arrival Time
    int priority; // Prioridad (para otros algoritmos)
};

struct ResultadoSimulacion {
    QString PID;
    int inicio;
    int duracion;
};

class SchedulingWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit SchedulingWindow(QWidget *parent = nullptr);
    void ejecutarProximaSimulacion();
    ~SchedulingWindow();

private slots:
    void onCargarArchivoClicked();
    void onEjecutarSimulacionClicked();
    QVector<ResultadoSimulacion> ejecutarFIFO(const QVector<Proceso>& procesos);
    QVector<ResultadoSimulacion> ejecutarSJF(const QVector<Proceso>& procesosOriginales);
    QVector<ResultadoSimulacion> ejecutarSRT(const QVector<Proceso>& procesosOriginales);
    QVector<ResultadoSimulacion> ejecutarRR(const QVector<Proceso>& procesosOriginales, int quantum);
    QVector<ResultadoSimulacion> ejecutarPriorityAging(const QVector<Proceso>& procesosOriginales);

private:
    Ui::SchedulingWindow *ui;
    QGraphicsScene *escenaGantt;
    QVector<Proceso> procesos;
    QString contenidoArchivo;
    QVector<QColor> coloresProcesos;

    int colorIndex;

    QVector<std::function<void()>> simulaciones;
    int simulacionActual;

    QHash<QString, Proceso> procesosMap;  // Para métricas

    int cicloAnimacion;
    int xAnimacion;
    QHash<QString, QColor> colorMapAnimacion;
    int colorIndexAnimacion;
    QVector<int> tiemposEsperaAnimacion;
    QVector<int> tiemposRespuestaAnimacion;
    int indexAnimacion;
    QVector<ResultadoSimulacion> resultadoActual;

    int bloqueActual;                   // Para rastrear bloques dentro de un proceso
    const ResultadoSimulacion* procesoActual;

    void animarSimulacion(const QVector<ResultadoSimulacion>& resultado, const QString& nombreAlgoritmo, int heightMult);
    void calcularMetricas(const QVector<ResultadoSimulacion>& resultado);

    void dibujarDiagramaFIFO();
    void parsearArchivo(const QString &contenido);
    void limpiarEscena();

signals:
    void simulacionTerminada();

};

#endif // SCHEDULINGWINDOW_H
