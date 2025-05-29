#ifndef SCHEDULINGWINDOW_H
#define SCHEDULINGWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsView>

class SchedulingWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit SchedulingWindow(QWidget *parent = nullptr);
private:
    QGraphicsScene *scene;
    void setupUI();
};

#endif
