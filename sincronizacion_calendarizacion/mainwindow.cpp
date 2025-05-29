#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "schedulingwindow.h"       // Incluir las nuevas ventanas
#include "synchronizationwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Conectar botones a las nuevas ventanas
    connect(ui->btn_calendarizacion, &QPushButton::clicked, [this]() {
        SchedulingWindow *schedWindow = new SchedulingWindow(this);
        schedWindow->show();
    });

    connect(ui->btn_sincronizacion, &QPushButton::clicked, [this]() {
        SynchronizationWindow *syncWindow = new SynchronizationWindow(this);
        syncWindow->show();
    });
}

MainWindow::~MainWindow() {
    delete ui;
}

