#include "schedulingwindow.h"
#include "ui_schedulingwindow.h"

SchedulingWindow::SchedulingWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SchedulingWindow)
{
    ui->setupUi(this);

    setWindowTitle("Simulación de Mecanismos de Calendarización");
    resize(600, 400);
}

SchedulingWindow::~SchedulingWindow()
{
    delete ui;
}
