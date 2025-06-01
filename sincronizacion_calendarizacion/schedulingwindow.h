#ifndef SCHEDULINGWINDOW_H
#define SCHEDULINGWINDOW_H

#include <QMainWindow>
#include <QString>

namespace Ui {
class SchedulingWindow;
}

class SchedulingWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit SchedulingWindow(QWidget *parent = nullptr);
    ~SchedulingWindow();

private slots:
    void onCargarArchivoClicked();   // Slot para cargar archivo
    void onEjecutarSimulacionClicked(); // Slot para simulación

private:
    Ui::SchedulingWindow *ui;
    QString contenidoArchivo;  // Variable para guardar el contenido del TXT
};

#endif // SCHEDULINGWINDOW_H
