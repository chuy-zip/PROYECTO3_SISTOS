#ifndef SYNCHRONIZATIONWINDOW_H
#define SYNCHRONIZATIONWINDOW_H

#include <QMainWindow>

namespace Ui {
class SynchronizationWindow;
}

class SynchronizationWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SynchronizationWindow(QWidget *parent = nullptr);
    ~SynchronizationWindow();

private:
    Ui::SynchronizationWindow *ui;
};

#endif // SYNCHRONIZATIONWINDOW_H
