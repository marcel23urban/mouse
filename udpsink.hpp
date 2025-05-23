#ifndef UDPSINK_HPP
#define UDPSINK_HPP

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QUdpSocket>

#include <complex>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>


// Der eigentlche UDP Verbindungsvorgang wird OHNE Qt gemacht, die notwendige GUI-Klasse folg unten
class UDPSender {
	struct sockaddr_in _dest_addr{};
	int _sockfd;

    // MTU 1500
    const uint64_t max_send_bytes = 1472;

public:
    UDPSender( std::string ip, uint16_t port) {
		// socket erstellen
        if( 0 > (_sockfd = socket( AF_INET, SOCK_DGRAM, 0))) {
			throw std::runtime_error("FEHLER kann socket nicht erstellen ");
		}
		
		// setzte ip:port und validiere
		_dest_addr.sin_family = AF_INET;
        //_dest_addr.sin_addr.s_addr =
		_dest_addr.sin_port = htons(port);
		if ( inet_pton( AF_INET, ip.c_str(), &_dest_addr.sin_addr) <= 0) {
			std::cerr << "Invalid address or address not supported: " << ip << std::endl;
			return;	
		}
		
		// verbindung herstellen
		if( connect( _sockfd, (struct sockaddr *)&_dest_addr, sizeof( _dest_addr)) < 0) {
			throw std::runtime_error("FEHLER UDP::connect");
		}
    }
	
	~UDPSender() {
        close(_sockfd);;
	}

	template< typename T>
    void sendData( const std::vector<T> &input) {
        uint64_t cnt = 0;
        while( cnt + max_send_bytes < input.size() * sizeof( T)) {
            send( _sockfd, &(reinterpret_cast<const char*>( input.data()))[cnt], max_send_bytes, 0);
            cnt += max_send_bytes;
        }
        if( cnt % max_send_bytes)
            send( _sockfd, &(reinterpret_cast<const char*>( input.data()))[cnt], cnt % max_send_bytes, 0);
	}
};

// Der GUI Teil vom eientlichen UDP Vorgang entkoppelt
class UDPSenderWidget : public QWidget {
    Q_OBJECT

    UDPSender *_udp;

public:
    UDPSenderWidget(QWidget *parent = nullptr) : QWidget(parent) {
        QHBoxLayout *qhbl = new QHBoxLayout(this);
        qhbl->addWidget(new QLabel("IP Address:"));
        ipInput = new QLineEdit("127.0.0.1");
        qhbl->addWidget(ipInput);
        qhbl->addWidget(new QLabel("Port:"));
        portInput = new QLineEdit("12345");
        qhbl->addWidget(portInput);

        startButton = new QPushButton("Start");
        stopButton = new QPushButton("Stop");
        stopButton->setEnabled(false);
        qhbl->addWidget(startButton);
        qhbl->addWidget(stopButton);

        statusLabel = new QLabel("Status: Stopped");
        qhbl->addWidget(statusLabel);

        // Connect signals and slots
        connect(startButton, &QPushButton::clicked, this, &UDPSenderWidget::startSending);
        connect(stopButton, &QPushButton::clicked, this, &UDPSenderWidget::stopSending);

        setLayout(qhbl);
    }

	template<typename T>
    void sendData( const std::vector<T> &input) {
        if( startButton->isEnabled()) {return ;}
        _udp->sendData<T>( input);
        std::cerr << "senddata: " << input.size() << std::endl;

        // std::cerr << "input: " << input.size() << std::endl;
        // _udp_socket->writeDatagram( reinterpret_cast<const char*>( input.data()),
        //                            input.size() / 2 * sizeof(std::complex<float>),
        //                            targetIP, targetPort);
        // _udp_socket->writeDatagram( reinterpret_cast<const char*>( &input.data()[input.size()/ 2]),
        //                            input.size() / 2 * sizeof(std::complex<float>),
        //                            targetIP, targetPort);
        // std::cerr << _udp_socket->errorString().toStdString() << std::endl;
    }

private slots:
    void startSending() {
        // Get IP and port from input fields
        _udp = new UDPSender( ipInput->text().toStdString(), portInput->text().toUInt());

        // Enable/disable buttons
        startButton->setEnabled(false);
        stopButton->setEnabled(true);
        statusLabel->setText("Status: Sending messages");
    }

    void stopSending() {
        // Enable/disable buttons
        startButton->setEnabled(true);
        stopButton->setEnabled(false);

        statusLabel->setText("Status: Stopped");
    }

private:
    QLineEdit *ipInput;
    QLineEdit *portInput;
    QPushButton *startButton;
    QPushButton *stopButton;
    QLabel *statusLabel;	
};

#endif // UDPSINK_HPP
