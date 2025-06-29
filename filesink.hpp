#ifndef FILESINK_HPP
#define FILESINK_HPP

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
#include <QDateTime>
#include <QString>

#include <complex>
#include <vector>

class FileWriterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileWriterWidget(QWidget *parent = nullptr)
        : QWidget(parent), file(nullptr), writing(false)
    {
        // Initialize UI elements
        pathLineEdit = new QLineEdit(this);
        pathLineEdit->setPlaceholderText("Dateiauswahl...");

        QPushButton *browseButton = new QPushButton("Browse", this);
        connect(browseButton, &QPushButton::clicked, this, &FileWriterWidget::onBrowse);

        startStopButton = new QPushButton("Start", this);
        startStopButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum);
        connect(startStopButton, &QPushButton::clicked, this, &FileWriterWidget::onStartStop);

        infoLabel = new QLabel("Dauer: 0:00:00 | Größe: 0.000 GB", this);

        _append_mode = new QCheckBox;
        _append_mode->setText( "anfügen (sonst überschreiben)");

        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &FileWriterWidget::updateInfo);

        // Layout setup
        QHBoxLayout *qhbl_first = new QHBoxLayout;
        qhbl_first->addWidget(pathLineEdit);
        qhbl_first->addWidget(browseButton);

        QHBoxLayout *qhbl_second = new QHBoxLayout;
        qhbl_second->addWidget( _append_mode);
        qhbl_second->addWidget(startStopButton);
        qhbl_second->addWidget(infoLabel);


        QVBoxLayout *mainLayout = new QVBoxLayout;
        mainLayout->addLayout( qhbl_first);
        mainLayout->addLayout( qhbl_second);

        setLayout(mainLayout);
        setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum);
    }

    ~FileWriterWidget()
    {
        if (file && file->isOpen()) {
            file->close();
            delete file;
        }
    }

    /// @brief bekommt eine Funktion uebergeben, welche eine string in den Dateinamen anbringt
    void addString( std::function<std::string()> &foo) {
        _getString = foo;
    }

    // Public method to write data to the file
    void writeToFile(const std::vector<std::complex<float>> &input)
    {
        if (file && file->isOpen()) {
            file->write( reinterpret_cast<const char*>(input.data()),
                         input.size() * sizeof( std::complex<float>));
        }
    }

private slots:
    void onBrowse()
    {
        QString fileName = QFileDialog::getSaveFileName(
            this,                   // Parent widget
            tr("Save File"),        // Dialog title
            "/home",                     // Initial directory (empty string means home directory)
            tr("Text Files (*.txt);;All Files (*)")); // Filters
        if ( ! fileName.isEmpty()) {
            pathLineEdit->setText(fileName);
        }
    }

    void onStartStop()
    {
        if ( ! writing) {
            startTime = QDateTime::currentDateTime();
            QString filePath = pathLineEdit->text();
            if (filePath.isEmpty()) {
                QMessageBox::warning(this, "Warnung", "kein Datei ausgewählt.");
                return;
            }

            file = new QFile(filePath);
            bool good;
            if( _append_mode->isChecked()) {
                good = file->open( QFile::WriteOnly | QFile::Text | QFile::Append);
                updateInfo();
            }
            else
                good = file->open( QFile::WriteOnly | QFile::Text);

            if ( ! good) {
                QMessageBox::critical(this, "Error", "konnte Datei nicht öffnen.");
                delete file;
                file = nullptr;
                return;
            }

            // .toString("yyMMdd_hhmmss")
            writing = true;

            timer->start(500); // Update every second
            startStopButton->setText("Stop");
            startStopButton->setStyleSheet("background-color: red; color: black;");
        } else {
            if (file && file->isOpen()) {
                file->close();
                delete file;
                file = nullptr;
            }

            writing = false;
            timer->stop();
            startStopButton->setText("Start");
            startStopButton->setStyleSheet("background-color: Pale gray ; color: black;");
        }
    }

    void updateInfo()
    {
        // Update write duration
        qint64 duration = startTime.msecsTo(QDateTime::currentDateTime()) / 1000; // Duration in seconds
        QTime time(0, 0);
        time = time.addSecs(duration);

        // Update file size
        double sizeInGB = 0.0;
        if (file && file->isOpen()) {
            qint64 size = file->size();
            sizeInGB = static_cast<double>(size) / (1024 * 1024 * 1024); // Convert to GB
        }

        infoLabel->setText(QString("Dauer: %1 | Größe: %2 GB")
                               .arg(time.toString("h:mm:ss"))
                               .arg(sizeInGB, 0, 'f', 3));
    }

private:
    QLineEdit *pathLineEdit;
    QPushButton *startStopButton;
    QLabel *infoLabel;
    QCheckBox *_append_mode;

    QFile *file;
    QTimer *timer;
    QDateTime startTime;
    bool writing;

    std::function<std::string()> _getString;
};


#endif // FILESINK_HPP
