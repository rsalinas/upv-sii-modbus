#ifndef TIMESERIESVIEWER_H
#define TIMESERIESVIEWER_H

#include <QCheckBox>
#include <QDateTime>
#include <QPointer> // For safe pointer tracking
#include <QTimer>
#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include "modbusreader.h" // Ensure this header is available

class TimeSeriesViewer : public QWidget
{
    Q_OBJECT
public:
    explicit TimeSeriesViewer(QWidget *parent = nullptr);

    // Methods to manage the Modbus connection
    bool connectModbus(const QString &host, int port, int deviceAddress, int pollInterval);
    void disconnectModbus();
    bool isConnected() const { return modbusReader; }

public slots:
    // Update the poll interval in real-time
    void updatePollInterval(int interval);
    // Update the time window (in seconds) and reset the axis range accordingly
    void updateTimeWindow(int seconds);
    // Clear the data and reset the chart
    void clearData();

signals:
    // Emitted when the connection status changes, to update the corresponding control
    void connectionStatusChanged(const QString &status);
    // Emitted when the Modbus request count is updated
    void modbusRequestCountChanged(int count);
    // Emitted when the last request time (in ms) is updated
    void lastRequestTimeUpdated(qint64 lastTime);
    // Emitted when the average request time (in ms) is updated
    void averageRequestTimeUpdated(double averageTime);

private slots:
    // Slot to process the asynchronous read for input registers
    void processInputRegisters(const QVector<quint16> &registers);
    // Slot to process the asynchronous read for discrete coils
    void processDiscreteCoils(const QBitArray &coils);
    // Slot to handle errors in Modbus communication
    void handleModbusError(const QString &error);

private:
    double time;
    QChart *chart;
    QLineSeries *pressureSeries;
    QLineSeries *temperatureSeries;
    QTimer *timer;
    QPointer<ModbusReader> modbusReader; // Changed from raw pointer to QPointer

    // New members for tracking requests and timings
    int modbusRequestCount;
    qint64 lastRequestTimestamp; // Timestamp of the last request in ms
    int requestTimingCount;
    double requestTimingSum; // Sum of request times in ms

    // Store the connection for the timer so we can disconnect it
    QMetaObject::Connection m_timerConnection;

    // New member: rolling time window (in seconds)
    int timeWindow;

    // New widget to show the discrete input state
    QCheckBox *checkBoxDiscreteInput;
};

#endif // TIMESERIESVIEWER_H
