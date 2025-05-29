#include "synchronizationwindow.h"

SynchronizationWindow::SynchronizationWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Simulación de Mecanismos de Sincronización");
    resize(600, 400);
    // Aquí irá la lógica de mutex/semáforos
}
