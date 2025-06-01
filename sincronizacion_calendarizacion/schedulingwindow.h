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

class SchedulingWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit SchedulingWindow(QWidget *parent = nullptr);
    ~SchedulingWindow();

private slots:
    void onCargarArchivoClicked();
    void onEjecutarSimulacionClicked();
    void ejecutarCicloFIFO();

private:
    Ui::SchedulingWindow *ui;
    QGraphicsScene *escenaGantt;
    QVector<Proceso> procesos;
    QString contenidoArchivo;
    QVector<QColor> coloresProcesos;

    QTimer *timerFIFO;
    int cicloActual;
    int xActual;
    int indiceProcesoFIFO;
    int tiempoEjecutadoProcesoActual;
    QHash<QString, QColor> coloresAsignados;
    int colorIndex;

    void dibujarDiagramaFIFO();
    void parsearArchivo(const QString &contenido);
    void limpiarEscena();
};

#endif // SCHEDULINGWINDOW_H
