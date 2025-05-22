#include "mainwindow.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QVBoxLayout *qvbl_main = new QVBoxLayout;
    QHBoxLayout *qhbl_sec = new QHBoxLayout;

    // MouseGui provides Control-Command and Data-Streaming
    MouseGUI *maus_gui = new MouseGUI();
    Sonarview *wfv = new Sonarview();

    QObject::connect( maus_gui, &MouseGUI::centerFreqChanged, &wfv->_axis, &Axis::chaneCenterFreq);
    QObject::connect( maus_gui, &MouseGUI::bandwidthChanged, &wfv->_axis, &Axis::setBandwidth);

    FileWriterWidget *fww = new FileWriterWidget;
    UDPSenderWidget *udp = new UDPSenderWidget;

    wfv->startProcessing();
    maus_gui->addStreamSink( std::bind( &Sonarview::dataIn, wfv, std::placeholders::_1));
    maus_gui->addStreamSink( std::bind( &FileWriterWidget::writeToFile, fww, std::placeholders::_1));
    maus_gui->addStreamSink( std::bind( &UDPSenderWidget::sendData<std::complex<float>>, udp, std::placeholders::_1));

    qvbl_main->addWidget( maus_gui);
    qvbl_main->addWidget( fww);
    qvbl_main->addWidget( udp);

    qhbl_sec->addWidget( wfv);
    qvbl_main->addLayout( qhbl_sec);

    QWidget *qw_main = new QWidget(this);
    qw_main->setLayout(qvbl_main);
    setCentralWidget(qw_main);
    setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
}

MainWindow::~MainWindow()
{

}


void MainWindow::closeEvent(QCloseEvent *event) {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Warnung", "Programm wirklich beenden",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        event->ignore();
    }
}
