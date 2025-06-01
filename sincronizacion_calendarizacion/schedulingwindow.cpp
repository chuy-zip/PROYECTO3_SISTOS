#include "schedulingwindow.h"
#include "ui_schedulingwindow.h"

#include "schedulingwindow.h"
#include "ui_schedulingwindow.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDebug>

SchedulingWindow::SchedulingWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::SchedulingWindow) {
    ui->setupUi(this);
    setWindowTitle("Simulación de Algoritmos de Calendarización");
    resize(900, 700);

    // Conectar botones a sus slots
    connect(ui->btnCargarArchivo, &QPushButton::clicked, this, &SchedulingWindow::onCargarArchivoClicked);
    connect(ui->btnEjecutarSimulacion, &QPushButton::clicked, this, &SchedulingWindow::onEjecutarSimulacionClicked);
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
    if (ui->checkBoxFIFO->isChecked()) qDebug() << "Algoritmo FIFO seleccionado";
    if (ui->checkBoxSJF->isChecked()) qDebug() << "Algoritmo SJF seleccionado";
    if (ui->checkBoxRR->isChecked()) qDebug() << "Algoritmo SRT seleccionado";
    if (ui->checkBoxSRT->isChecked()) qDebug() << "Algoritmo Round Robin seleccionado";
    if (ui->checkBoxPriority->isChecked()) qDebug() << "Algoritmo FIFO seleccionado";

}

SchedulingWindow::~SchedulingWindow()
{
    delete ui;
}
