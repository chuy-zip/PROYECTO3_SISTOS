#ifndef SCHEDULINGWINDOW_H
#define SCHEDULINGWINDOW_H

#include <QMainWindow>

namespace Ui {
class SchedulingWindow;
}

class SchedulingWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SchedulingWindow(QWidget *parent = nullptr);
    ~SchedulingWindow();

private:
    Ui::SchedulingWindow *ui;
};

#endif // SCHEDULINGWINDOW_H
