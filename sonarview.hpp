#ifndef Sonarview_NEW_H
#define Sonarview_NEW_H

#include <QWidget>
#include <QPainter>
#include <QThread>
#include <QImage>
#include <QResizeEvent>
#include <QLabel>
#include <QColorSpace>
#include <QMutexLocker>
#include <QSplitter>
#include <QVBoxLayout>
#include <QMouseEvent>

#include <QStaticText>

#include <QMouseEvent>

#include <fstream>
#include <iostream>
#include <complex>
#include <vector>
#include <thread>
#include <execution>

#include "conditionalsafequeue.hpp"
#include "fft.hpp"
#include "tools.hpp"


class Marker : public QWidget{
    Q_OBJECT
    // void
    // mousePressEvent(QMouseEvent *event) {
    //     if(event->buttons() & Qt::Key_Shift & Qt::LeftButton)
    //     {
    //     }

    //     if(event->buttons() & Qt::LeftButton)
    //     {
    //         // absolute Amplitude-Position in bestimmen
    //         int mouse_pos = event->pos().x();

    //         // Pruefen, ob Marker verschoben werden solln
    //         // if(mouse_pos == m_marker_one_pos)
    //         // {
    //         //     m_drag_marker_one = true;
    //         // }
    //         // else if(mouse_pos == m_marker_two_pos)
    //         // {
    //         //     m_drag_marker_two = true;
    //         // }
    //         // // setze den Marker
    //         // else
    //         // {
    //         //     if(m_marker_status == 0) {
    //         //         setMarkerOne((event->pos().x() - y_width) / m_x_axes_ratio);
    //         //         m_marker_status = 1;
    //         //     }
    //         //     else if(m_marker_status == 1)  {
    //         //         setMarkerTwo((event->pos().x() - y_width) / m_x_axes_ratio);
    //         //         m_marker_status = 0;
    //         //     }
    //         // }
    //     }
    // }


    // /// @brief Zustaendig fuer:
    // ///
    // void
    // mouseMoveEvent(QMouseEvent *event){
    //     Q_UNUSED( event);
    //     // if(_magnitudes.empty()) return;

    //     // // Amplituden-Position bestimmen

    //     // if(mouse_pos < 0) return;

    //     // if(m_drag_marker_one)
    //     // {
    //     _x_a = event->pos().x();
    //     // }
    //     // if(m_drag_marker_two)
    //     // {
    //     //     setMarkerTwo((event->pos().x() - y_width) / m_x_axes_ratio);
    //     // }


    //     // if(mouse_pos == m_marker_one_pos)
    //     //     this->setCursor(Qt::SizeHorCursor);
    //     // else if(mouse_pos == m_marker_two_pos)
    //     //     this->setCursor(Qt::SizeHorCursor);
    //     // else
    //     //     this->setCursor(Qt::ArrowCursor);

    //     // if(event->buttons() & Qt::LeftButton)
    //     //     m_selection_start = event->pos().x();
    // }
protected:
    void
    mousePressEvent(QMouseEvent *event) override {
        if(event->button() & Qt::LeftButton) {
            _x_a = event->pos().x();
            std::cerr << "_x_a: " << _x_a << std::endl;
        }
        //    if(event->button() & Qt::LeftButton)
        //    {
        //        m_selection_stop = event->pos().x();
        //        QPainter painter(this);
        //        painter.drawRect(0, )
        //    }
        // m_drag_marker_one = false;
        // m_drag_marker_two = false;
    }

public:
    Marker() : _x_a( 0), _x_b( 0) {}

    void setPosA( int32_t x){ _x_a = x;}
    void setPosB( int32_t x){ _x_b = x;}

    void draw( QImage *image) {
        QPainter qpaint(image);
        qpaint.setPen( QPen( Qt::green));

        qpaint.drawLine( _x_a, image->height(), _x_a, 0);
        qpaint.drawText( _x_a + 5, static_cast<int>( image->height() * 0.9), "a");

        qpaint.drawLine( _x_b, image->height(), _x_b, 0);
        qpaint.drawText( _x_b + 5, static_cast<int>( image->height() * 0.9), "b");
    }

private:
    uint64_t _x_a, _x_b;

};


class Axis : public QObject {
    Q_OBJECT

public:
    Axis() {}

    /// @brief raws Axis on an Image
    void draw( QImage *image, float rel_y_pos = 0.9, float rel_size = 0.05) {
        if( ! image) return;
        QPainter qpaint( image);
        qpaint.setPen( QPen( Qt::black));

        int y = static_cast<int>( image->height() * rel_y_pos);
        qpaint.drawStaticText( .0, y, QStaticText( QString( "[MHz]")));
        qpaint.drawStaticText( static_cast<int>( image->width() * 0.1),
                              y, QStaticText( QString::number( (_center_freq - _bandwidth / 2) / 1e6, 'f', 3)));
        qpaint.drawStaticText( static_cast<int>( image->width() * 0.5),
                              y, QStaticText( QString::number( _center_freq / 1e6, 'f', 3)));
        qpaint.drawStaticText( static_cast<int>( image->width() * 0.9),
                              y, QStaticText( QString::number( (_center_freq + _bandwidth / 2) / 1e6, 'f', 3)));

    }

public slots:
    void chaneCenterFreq( int32_t freq) { _center_freq = freq;}
    void setBandwidth( int32_t bandwidth) { _bandwidth = bandwidth;}
private:
    int32_t _center_freq, _bandwidth;
};



class Waterfall : public QWidget {

};



/// @brief Ordinary Constructor for showview
class Sonarview : public QWidget{
    Q_OBJECT

public:
    explicit Sonarview( uint64_t fft_exp = 10) : _spectrogramm(nullptr),
        _psd(nullptr), _current_line(0), _is_processing(false), _fft(nullptr),
        _draw_upsidedown(true), _avg(nullptr) {

        setFFTExp( fft_exp);
        _buf_fft.resize( _fft->leng());
        _buf_fft_abs.resize( _fft->leng());
        _avg = new Tools::MovingAverage<float>( 5);

        _qs_splitter = new QSplitter( this);
        _qs_splitter->setOrientation( Qt::Vertical);
        connect( _qs_splitter, &QSplitter::splitterMoved, this, &Sonarview::rescaleLabels);

        _ql_spec = new QLabel( this);
        _ql_spec->setScaledContents( true);
        _ql_psd = new QLabel( this);
        _ql_psd->setScaledContents( true);

        _qs_splitter->addWidget( _ql_psd);
        _qs_splitter->addWidget( _ql_spec);
        _qs_splitter->setStretchFactor( 0, 1);
        _qs_splitter->setStretchFactor( 1, 1);

        QVBoxLayout *qvbl_main = new QVBoxLayout;
        qvbl_main->addWidget( _qs_splitter);
        setLayout( qvbl_main);
        setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        {
            QMutexLocker locker( &imageMutex);
            if( _spectrogramm && _psd) {
                _ql_spec->setPixmap( QPixmap::fromImage( _spectrogramm->copy(
                    0, getLine(), _spectrogramm->width(), _spectrogramm->height() / 2)));
                _ql_psd->setPixmap( QPixmap::fromImage( *_psd));
            }
        }
    }
    ~Sonarview() {
        stopProcessing();
        QMutexLocker locker( &imageMutex);
        delete _fft; delete _avg;
    }


    void startProcessing() {
        if( _is_processing) throw std::runtime_error("start a thread which is allready running");
        _is_processing = true;
        _proc = std::thread( &Sonarview::process, this);
    }

    void stopProcessing() {
        if( ! _is_processing) return;
        _is_processing = false;
        if( _proc.joinable())
            _proc.join();
    }

    // Reset der aktuellen fft
    void
    setFFTExp( const uint64_t fft_exp) {
        uint64_t leng = 1 << fft_exp;
        if( _fft) delete _fft;
        _fft = new FFT( leng);
        _buf_fft.resize( leng);
        _buf_fft_abs.resize( leng);
        _pos_overlap = leng / 4;
        setRows( 480);
    }

    /// ...push data to buffer
	/// ensure buffer is not overfilled , and input.size() matches fft::leng
    void dataIn( const std::vector<std::complex<float>> &input) {
        if( _puff.size() < 1) {
            std::vector<std::complex<float>> tmp = input;
            tmp.resize(_fft->leng(), std::complex<float>( .0 , .0));
            _puff.push( tmp);
        }
    }

    /// @brief Setzt die Anzahl der ffts ueber die gemittelt wird
    void setAverage( uint64_t avg) {_avg->setLeng( avg);}

    Axis _axis;
    Marker _marker;

public slots:
    void redraw() {
        this->repaint();
    }

private slots:
    // passt die Pixmaps der tatsaechlichen Anzeige (Labels) an
    void rescaleLabels() {
        QMutexLocker locker( &imageMutex);

        // Kopie des Spektrogramm auf die Fenstergroesse anpassen
        QImage tmp = _spectrogramm->copy( 0, getLine(), _spectrogramm->width(),
                        _spectrogramm->height() / 2).scaled(_ql_spec->size(),
                                 Qt::KeepAspectRatio, Qt::SmoothTransformation);

        // Wasserfalldiagramm zeichnen
        if( _draw_upsidedown) { tmp.mirror( false, true);}
        _marker.draw( &tmp);
        _ql_spec->setPixmap( QPixmap::fromImage( tmp));

        // scales psd to label::size
        QImage psd = _psd->scaled( _ql_psd->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        _marker.draw( &psd);
        _axis.draw( &psd);

        _ql_psd->setPixmap( QPixmap::fromImage( psd));
    }

protected:

    void mousePressEvent(QMouseEvent *event) override {
        if(event->button() & Qt::LeftButton) {
            _marker.setPosA( event->pos().x());
        }
    }
private:
    /// @brief Fuegt dem Sonargramm eine FFT Reihe hinzu
    /// @param input fft magnitudes in 10*log10
    void
    addLineToSpectrogram( const std::vector<float> &input) {
        if( _current_line >= _visible_rows) {
            _current_line = 0;
            // Bild von unterer Haelfte in obere haelfte kopieren, wenn ende erreicht
            for( int w = 0; w < _spectrogramm->width(); ++w)
                for( uint64_t x = 0; x < _visible_rows; ++x)
                    _spectrogramm->setPixel( w, x, _spectrogramm->pixel( w, _visible_rows + x));

        }
        uint64_t start_row = _visible_rows + _current_line;
        ++_current_line;

        // draw new column on spectro
        for( uint64_t w = 0; w < input.size(); ++w) {
            // to do: range setzen
            double x = 255. * (input.at( w) + 30.) / 60.;
            int i = std::clamp<int>( static_cast<int>( x) , 0, 255);
            _spectrogramm->setPixel( w, start_row, qRgb( 255 - i, 255 - i, 255 - i));
        }
    }

    /// @brief Fuegt das PSD unter das Sonargramm hinzu
    /// @param input 10*log10 magnitudes
    void
    addPSDToImage( const std::vector<float> &input) {
        if( ! _psd) return;
        QPainter qpaint( _psd);
        qpaint.setPen( QPen( Qt::black));

        std::vector<int> tmp( input.size());
        int height = _psd->height();
        std::transform( std::execution::par_unseq, input.begin(), input.end(), tmp.begin(),
                       [ height]( float val) { return static_cast<int>( height * (val-15) / -45.);});

        for( uint64_t w = 0; w < tmp.size() - 1; ++w) {
            qpaint.drawLine( w, tmp.at( w), w + 1, tmp.at( w + 1));
        }
    }

    /// @brief Sets the amount of rows in the QImage
    void
    setRows( uint64_t rows = 300) {
        QMutexLocker locker( &imageMutex);

        _visible_rows = rows;
        _current_line = rows;

        _spectrogramm = std::make_unique<QImage>( _fft->leng(), 2 * _visible_rows, QImage::Format_ARGB32);
        _spectrogramm->fill( Qt::white);
        _psd = std::make_unique<QImage>( _fft->leng(), static_cast<uint64_t>( _visible_rows),
                          QImage::Format_ARGB32);
    }

    /// gibt die aktuell Zeile an, auf die das PSD gezeichnet wird
    uint64_t getLine() const {
        if( _current_line <= _visible_rows ) {
            if( _current_line ) {
                return _current_line -1;
            }
            else {
                return _current_line;
            }
        }
        // avoid Scheiße
        return 0;
    }

    /// @brief processes data from _input_buf as long as there are any
    ///        -> wird als thread ausgef
    void process() {
        std::vector<std::complex<float>> data;

        while( _is_processing) {
            _puff.try_pop( _input_buf);

            uint64_t consumed = 0;
            while(( _input_buf.size() - consumed) >= _fft->leng()) {
                _fft->fft( _input_buf.data() + consumed, _buf_fft.data());
                Tools::abs<std::complex<float>, float>( _buf_fft, _buf_fft_abs);
                Tools::center( _buf_fft_abs);
                Tools::log10( _buf_fft_abs);

                std::this_thread::sleep_for( std::chrono::milliseconds( 20));
                _psd->fill( Qt::white);

                // push result to set images
                _avg->push( _buf_fft_abs);
                addLineToSpectrogram( _avg->getAverage());
                addPSDToImage( _avg->getAverage());
                rescaleLabels();

                // addLineToSpectrogram( _buf_fft_abs);
                // addPSDToImage( _buf_fft_abs);

                // update GUI
                QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);

                consumed += _fft->leng();

            }

            // Verarbeitete Samples aus dem Puffer entfernen
            if( consumed) _input_buf.erase( _input_buf.begin(), _input_buf.begin() + consumed);
        }
    }

    /// @brief setzt die Zoomstufe auf den Graphen
    void setZoom();



    ConditionSafeQueue<std::complex<float>> _puff;
    std::vector<std::complex<float>> _input_buf, _buf_fft, _buf_fft_2;
    std::vector<float> _buf_fft_abs, _sonat;

    uint64_t _file_byte_size;
    QPainter* _painter;
    std::unique_ptr<QImage> _spectrogramm, _psd;
    QPixmap _pixmap;
    QSplitter *_qs_splitter;
    QLabel *_ql_psd,
        *_ql_spec;

    std::fstream _file;
    uint64_t _chunk_leng;
    uint64_t _pos_overlap;
    uint64_t _current_line;
    uint64_t _visible_rows;

    std::atomic_bool _is_processing;

    std::thread _proc;
    QMutex imageMutex;

    FFT* _fft;
    std::vector<std::ifstream> _file_reader;
    int _mode;

    bool _draw_upsidedown;

    Tools::MovingAverage<float> *_avg;


};

#endif // Sonarview_NEW_H
