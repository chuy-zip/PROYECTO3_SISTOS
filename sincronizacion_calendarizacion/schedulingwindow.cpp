#include "schedulingwindow.h"
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QBrush>

SchedulingWindow::SchedulingWindow(QWidget *parent) : QMainWindow(parent) {
    scene = new QGraphicsScene(this);
    QGraphicsView *view = new QGraphicsView(scene);
    setCentralWidget(view);
    setupUI();
}

void SchedulingWindow::setupUI() {
    setWindowTitle("Simulación de Algoritmos de Calendarización");

    // Ejemplo: Añadir un bloque de proceso (Gantt)
    QGraphicsRectItem *process = new QGraphicsRectItem(0, 0, 100, 30);
    process->setBrush(Qt::blue);  // Corregido: "Qt" con Q mayúscula
    scene->addItem(process);
}
