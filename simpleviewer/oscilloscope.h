#ifndef OSCILLOSCOPE_H
#define OSCILLOSCOPE_H

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QTimer>

//QT_CHARTS_USE_NAMESPACE

class Oscilloscope : public QWidget {
    Q_OBJECT

public:
    explicit Oscilloscope(QWidget *parent = nullptr);

private slots:
    void updateData();

private:
    QLineSeries *series; // Serie de datos
    QChart *chart;       // Gráfico
    QTimer *timer;       // Temporizador para actualizar
    double time;         // Tiempo simulado
};

#endif // OSCILLOSCOPE_H

