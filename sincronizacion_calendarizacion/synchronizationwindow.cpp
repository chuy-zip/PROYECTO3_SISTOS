#include "synchronizationwindow.h"
#include "ui_synchronizationwindow.h"

SynchronizationWindow::SynchronizationWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SynchronizationWindow)
{
    ui->setupUi(this);

    setWindowTitle("Simulación de Mecanismos de Sincronización");
    resize(600, 400);
}

SynchronizationWindow::~SynchronizationWindow()
{
    delete ui;
}
