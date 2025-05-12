#ifndef LIBMOUSE_HPP
#define LIBMOUSE_HPP

#include <iostream>
#include <vector>
#include <mutex>
#include <complex>
#include <libusb-1.0/libusb.h>
#include <fstream>
#include <atomic>
#include <unistd.h>     // usleep()

#ifndef DEBUG_FUNCTION_CALL
#define DEBUG_FUNCTION_CALL
#endif

class Mouse
{
mutable std::mutex _mutexer;

enum
{
    SUCCESS = 0,
    ERROR = -1,
    CMD_TRANSFER_SIZE = 64,

    CMD_GET_MODE      = 0x9E, /* Read current operating mode */
    CMD_MODE_MASK     = 0x3F, /* */
    CMD_SLAVE_MODE    = 0xC0, /* Set FIFO slave mode */
    CMD_IDLE_MODE     = 0xC1, /* Set idle mode */
    CMD_GPIF_MODE     = 0xC2, /* Set GPIF transfer mode */
    CMD_GPIF_BLK_MODE = 0xC3, /* Set GPIF blockwise transfer mode */

    MOUSE_PRODUCT_ID = 0xee05,
    MOUSE_VENDOR_ID  = 0x2839,
    MOUSE_ADRESS     = 32,      // I2C Mouse spezifische Geraete Adresse
    ENDPOINT_1_OUT   = 0x01,     // Das ist der Config EP (duplex)
    ENDPOINT_1_IN    = 0x81,
    ENDPOINT_6_IN    = 0x86,

    /* Interpretation High Nibble  */
    I2C_ERROR          = 0xEE, /* Allgemeiner Fehler  */
    I2C_ERROR_NO_MOUSE = 0x10,
    I2C_ERROR_XMIT     = 0x20,
    I2C_ERROR_MISC     = 0x40,

    I2C_RETRIES    = 3,
    I2C_SLEEP_TIME = 50,

    // I2C Kommando-Definitionen  (Commandbyte)
    I2C_ERROR_BUSERR      = 0x01,
    I2C_ERROR_NOACK       = 0x02,
    I2C_ERROR_LOCKUP      = 0x03,
    I2C_ERROR_STOP_TIMEOUT = 0x04,

    CMD_SET_FIFO_FLUSH = 0x9D,

    CMD_I2C_WRITE    = 0x83, // Write data to I2C bus
    CMD_I2C_READ     = 0x84, // Read data from I2C bus
    CMD_I2C_SPEED    = 0x85, // Set I2C speed
    CMD_I2C_TRANSFER = 0x86, /*  */

    FILTER_BOUNDARIES_BAND_1 = 221, // Übergabe der Filtergrenzen HF-Band1 (string,11)
    FILTER_BOUNDARIES_BAND_2 = 222, // Übergabe der Filtergrenzen HF-Band1 (string,11)
    FILTER_BOUNDARIES_BAND_3 = 223, // Übergabe der Filtergrenzen HF-Band1 (string,11)
    FILTER_BOUNDARIES_BAND_4 = 224, // Übergabe der Filtergrenzen HF-Band1 (string,11)

    CMD_I2C_RECEIVER_MOUSE   = 15, // AD6636
    CMD_I2C_RECEIVER_MAX3543 = 9
};

libusb_device_handle *_mouse_dev;
std::string _error = {};
bool _mouse_is_receiver;
bool _is_open;

///// @brief Bettet ein Kommando in einen libusb_bulk_transfer ein und wertet den
/////        return-Wert aus
//void
//writeCommand(unsigned char command)
//{
//    std::vector<unsigned char> data_buffer(CMD_TRANSFER_SIZE); /* nimmt die Bytes auf  */
//    data_buffer[0] = command;
//    int32_t return_value = 0,
//            transfered   = 0;

//    std::lock_guard<std::mutex> lock( _mutexer);
//    /* Es folgt der eigentliche Sendevorgang.  */
//    return_value = libusb_bulk_transfer(_mouse_dev,
//                                        ENDPOINT_1_OUT,
//                                        data_buffer.data(),
//                                        1,
//                                        &transfered,
//                                        100);

//    /* Rueckgabewert ungleich NULL war nicht erfolgreich.  */
//    if(return_value)
//        throw std::runtime_error("FEHLER libmouse::writeCommand() "
//                                + std::string(libusb_error_name(return_value)));
//}
/// @brief Bettet ein Kommando in einen libusb_bulk_transfer ein und wertet den
///        return-Wert aus
std::vector<unsigned char>
writeCommand(unsigned char command)
{
    std::vector<unsigned char> data_buffer(CMD_TRANSFER_SIZE); /* nimmt die Bytes auf  */
    data_buffer[0] = command;
    int32_t return_value = 0,
            transfered   = 0;

    std::lock_guard<std::mutex> lock( _mutexer);
    /* Es folgt der eigentliche Sendevorgang.  */
    return_value = libusb_bulk_transfer(_mouse_dev,
                                        ENDPOINT_1_OUT,
                                        data_buffer.data(),
                                        1,
                                        &transfered,
                                        100);

    /* Rueckgabewert ungleich NULL: war nicht erfolgreich.  */
    if( return_value)
        throw std::runtime_error("FEHLER libmouse::writeCommand() "
                                + std::string(libusb_error_name(return_value)));

    return_value = libusb_bulk_transfer(_mouse_dev,
                                        ENDPOINT_1_IN,
                                        data_buffer.data(),
                                        4,
                                        &transfered,
                                        0);

    return data_buffer;
}

/// @brief  I2C adressiertes Kommando an den USB Controller senden und
///         Uebertragungsstatus pruefen.
///         Soll die Funktion USBI2CWriteByte() (nutzt bulk_transfer_out())
///          aus der usb2.dll ersetzen.
/// @param  command               zu sendende Daten
/// @return 0: alles normal, ERROR   bei Fehlern
//int32_t
//i2cWriteData(unsigned char command)
//{
//#ifdef DEBUG_FUNCTION_CALL
//    std::cerr << "libmouse::i2cWriteData()" << std::endl;
//#endif
//    std::vector<unsigned char> data_buffer;
//    data_buffer.reserve( CMD_TRANSFER_SIZE); // nimmt die Bytes auf

//    /* Belegung der zu senden Bytes  */
//    data_buffer.push_back( CMD_I2C_WRITE); /* CMD_TRANSFER_SIZE  */
//    data_buffer.push_back( (unsigned char)(MOUSE_ADRESS << 1));
//    data_buffer.push_back( 1); /* Anzahl der zu senden Bytes  */
//    data_buffer.push_back( 0x00); /* reserviert fuer Anzahl der Empfangsbytes */
//    data_buffer.push_back( command); /* I2C Kommando an den AVR  */

//    std::lock_guard<std::mutex> lock( _mutexer);

//    int32_t transfered   = 0;
//    /* Es folgt der eigentliche Sendevorgang.  */
//    int return_value = libusb_bulk_transfer(_mouse_dev,
//                                        ENDPOINT_1_OUT,
//                                        data_buffer.data(),
//                                        data_buffer.size(),
//                                        &transfered,
//                                        100);

//    /* Rueckgabewert ungleich NULL war nicht erfolgreich.  */
//    if(return_value)
//        throw std::runtime_error("FEHLER libmouse::i2cWriteData(1) "
//                                 + std::string(libusb_error_name(return_value)));

//    /* Der Rueckgabewert des vorherigen libusb_bulk_transfer() wurde im  */
//    /* Puffer gespeichert.  */
//    data_buffer.clear();
//    return_value = libusb_bulk_transfer(_mouse_dev,
//                                        ENDPOINT_1_IN,
//                                        data_buffer.data(),
//                                        CMD_TRANSFER_SIZE,
//                                        &transfered,  // Anzah
//                                        100);         // timeout

//    if(return_value)
//        throw std::runtime_error("FEHLER libmouse::i2cWriteData(2) "
//                                 + std::string(libusb_error_name(return_value)));

//    /* Rueckgabewert im Puffer auf Fehler in der Uebrtragung pruefen.  */
//    if(I2C_ERROR ==  data_buffer[2]) {
//        i2cCheckErrorCode(data_buffer[3]);

//        return ERROR;
//    }

//    return SUCCESS;
//}


/// @brief I2C adressierte Daten an den USB Controller senden und
///        Uebertragungsstatus pruefen. Soll die Funktion USBI2CWriteBytes()
///       (nutzt bulk_transfer_out()) aus der usb2.dll ersetzen.
/// @param  mouse    geoeffnetes USB Geraet (libusb_open())
/// @param  command   I2C Kommando an den ATMEGA
/// @param  data      zu uebertragende Daten
/// @param  bytes     Anzahl der zu sendenden Bytes
/// @return 0: alles normal, ERROR: bei Fehlern
void
i2cWriteData( const unsigned char command, const std::vector<unsigned char> &data = {}) {
    std::vector<unsigned char> data_buffer;
    data_buffer.reserve(CMD_TRANSFER_SIZE);

    /* Belegung der zu senden Bytes  */
    data_buffer.push_back( CMD_I2C_WRITE);
    data_buffer.push_back( MOUSE_ADRESS << 1);
    data_buffer.push_back( data.size() + 1); /* Anzahl der zu senden Bytes  */
    data_buffer.push_back( 0x00); /* reserviert fuer Anzahl der Empfangsbytes  */
    data_buffer.push_back( command); /* I2C Kommando an den AVR  */

    /* Payload in den Uebrtragungspuffer schreiben  */
    for( auto byte : data)
        data_buffer.push_back( byte);

    // wichtig ?
    std::lock_guard<std::mutex> lock( _mutexer);


    int32_t  transfered   = 0;
    /* Es folgt der eigentliche Sendevorgang.  */
    int return_value = libusb_bulk_transfer(_mouse_dev,
                                        ENDPOINT_1_OUT,
                                        data_buffer.data(),
                                        data_buffer.size(),
                                        &transfered,
                                        100);

    if(return_value) {
        throw std::runtime_error("FEHLER libmouse::i2cWriteData()"
                               + std::string( libusb_error_name(return_value)));
    }

    /* Der Rueckgabewert des vorherigen libusb_bulk_transfer() wurde im  */
    /* Puffer gespeichert.  */
    return_value = libusb_bulk_transfer(_mouse_dev,
                                        ENDPOINT_1_IN,
                                        data_buffer.data(),
                                        CMD_TRANSFER_SIZE,
                                        &transfered,
                                        100);
    if(return_value)
        throw std::runtime_error("FEHLER libmouse::i2cWriteData()"
                               + std::string( libusb_error_name(return_value)));
}


/// @brief  Daten an den USB Controller senden und Uebertragungsstatus pruefen.
///         Soll die Funktion bulk_transfer_out() aus der usb2.dll ersetzen.
/// @param  data zu empfangende Daten
/// @param  bytes Anzahl der zu sendenden Bytes
/// @return Anzahl der empfangenen Bytes, ERROR bei Fehlern
void
i2cReadData( std::vector<unsigned char> &input) {
    i2cReadData( input.data(), input.size());
}
void
i2cReadData( unsigned char *data, uint64_t leng) {
    std::vector<unsigned char> data_buffer( CMD_TRANSFER_SIZE, 0); /* nimmt die Bytes auf  */
    unsigned char length = 0;
    /* Belegung der zu senden Bytes  */
    data_buffer[length++] = CMD_I2C_READ; /* CMD_TRANSFER_SIZE  */
    data_buffer[length++] = (unsigned char)(MOUSE_ADRESS << 1);

    /* Null, diese Stelle wird bei i2cWriteData genutzt.  */
    data_buffer[length++] = 0x00;
    data_buffer[length++] = static_cast<unsigned char>( leng); /* Anzahl der zu lesenden Bytes  */

    /* Daten vor Zweitzugriff schuetzen. - noch umzusetzen  */
//     interface_lock_mutex();

    int32_t transfered   = 0;

    /* Es folgt der eigentliche Sendevorgang.  */
    unsigned char return_value = libusb_bulk_transfer( _mouse_dev,
                                                       ENDPOINT_1_OUT,
                                                       data_buffer.data(),
                                                       length,
                                                       &transfered,
                                                       100);

    /* Rueckgabewert ungleich NULL war nicht erfolgreich.  */
    if( return_value)
        throw std::runtime_error("FEHLER libmouse::i2cReadData()"
                               + std::string( libusb_error_name(return_value)));

    /* Der Rueckgabewert des vorherigen libusb_bulk_transfer() wurde im  */
    /* Puffer gespeichert.  */
    data_buffer.clear();
    data_buffer.resize( CMD_TRANSFER_SIZE);
    return_value = libusb_bulk_transfer(_mouse_dev,
                                        ENDPOINT_1_IN,
                                        data_buffer.data(),
                                        data_buffer.size(),
                                        &transfered,
                                        100);

    if(return_value)
        throw std::runtime_error("FEHLER libmouse::i2cReadData()"
                               + std::string( libusb_error_name(return_value)));


    /* Rueckgabewert im Puffer auf Fehler in der Uebrtragung pruefen.  */
    if(data_buffer[2] == I2C_ERROR) {
        i2cCheckErrorCode(data_buffer[3]);
    }

    /* Empfangene Daten in den uebergebenen Puffer kopieren.  */
    for(int32_t w = 4; w < transfered; w++)
        data[w - 4] = data_buffer[w];
}


/// @brief Interpretiert das High und das Low Nibble bezueglich des Fehlercodes.
/// @param i2c_error_code_byte
void
i2cCheckErrorCode(unsigned char i2c_error_code_byte) {
    /* Interpretiert das High Nibble.  */
    switch(i2c_error_code_byte & 0xF0)
    {
        case I2C_ERROR_NO_MOUSE: { fprintf(stderr, "FEHLER kein Geraet erkannt\n");
            break;
        }
        case I2C_ERROR_XMIT: { fprintf(stderr, "FEHLER Uebertragungsfehler\n");
            break;
        }
        case I2C_ERROR_MISC: { fprintf(stderr, "FEHLER MISC Fehler\n");
            break;
        }
        default: { fprintf(stderr, "FEHLER unbekannt\n");
            break;
        }
    }

    /* Interpretiert das Low Nibble.  */
    switch(i2c_error_code_byte & 0x0F)
    {
        case I2C_ERROR_BUSERR: { fprintf(stderr, "FEHLER Bus\n");
            break;
        }
        case I2C_ERROR_NOACK: { fprintf(stderr, "FEHLER keine Antwort\n");
            break;
        }
        case I2C_ERROR_LOCKUP: { fprintf(stderr, "FEHLER Locked up\n");
            break;
        }
        case I2C_ERROR_STOP_TIMEOUT: { fprintf(stderr, "FEHLER STOP Zeit abgelaufen\n");
            break;
        }
        default: { fprintf(stderr, "FEHLER unbekannt\n");
            break;
        }
    }
}


//int set_gpif_mode(libusb_device_handle *usb_device)
//{
//    unsigned char buffer[CMD_TRANSFER_SIZE],
//            return_value,
//            length = 0,
//            retval = 0;

//    int32_t  transfered   = 0;

//    buffer[length++] = CMD_GPIF_MODE;


//    _mutex.lock();
//    return_value = libusb_bulk_transfer(usb_device,
//                                        ENDPOINT_1_OUT,
//                                        buffer,
//                                        length,
//                                        &transfered,
//                                        0);

//    retval = bulk_transfer_out ( device_id, buffer, length );
//    if ( retval != length )
//    {
//        interface_release_mutex();
//        return E_FAIL;
//    }

//    if ( check_returncode ( device_id,  CMD_GPIF_MODE, buffer, 1 ) != E_OK )
//    {
//        interface_release_mutex();
//        return E_FAIL;
//    }

//	while ( get_current_mode(device_id) != MODE_GPIF );

//    interface_release_mutex();
//    return E_OK;
//}


public:

bool isOpen() const { return _is_open;}

Mouse(void) : _mouse_is_receiver( true) , _is_open( false)
{

}
~Mouse(void) {
    close();
}



/// @brief Versucht ein angeschlossene MOUSE zu oeffnen
int
open(void) {
    libusb_context *ctx = NULL;

    int32_t return_value = 0;
    /* Pruefen, ob libsub-Sitzung initialisiert wurde.  */
    if((return_value = libusb_init(&ctx))){
#ifdef DEBUG
        throw std::runtime_error("FEHLER mouse::open(): "
                                + std::string(libusb_error_name(return_value)));
#endif
        _error = ("FEHLER Mouse::open(): "
                + std::string(libusb_error_name(return_value)));

        return -1;
    }


    if( ! (_mouse_dev = libusb_open_device_with_vid_pid(ctx,
                                                         MOUSE_VENDOR_ID,
                                                         MOUSE_PRODUCT_ID))) {
#ifdef DEBUG
        throw std::runtime_error("FEHLER mouse::open() "
                           "MOUSE nicht angeschlossen oder sudo vergessen?\n");
#endif
        _error = ("FEHLER Mouse::open(): "
                   "MOUSE nicht angeschlossen oder sudo vergessen?\n");

        return -1;
    }

    if( (return_value = libusb_kernel_driver_active( _mouse_dev, 0))
        && (return_value = libusb_detach_kernel_driver( _mouse_dev, 0))) {
        _error = ("FEHLER Mouse::open(): libusb_detach_kernel_driver() "
                   + std::string(libusb_error_name(return_value)));
        return ERROR;
    }

    if(( return_value = libusb_claim_interface(_mouse_dev, 0))) {
        _error = ("FEHLER Mouse::open(): libusb_claim_interface() "
                   + std::string(libusb_error_name(return_value)));
        return ERROR;
    }

    _is_open = true;
    return 0;
}

void
close(void) {
    if(_is_open) {
        libusb_release_interface( _mouse_dev, 0);
        libusb_close( _mouse_dev);
        _is_open = false;
    }
}

std::string getError(void) const {return _error;}
std::vector<std::vector<uint32_t>> getFilter(void) { return i2cReadFilter();}


/// @brief Schaltet Tiefpass-Filter UND Samplerate um
/// @brief Index des auf der MOUSE verfuegbaren Filter
/// @return Abtastrate der Filtereinstellung
int
setFilter( uint32_t index) {
    if( ! _is_open) {return -1;}
    std::vector<unsigned char> buffer( 10, 0);
    i2cWriteData( 100);
    usleep( 1000);
    i2cReadData( buffer.data(), 1);
    uint64_t filter_count = static_cast<uint64_t>( buffer.at( 0));
    if( index >= filter_count)
        throw std::invalid_argument( "FEHLER setFilter(): "
                                     + std::to_string(index) + " >= "
                                     + std::to_string(filter_count));
    // Setze Filter - Kommando
    i2cWriteData( 101 + index, { 1});
    usleep( 1000);
    i2cReadData( buffer);
    usleep( 25000);

    int32_t bw = reinterpret_cast<int*>(&buffer.data()[1])[0];
    int32_t sps  = reinterpret_cast<int*>(&buffer.data()[1])[1];

    std::cerr << "bw: " << bw << std::endl;
    std::cerr << "sps: " << sps << std::endl;

    return sps;
}

/// @brief Setzt zum einen die Mittenfrequenz und zum anderen die frequenzab.
///        Empfaenger
void
setCenterFrequency(int32_t frequency) {
    frequency = std::min(1240000000, std::max(5000, frequency));
    std::vector<unsigned char> freq;
    freq.reserve( 4);
    freq.push_back(frequency);
    freq.push_back(frequency >> 8);
    freq.push_back(frequency >> 16);
    freq.push_back(frequency >> 24);

//    freq.push_back(frequency >> 24);
//    freq.push_back(frequency >> 16);
//    freq.push_back(frequency >> 8);
//    freq.push_back(frequency);

    // Frequenzabhaengige Empfangsbausteine beachten ( < 40MHz / >= 40 MHz)
    if( frequency > 40e6) {
        if( _mouse_is_receiver) {
            i2cWriteData( 30);
            usleep( 2000);
            i2cWriteData( CMD_I2C_RECEIVER_MAX3543, freq);
            usleep( 10000);
            _mouse_is_receiver = false;
        }
        std::cerr << "CMD_I2C_RECEIVER_MAX3543 "
                  << frequency <<std::endl;
        i2cWriteData( CMD_I2C_RECEIVER_MAX3543, freq);
    }
    else {
        i2cWriteData( CMD_I2C_RECEIVER_MOUSE, freq);
        _mouse_is_receiver = true;
    }
    usleep( 2000);
}

/// @brief  Liest die Anzahl und Bestimmt den Typ der in der MOUSE verfuegbaren
///         Filter um diese spaeter in einer GUI darzustellen.
/// @param  mouse
/// @param  filter_bandwidth
/// @param  filter_middlefrq
/// @return Anzahl der gelesenen Filter, ERROR: bei Fehlern
std::vector<std::vector<uint32_t>>
i2cReadFilter(void) {
    std::vector<unsigned char> buffer( 30, 0);

    /* Command 100: Filteranzahl im AVR auslesen  */
    i2cWriteData(100);
    usleep(1000);
    i2cReadData(buffer.data(), 1);
    uint64_t filter_count = static_cast<uint64_t>( buffer.at( 0));

    /* Auslesen der einzelnen Filterwerte  */
    std::vector<std::vector<uint32_t>> output;
    for(uint64_t w = 0; w < filter_count; ++w) {
        /* Die Nummer des Filters als I2C Kommando uebergeben.  */
        i2cWriteData( 101 + w);
        usleep(1000);
        std::fill(buffer.begin(), buffer.end(), 0);
        i2cReadData(buffer.data(), 9);
        const auto puff(buffer);
        std::vector<uint32_t> filter(2);
        // Filterweite
        filter[1] = (buffer[3] << 16) | (buffer[2] << 8)
                  | (buffer[1] << 0);// | (buffer[0]);
        // Samplerate
        filter[0] = (buffer[7] << 16) | (buffer[6] << 8)
                  | (buffer[5] <<  0);// | (buffer[4]);
        filter[1] = static_cast<uint32_t>( filter[1] / 2);
        output.push_back(filter);
    }

    return output;
}

void setGPIFMode() { writeCommand(CMD_GPIF_MODE); }
void setIDLEMode() { writeCommand(CMD_IDLE_MODE); }
void setSLAVEMode(){ writeCommand(CMD_SLAVE_MODE); }
void setFIFOFlush(void) {writeCommand(CMD_SET_FIFO_FLUSH); }


/// @brief Diese FUNKTION ist zum Daten abholen, diese kommen im INT16 Format!!
/// @param output int16
/// @return Anzahl gelesener Bytes
int32_t
streamData( std::vector<std::complex<int16_t>> &output) {
    int32_t transfered = 0;

    std::unique_lock<std::mutex> _mutexer;

    int return_value = libusb_bulk_transfer(_mouse_dev,
                                ENDPOINT_6_IN,
                                reinterpret_cast<unsigned char*>( output.data()),
                                output.size() * sizeof( std::complex<int16_t>),
                                &transfered,
                                10000);
#ifdef DEBUG
    if( transfered != output.size() * sizeof( std::complex<int16_t>))
        std::cerr << "streamData: " << transfered << " != " << output.size()
                  << std::endl;
#endif
    if( return_value)
        throw std::runtime_error("FEHLER Mouse::streamData(): "
                                 "libusb_bulk_transfer() "
                                + std::string( libusb_error_name( return_value)));

    return transfered / sizeof( std::complex<int16_t>);
}

/// @brief Gibt den index des MOUSE-Modi zurueck
/// @return 1: CMD_IDLE, 2: CMD_GPIF
unsigned char getCurrentMode() { return writeCommand( CMD_GET_MODE).at( 1);}

};


#endif // LIBMOUSE_HPP
