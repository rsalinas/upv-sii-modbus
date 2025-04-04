#include <QApplication>
#include <QCheckBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include "modbusreader.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Conectar con Modbus
    ModbusReader reader("localhost", 1502, 1);
    if (!reader.connectDevice()) {
        qWarning() << "Error al conectar con el dispositivo:" << reader.errorString();
        return -1;
    } else {
        qDebug() << "Conexión Modbus establecida correctamente";
    }

    // Series
    QLineSeries *seriePresion = new QLineSeries();
    seriePresion->setName("Presión (hPa)");

    QLineSeries *serieTemperatura = new QLineSeries();
    serieTemperatura->setName("Temperatura (°C)");

    // Chart
    QChart *chart = new QChart();
    chart->addSeries(seriePresion);
    chart->addSeries(serieTemperatura);
    chart->setTitle("Presión y Temperatura en tiempo real");

    // Eje X (tiempo)
    QValueAxis *axisX = new QValueAxis;
    axisX->setTitleText("Tiempo (s)");
    axisX->setLabelFormat("%.1f");
    axisX->setRange(0, 10);

    // Eje Y para la presión
    QValueAxis *axisYPresion = new QValueAxis;
    axisYPresion->setTitleText("Presión (hPa)");
    axisYPresion->setRange(950, 1050);

    // Eje Y para la temperatura
    QValueAxis *axisYTemperatura = new QValueAxis;
    axisYTemperatura->setTitleText("Temperatura (°C)");
    axisYTemperatura->setRange(0, 200);

    // Añadimos los ejes al gráfico
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisYPresion, Qt::AlignLeft);
    chart->addAxis(axisYTemperatura, Qt::AlignRight);

    // Conectamos las series a sus ejes
    seriePresion->attachAxis(axisX);
    seriePresion->attachAxis(axisYPresion);

    serieTemperatura->attachAxis(axisX);
    serieTemperatura->attachAxis(axisYTemperatura);

    // ChartView
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // Crear widget principal y layout
    QWidget *mainWidget = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->addWidget(chartView);

    // Línea inferior con 2 CheckBox
    QWidget *checkBoxWidget = new QWidget;
    QHBoxLayout *checkBoxLayout = new QHBoxLayout(checkBoxWidget);
    QCheckBox *checkBoxPresion = new QCheckBox("Dibujar línea de Presión");
    QCheckBox *checkBoxTemperatura = new QCheckBox("Dibujar línea de Temperatura");

    // Ambos activados inicialmente
    checkBoxPresion->setChecked(true);
    checkBoxTemperatura->setChecked(true);
    checkBoxLayout->addWidget(checkBoxPresion);
    checkBoxLayout->addWidget(checkBoxTemperatura);
    checkBoxLayout->addStretch();

    mainLayout->addWidget(checkBoxWidget);

    // Establecer título de la ventana
    mainWidget->setWindowTitle("SII - Práctica 2");
    mainWidget->resize(800, 600);
    mainWidget->show();

    // Conectar checkboxes para controlar la visibilidad de las series
    QObject::connect(checkBoxPresion, &QCheckBox::toggled, [&](bool checked) {
        // Evitar que ambas se desactiven
        if (!checked && !checkBoxTemperatura->isChecked()) {
            checkBoxPresion->blockSignals(true);
            checkBoxPresion->setChecked(true);
            checkBoxPresion->blockSignals(false);
            return;
        }
        seriePresion->setVisible(checked);
    });

    QObject::connect(checkBoxTemperatura, &QCheckBox::toggled, [&](bool checked) {
        if (!checked && !checkBoxPresion->isChecked()) {
            checkBoxTemperatura->blockSignals(true);
            checkBoxTemperatura->setChecked(true);
            checkBoxTemperatura->blockSignals(false);
            return;
        }
        serieTemperatura->setVisible(checked);
    });

    // Timer
    QTimer timer;
    static qreal x = 0.0;

    QObject::connect(&timer, &QTimer::timeout, [&]() {
        // Lectura de registros (2 valores: presión y temperatura)
        QVector<quint16> regs = reader.getInputRegisters(2, 0);
        if (regs.size() >= 2) {
            qreal presion = regs[0];     // ~1013
            qreal temperatura = regs[1]; // ~25

            x += 0.1; // cada 100 ms
            seriePresion->append(x, presion);
            serieTemperatura->append(x, temperatura);

            // Desplazar eje X para ver últimos 10 s
            if (x > 10) {
                axisX->setRange(x - 10, x);
            }
        }
    });

    timer.start(100); // cada 100 ms

    return app.exec();
}
