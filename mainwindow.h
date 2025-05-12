#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QCloseEvent>


#include "libmouse.hpp"
#include "mousegui.hpp"
#include "sonarview.hpp"
#include "filesink.hpp"
#include "udpsink.hpp"


class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);
};
#endif // MAINWINDOW_H
