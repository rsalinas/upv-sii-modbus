#include "oscilloscope.h"
#include <QVBoxLayout>
#include <QtCharts/QValueAxis>
#include <QtMath>

Oscilloscope::Oscilloscope(QWidget *parent)
    : QWidget(parent)
    , time(0)
{
    series = new QLineSeries();

    // Configurar el gráfico
    chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Osciloscopio");

    // Crear y configurar los ejes
    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Tiempo (s)");
    axisX->setRange(0, 10);        // Rango inicial
    axisX->setLabelFormat("%.1f"); // Formato de etiquetas
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Valor");
    axisY->setRange(-1, 1); // Rango inicial
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // Crear el ChartView para mostrar el gráfico
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // Configurar el layout
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(chartView);

    // Crear el temporizador
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Oscilloscope::updateData);
    timer->start(50); // Actualizar cada 50 ms
}

void Oscilloscope::updateData()
{
    // Generar un valor simulado (ejemplo: señal sinusoidal)
    double sensorValue = qSin(time);
    time += 0.05;

    series->append(time, sensorValue);

    // Eliminar puntos antiguos para mantener solo 10s de datos
    while (!series->points().isEmpty() && series->points().first().x() < time - 10) {
        series->remove(0);
    }

    // Actualizar los ejes dinámicamente si es necesario
    static_cast<QValueAxis *>(chart->axes(Qt::Horizontal).first())->setRange(time - 10, time);
}
