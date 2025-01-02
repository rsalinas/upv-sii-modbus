#include "mainwindow.h"
#include <QMessageBox>
#include "timeseriesviewer.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Connect buttons to their slots
    connect(ui->pushButtonConnect, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(ui->pushButtonDisconnect, &QPushButton::clicked, this, &MainWindow::onDisconnectClicked);

    // Initial state: not connected
    ui->pushButtonConnect->setEnabled(true);
    ui->pushButtonDisconnect->setEnabled(false);
    // Show the connection status (to the right of the buttons)
    ui->labelConnectionStatus->setText("Disconnected");

    // Connect the connection status signal from TimeSeriesViewer to update the corresponding control
    TimeSeriesViewer *viewer = ui->oscilloscopeWidget;
    connect(viewer, &TimeSeriesViewer::connectionStatusChanged, this, &MainWindow::updateStatusBar);

    // Update the poll interval in real-time if changed in the UI
    connect(ui->spinBoxPollInterval,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            [viewer](int interval) {
                if (viewer->isConnected()) {
                    viewer->updatePollInterval(interval);
                }
            });

    // Update the time window in real-time if changed in the UI
    connect(ui->spinBoxTimeWindow,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            [viewer](int seconds) {
                if (viewer->isConnected()) {
                    viewer->updateTimeWindow(seconds);
                }
            });

    // Connect the signal to update the number of Modbus requests
    connect(viewer, &TimeSeriesViewer::modbusRequestCountChanged, this, [this](int count) {
        ui->labelRequestCount->setText(QString::number(count));
    });

    // Connect signals to update the request times
    connect(viewer, &TimeSeriesViewer::lastRequestTimeUpdated, this, [this](qint64 elapsed) {
        ui->labelLastRequestTime->setText(QString::number(elapsed) + " ms");
    });
    connect(viewer, &TimeSeriesViewer::averageRequestTimeUpdated, this, [this](double avg) {
        ui->labelAverageRequestTime->setText(QString::number(avg, 'f', 2) + " ms");
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onConnectClicked()
{
    // Read parameters from the UI
    QString host = ui->lineEditHost->text();
    int port = ui->spinBoxPort->value();
    int deviceAddress = ui->spinBoxDevice->value();
    int pollInterval = ui->spinBoxPollInterval->value();

    // Validate the poll interval
    if (pollInterval <= 0 || pollInterval >= 1000) {
        QMessageBox::warning(this,
                             tr("Invalid interval"),
                             tr("The poll interval must be greater than 0 and less than 1000 ms."));
        return;
    }

    TimeSeriesViewer *viewer = ui->oscilloscopeWidget;
    if (viewer->isConnected()) {
        QMessageBox::information(this, tr("Information"), tr("Already connected."));
        return;
    }

    // Attempt connection
    if (viewer->connectModbus(host, port, deviceAddress, pollInterval)) {
        ui->pushButtonConnect->setEnabled(false);
        ui->pushButtonDisconnect->setEnabled(true);
        // Disable host, port, and address fields while connected
        ui->lineEditHost->setEnabled(false);
        ui->spinBoxPort->setEnabled(false);
        ui->spinBoxDevice->setEnabled(false);
    } else {
        ui->pushButtonConnect->setEnabled(true);
        ui->pushButtonDisconnect->setEnabled(false);
        // Re-enable host, port, and address fields if connection fails
        ui->lineEditHost->setEnabled(true);
        ui->spinBoxPort->setEnabled(true);
        ui->spinBoxDevice->setEnabled(true);
    }
}

void MainWindow::onDisconnectClicked()
{
    TimeSeriesViewer *viewer = ui->oscilloscopeWidget;
    if (viewer->isConnected()) {
        viewer->disconnectModbus();
        ui->pushButtonConnect->setEnabled(true);
        ui->pushButtonDisconnect->setEnabled(false);
        // Re-enable host, port, and address fields
        ui->lineEditHost->setEnabled(true);
        ui->spinBoxPort->setEnabled(true);
        ui->spinBoxDevice->setEnabled(true);
    }
}

void MainWindow::updateStatusBar(const QString &status)
{
    // Update the connection status in the corresponding control
    ui->labelConnectionStatus->setText(status);
    TimeSeriesViewer *viewer = ui->oscilloscopeWidget;
    if (viewer->isConnected()) {
        ui->pushButtonConnect->setEnabled(false);
        ui->pushButtonDisconnect->setEnabled(true);
    } else {
        ui->pushButtonConnect->setEnabled(true);
        ui->pushButtonDisconnect->setEnabled(false);
        // Re-enable connection parameters if disconnected
        ui->lineEditHost->setEnabled(true);
        ui->spinBoxPort->setEnabled(true);
        ui->spinBoxDevice->setEnabled(true);
    }
}

void MainWindow::on_action_Clear_triggered()
{
    ui->oscilloscopeWidget->clearData();
}

void MainWindow::on_action_Exit_triggered()
{
    QApplication::exit(0);
}
