#include "timeseriesviewer.h"
#include <QCheckBox>
#include <QDateTime>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QVBoxLayout>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtMath>

TimeSeriesViewer::TimeSeriesViewer(QWidget *parent)
    : QWidget(parent)
    , time(0)
    , chart(nullptr)
    , pressureSeries(nullptr)
    , temperatureSeries(nullptr)
    , timer(new QTimer(this))
    , modbusReader(nullptr) // Not created at start
    , modbusRequestCount(0)
    , lastRequestTimestamp(0)
    , requestTimingCount(0)
    , requestTimingSum(0)
    , timeWindow(10) // Default to 10 seconds
    , checkBoxDiscreteInput(nullptr)
{
    // Create series for pressure and temperature
    pressureSeries = new QLineSeries();
    pressureSeries->setName("Pressure");
    temperatureSeries = new QLineSeries();
    temperatureSeries->setName("Temperature");

    // Configure the chart and add both series
    chart = new QChart();
    chart->addSeries(pressureSeries);
    chart->addSeries(temperatureSeries);

    // X-axis (time)
    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Time (s)");
    axisX->setRange(0, timeWindow);
    axisX->setLabelFormat("%.1f");
    chart->addAxis(axisX, Qt::AlignBottom);
    pressureSeries->attachAxis(axisX);
    temperatureSeries->attachAxis(axisX);

    // Y-axis for pressure
    QValueAxis *axisYPressure = new QValueAxis();
    axisYPressure->setTitleText("Pressure (bar)");
    axisYPressure->setRange(900, 1100);
    chart->addAxis(axisYPressure, Qt::AlignLeft);
    pressureSeries->attachAxis(axisYPressure);

    // Y-axis for temperature
    QValueAxis *axisYTemperature = new QValueAxis();
    axisYTemperature->setTitleText("Temperature (°C)");
    axisYTemperature->setRange(0, 200);
    chart->addAxis(axisYTemperature, Qt::AlignRight);
    temperatureSeries->attachAxis(axisYTemperature);

    // Create the ChartView to display the chart
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // Set up the layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(chartView);

    // ---- Nuevo: Panel para discrete inputs ----
    QHBoxLayout *discreteLayout = new QHBoxLayout();
    QLabel *labelDiscrete = new QLabel("Discrete inputs:");
    checkBoxDiscreteInput = new QCheckBox();
    // El checkbox se utiliza únicamente como indicador
    checkBoxDiscreteInput->setEnabled(false);
    discreteLayout->addWidget(labelDiscrete);
    discreteLayout->addWidget(checkBoxDiscreteInput);
    discreteLayout->addStretch();
    layout->addLayout(discreteLayout);
}

bool TimeSeriesViewer::connectModbus(const QString &host,
                                     int port,
                                     int deviceAddress,
                                     int pollInterval)
{
    // If already connected, do nothing.
    if (modbusReader)
        return true;

    // Create modbusReader with the parameters provided by the UI.
    modbusReader = new ModbusReader(host, port, deviceAddress, this);
    // Connect the signal to process received registers.
    connect(modbusReader,
            &ModbusReader::inputRegistersReady,
            this,
            &TimeSeriesViewer::processInputRegisters);
    // Connect the signal to process discrete coils.
    connect(modbusReader,
            &ModbusReader::discreteCoilsReady,
            this,
            &TimeSeriesViewer::processDiscreteCoils);
    // Connect the error signal to handle read errors.
    connect(modbusReader, &ModbusReader::modbusError, this, &TimeSeriesViewer::handleModbusError);

    // Attempt the real connection to the Modbus device.
    if (!modbusReader->connectDevice()) {
        QMessageBox::critical(this,
                              tr("Connection error"),
                              tr("Could not connect to the Modbus device.\n%1")
                                  .arg(modbusReader->errorString()));
        delete modbusReader;
        modbusReader = nullptr;
        return false;
    }

    // Reset statistics upon a new connection
    modbusRequestCount = 0;
    requestTimingCount = 0;
    requestTimingSum = 0;
    lastRequestTimestamp = 0;
    time = 0;

    // Connect the timer now (store the connection)
    m_timerConnection = connect(timer, &QTimer::timeout, this, [this]() {
        QPointer<ModbusReader> reader = modbusReader; // Capture a safe copy
        if (reader) {
            modbusRequestCount++;
            emit modbusRequestCountChanged(modbusRequestCount);
            // Record the timestamp for the current request
            lastRequestTimestamp = QDateTime::currentMSecsSinceEpoch();
            reader->readInputRegistersAsync(2, 0);
            reader->readDiscreteCoilsAsync(1, 0);
        }
    });

    // Configure and start the timer with the specified poll interval.
    timer->start(pollInterval);

    // Emit connection status.
    emit connectionStatusChanged(
        QString("Connected to %1:%2 (Device %3)").arg(host).arg(port).arg(deviceAddress));
    return true;
}

void TimeSeriesViewer::disconnectModbus()
{
    if (modbusReader) {
        timer->stop();
        // Disconnect the timer signal to prevent any queued calls from firing
        disconnect(m_timerConnection);
        modbusReader->disconnectDevice();
        modbusReader->deleteLater();
        modbusReader = nullptr;
        emit connectionStatusChanged("Disconnected");
    }
}

void TimeSeriesViewer::processInputRegisters(const QVector<quint16> &registers)
{
    // If fewer than 2 registers are received, consider it an error.
    if (registers.size() < 2) {
        handleModbusError(tr("Incomplete register read. At least 2 values were expected."));
        return;
    }

    double pressure = registers[0];
    double temperature = registers[1];
    time += timer->interval() / 1000.0; // Update time in seconds

    // Add new data points
    pressureSeries->append(time, pressure);
    temperatureSeries->append(time, temperature);

    // Remove old points to keep only those within the time window
    while (!pressureSeries->points().isEmpty()
           && pressureSeries->points().first().x() < time - timeWindow) {
        pressureSeries->remove(0);
    }
    while (!temperatureSeries->points().isEmpty()
           && temperatureSeries->points().first().x() < time - timeWindow) {
        temperatureSeries->remove(0);
    }

    // Update the range of the X-axis using the current time window
    QValueAxis *axisX = static_cast<QValueAxis *>(chart->axes(Qt::Horizontal).first());
    if (axisX) {
        axisX->setRange(time - timeWindow, time);
    }

    // If a timestamp was recorded for the request, calculate the elapsed time
    if (lastRequestTimestamp != 0) {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        qint64 elapsed = now - lastRequestTimestamp;
        // Update timing statistics
        requestTimingCount++;
        requestTimingSum += elapsed;
        double averageTime = requestTimingSum / static_cast<double>(requestTimingCount);
        // Emit signals to update the UI
        emit lastRequestTimeUpdated(elapsed);
        emit averageRequestTimeUpdated(averageTime);
        // Reset the timestamp to avoid counting signals without a request
        lastRequestTimestamp = 0;
    }
}

void TimeSeriesViewer::processDiscreteCoils(const QBitArray &coils)
{
    if (coils.size() > 0) {
        checkBoxDiscreteInput->setChecked(coils.testBit(0));
    } else {
        qWarning() << "processDiscreteCoils returned no data";
    }
}

void TimeSeriesViewer::handleModbusError(const QString &error)
{
    // Show the error and disconnect to be ready for a new connection
    QMessageBox::critical(this, tr("Modbus read error"), error);
    disconnectModbus();
}

void TimeSeriesViewer::updatePollInterval(int interval)
{
    // Validate the interval (must be > 0 and <= 1000 ms)
    if (interval > 0 && interval <= 1000) {
        timer->setInterval(interval);
    } else {
        QMessageBox::warning(this,
                             tr("Invalid interval"),
                             tr("The poll interval must be between 0 and 1000 ms."));
    }
}

void TimeSeriesViewer::updateTimeWindow(int seconds)
{
    // Update the rolling time window and adjust the X-axis range accordingly.
    if (seconds < 1)
        seconds = 1;
    else if (seconds > 3600)
        seconds = 3600;
    timeWindow = seconds;

    // Update the X-axis range
    QValueAxis *axisX = static_cast<QValueAxis *>(chart->axes(Qt::Horizontal).first());
    if (axisX) {
        // When time is less than the new window, start at 0
        double start = (time > timeWindow) ? time - timeWindow : 0;
        axisX->setRange(start, time);
    }
}

void TimeSeriesViewer::clearData()
{
    pressureSeries->clear();
    temperatureSeries->clear();
    time = 0;
    // Reset the X-axis using the current time window
    QValueAxis *axisX = static_cast<QValueAxis *>(chart->axes(Qt::Horizontal).first());
    if (axisX) {
        axisX->setRange(0, timeWindow);
    }
}
