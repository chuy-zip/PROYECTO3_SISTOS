#include "synchronizationwindow.h"
#include "ui_synchronizationwindow.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QPen>
#include <QBrush>
#include <algorithm>


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
