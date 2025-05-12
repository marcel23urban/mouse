#ifndef MOUSEGUI_HPP
#define MOUSEGUI_HPP

#include <vector>
#include <complex>
#include <thread>

#include <QMainWindow>
#include <QComboBox>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QFile>
#include <QThread>
#include <QMutex>
#include <QCheckBox>
#include <QFileDialog>
#include <QTimer>
#include <QDateTime>
#include <QMessageBox>
#include <QTextStream>

#include <limits>

#include "libmouse.hpp"


/// Control Widget for Mouse
class MouseGUI : public QWidget
{
    Q_OBJECT

    Mouse _maus;
    QTextEdit *_qte_user_info;
    QPushButton *_qpb_connection;
    QLineEdit *_qle_center_freq;
    QComboBox *_qcb_filter_select;
    QFile *_file_out;
    std::atomic_bool _is_streaming;
    bool _is_record;

    std::vector< std::complex<float>> output;
    std::thread _th;

    std::vector< std::function<void( const std::vector<std::complex<float>> &)>> _stream_sinks;
    const float _norm = 1.0 / static_cast<float>( std::numeric_limits<int16_t>::max());

public:

    MouseGUI( void) : _is_streaming(false) {
        createGUI();
        setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
        setMaximumHeight( 200);
    }

    ~MouseGUI(void) {
        if(_maus.isOpen())
            openClose();
        delete _qte_user_info;
        delete _qpb_connection;
        delete _qle_center_freq;
        delete _file_out;
    }

    /// @brief Fuegt dem Sample-Streaming-Thread eine Datensenke in form einer Zielfunktion hinzu
    void
    addStreamSink( const std::function<void( const std::vector<std::complex<float>> &)> &func) {
        _stream_sinks.push_back( func);
    }

private:
    void
    startStreaming(void) {
        if( ! _maus.isOpen() || _is_streaming) { return;}
        _th = std::thread( &MouseGUI::streaming, this);
        _is_streaming = true;
    }

    void
    stopStreaming() {
        if( ! _maus.isOpen() || ! _is_streaming) { return;}
        _is_streaming = false;
        if( _th.joinable())
            _th.join();
        else throw std::runtime_error("thread not joinable");
    }

    void outputData( const std::vector<std::complex<float>> input) {
        if ( _stream_sinks.empty())
            return;
        for( auto &sink : _stream_sinks)
            sink( input);
    }

    /// nebenlaeufige Funktion
    /// holt Daten von der MOUSE und sendet diese an alle _stream_sinks
    void streaming() {
        if( ! _is_streaming)
            return;
        std::vector<std::complex<int16_t>> in(  8 * 1024);
        std::vector<std::complex<float>> out;

        while( _is_streaming) {
            std::this_thread::sleep_for( std::chrono::milliseconds( 20));
            int32_t received = _maus.streamData( in);
            if( received != static_cast<int32_t>( in.size()))
                std::cerr << ": received != _tmp.size() - " << received << " != " <<
                    in.size() << std::endl;
            for( auto &val : in) {
                out.push_back( std::complex<float>(
                    static_cast<float>( val.real()) * _norm,
                    static_cast<float>( val.imag()) * _norm));
            }
            outputData( out);
            out.clear();
        }
        std::cerr << "leaving streaming" << std::endl;
    }

    void
    createGUI(void) {
        _qpb_connection = new QPushButton;
        _qpb_connection->setText("Verbindung getrennnt");
        connect(_qpb_connection, &QPushButton::clicked,
                this           , &MouseGUI::openClose);

        _qte_user_info = new QTextEdit;
        _qte_user_info->setMaximumHeight( 150 );

        _qle_center_freq = new QLineEdit;
        _qle_center_freq->setText("100.000000");
        _qle_center_freq->setMaximumWidth(100);
        connect(_qle_center_freq, &QLineEdit::editingFinished,
                this            , &MouseGUI::setCenterFrequency);

        _qcb_filter_select = new QComboBox;
        QHBoxLayout *qhbl_control = new QHBoxLayout;

        qhbl_control->addWidget(_qpb_connection);
        qhbl_control->addWidget( new QLabel("Frequenz [Mhz]: "));
        qhbl_control->addWidget( _qle_center_freq);
        qhbl_control->addWidget( new QLabel( "Filter: "));
        qhbl_control->addWidget( _qcb_filter_select);

        connect( _qcb_filter_select, &QComboBox::currentIndexChanged,
                this               , &MouseGUI::setFilter);

        QVBoxLayout *qvbl_main = new QVBoxLayout;
        qvbl_main->addLayout(qhbl_control);
        qvbl_main->addWidget(_qte_user_info);

        setLayout(qvbl_main);
    }

    /// Filter auslesen und als Menu darstellen
    void  loadFilter() {
        for( const std::vector<uint32_t> &filter : _maus.getFilter()) {
            double filt = static_cast<double>( filter.at( 1)) * 4 / 1000.0;
            double sps = static_cast<double>( filter.at( 0)) / 1000.0;
            _qcb_filter_select->addItem( QString::number( sps) + " kSps, " +
                                        QString::number( filt) + " kHz");
        }
    }

private slots:

    /// @brief Versucht eine angeschlossene Mouse zu oeffnen
    void
    openClose(void) {
        if(_maus.isOpen()) {
            stopStreaming();
            _maus.close();
            _qcb_filter_select->clear();
            _qpb_connection->setText( "Verbindung getrennt");
            _qpb_connection->setStyleSheet( "background-color: Pale gray; color: black;");
            return;
        }
        if(_maus.open()) {
            _qte_user_info->append(QString::fromStdString(_maus.getError()));
            return;
        }
        _qpb_connection->setText( "Verbindung hergestellt");
        _qpb_connection->setStyleSheet( "background-color: orange ; color: black;");

        // load mouse_internal filter-options
        loadFilter();
        // set streaming mode
        _maus.setGPIFMode();
        // start polling samples
        startStreaming();
        // set first center to 100 MHz
        emit setCenterFrequency();

}

public slots:

    void setFilter( int index) {
        if( ! _maus.isOpen()) return;
        int32_t sps = _maus.setFilter( index);

        emit bandwidthChanged( sps);
    }

    void setCenterFrequency() {
        if( ! _maus.isOpen()) return;
        int frequency = static_cast<int>(_qle_center_freq->text().toFloat() * 1000000.0);
        std::cerr << "INFO setCenterFrequency(): " << frequency << std::endl;
        _maus.setCenterFrequency(static_cast<int32_t>(frequency));
        emit centerFreqChanged( frequency);
    }

signals:

    void centerFreqChanged( int32_t freq);
    void bandwidthChanged( int32_t bandwidth);
};


#endif // MOUSEGUI_HPP
