#include "mainwindow.h"
#include "aboutdialog.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QHostAddress>
#include <QMessageBox>
#include <QModbusDevice>
#include <QModbusTcpServer>
#include <QNetworkInterface>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , dataUnitInputRegisters(QModbusDataUnit::InputRegisters, 0, 2) // Registros pressure,temp
    , dataUnitDiscreteInputs(QModbusDataUnit::DiscreteInputs, 0, 2) // Botones
    , dataUnitCoils(QModbusDataUnit::Coils, 0, 3)                   // Coils para los LEDs
{
    ui->setupUi(this);

    // Inicializar los LEDs
    ledRed = ui->ledRed;
    ledBlue = ui->ledBlue;
    ledGreen = ui->ledGreen;

    // Inicializar el segundo botón
    buttonDiscrete2 = ui->buttonDiscrete2;

    // Conectar señales y slots para el segundo botón
    connect(buttonDiscrete2, &QPushButton::pressed, this, &MainWindow::on_buttonDiscrete2_pressed);
    connect(buttonDiscrete2, &QPushButton::released, this, &MainWindow::on_buttonDiscrete2_released);

    // Declarar la dirección del dispositivo para usar en varios bloques
    const int deviceAddress = 1;

    { // 1. Configurar el árbol Modbus
        ui->treeWidgetModbus->clear();
        ui->treeWidgetModbus->setColumnCount(2);
        ui->treeWidgetModbus->setHeaderLabels(QStringList() << "Register" << "Value");

        // Configurar entradas discretas
        treeDiscreteInputItem = new QTreeWidgetItem(ui->treeWidgetModbus);
        treeDiscreteInputItem->setText(0, "Entradas Discretas");
        QTreeWidgetItem *discreteItem = new QTreeWidgetItem(treeDiscreteInputItem);
        discreteItem->setText(0, "0");
        discreteItem->setText(1, "0"); // Valor inicial
        QTreeWidgetItem *discreteItem1 = new QTreeWidgetItem(treeDiscreteInputItem);
        discreteItem1->setText(0, "1");
        discreteItem1->setText(1, "1"); // Valor inicial

        // Configurar registros de entrada
        treeInputRegistersItem = new QTreeWidgetItem(ui->treeWidgetModbus);
        treeInputRegistersItem->setText(0, "Registros de Entrada");
        // Se crean los dos items pero se dejan los valores en "0" hasta configurar los sliders
        QTreeWidgetItem *registerPressure = new QTreeWidgetItem(treeInputRegistersItem);
        registerPressure->setText(0, "0 (Presión)");
        registerPressure->setText(1, "0");
        QTreeWidgetItem *registerTemperature = new QTreeWidgetItem(treeInputRegistersItem);
        registerTemperature->setText(0, "1 (Temperatura)");
        registerTemperature->setText(1, "0");

        // Configurar coils para los LEDs
        QTreeWidgetItem *treeCoilsItem = new QTreeWidgetItem(ui->treeWidgetModbus);
        treeCoilsItem->setText(0, "Coils");
        QTreeWidgetItem *coilRed = new QTreeWidgetItem(treeCoilsItem);
        coilRed->setText(0, "0 (Rojo)");
        coilRed->setText(1, "0");
        QTreeWidgetItem *coilBlue = new QTreeWidgetItem(treeCoilsItem);
        coilBlue->setText(0, "1 (Azul)");
        coilBlue->setText(1, "0");
        QTreeWidgetItem *coilGreen = new QTreeWidgetItem(treeCoilsItem);
        coilGreen->setText(0, "2 (Verde)");
        coilGreen->setText(1, "0");

        ui->treeWidgetModbus->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->treeWidgetModbus->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        ui->treeWidgetModbus->expandAll();
    }

    { // 2. Inicializar el servidor Modbus
        modbusServer = new QModbusTcpServer(this);
        if (!modbusServer) {
            QMessageBox::critical(this, "Error", "No se pudo crear el servidor Modbus.");
            return;
        }

        // Configurar el mapeo de datos Modbus
        QMap<QModbusDataUnit::RegisterType, QModbusDataUnit> dataMap;
        dataMap.insert(QModbusDataUnit::InputRegisters, dataUnitInputRegisters);
        dataMap.insert(QModbusDataUnit::DiscreteInputs, dataUnitDiscreteInputs);
        dataMap.insert(QModbusDataUnit::Coils, dataUnitCoils);
        modbusServer->setMap(dataMap);

        modbusServer->setServerAddress(deviceAddress);
        qDebug() << "Server address: " << modbusServer->serverAddress();

        // Configurar parámetros y conectar el servidor en el puerto 1502
        modbusServer->setConnectionParameter(QModbusDevice::NetworkAddressParameter, "0.0.0.0");
        auto port = 1502;
        modbusServer->setConnectionParameter(QModbusDevice::NetworkPortParameter, port);
        if (!modbusServer->connectDevice()) {
            QMessageBox::critical(this,
                                  "Error",
                                  QString("No se pudo iniciar el servidor Modbus en el puerto %1.")
                                      .arg(port));
            qInfo() << "Cannot listen on port " << port;
            QApplication::exit(1);
        }
        qDebug() << "Servidor Modbus escuchando en puerto " << port;
    }

    { // 3. Inicializar sliders y campos de edición
        ui->sliderPressure->setMinimum(950);
        ui->sliderPressure->setMaximum(1050);
        ui->sliderPressure->setValue(1013);
        ui->editPressure->setText(QString::number(ui->sliderPressure->value()));

        ui->sliderTemperature->setMinimum(0);
        ui->sliderTemperature->setMaximum(200);
        ui->sliderTemperature->setValue(25);
        ui->editTemperature->setText(QString::number(ui->sliderTemperature->value()));

        // Actualizar el árbol con los valores iniciales
        treeInputRegistersItem->child(0)->setText(1, QString::number(ui->sliderPressure->value()));
        treeInputRegistersItem->child(1)->setText(1,
                                                  QString::number(ui->sliderTemperature->value()));
    }

    { // 4. Conectar señales y configurar la barra de estado
        connect(ui->buttonDiscrete,
                &QPushButton::pressed,
                this,
                &MainWindow::on_buttonDiscrete_pressed);
        connect(ui->buttonDiscrete,
                &QPushButton::released,
                this,
                &MainWindow::on_buttonDiscrete_released);

        // Conectar la señal de cambio de datos Modbus
        connect(modbusServer, &QModbusTcpServer::dataWritten, this, &MainWindow::onCoilsChanged);

        { // Mostrar información de red en la barra de estado
            QStringList ips;
            const QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
            for (const QHostAddress &address : addresses) {
                if (address.protocol() == QAbstractSocket::IPv4Protocol && !address.isLoopback())
                    ips << address.toString();
            }
            const int port = modbusServer->connectionParameter(QModbusDevice::NetworkPortParameter)
                                 .toInt();
            auto msg = QString("Port: %2 | Addr: %3 | IPs: %1")
                           .arg(ips.join(", "))
                           .arg(port)
                           .arg(deviceAddress);
            statusBar()->showMessage(msg);
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_sliderPressure_valueChanged(int value)
{
    ui->editPressure->setText(QString::number(value));

    dataUnitInputRegisters.setValue(0, ui->sliderPressure->value()); // Update pressure

    if (modbusServer->setData(dataUnitInputRegisters)) {
        QTreeWidgetItem *item = treeInputRegistersItem->child(0);
        if (item) {
            item->setText(1, QString::number(value));
        } else {
            qDebug() << "no item to update";
        }
    } else {
        // Handle error: Failed to set Modbus data
        qDebug() << "modbus error";
    }
}

void MainWindow::on_editPressure_editingFinished()
{
    bool ok;
    int value = ui->editPressure->text().toInt(&ok);
    if (ok && value >= ui->sliderPressure->minimum() && value <= ui->sliderPressure->maximum()) {
        ui->sliderPressure->setValue(value);
    } else {
        ui->editPressure->setText(QString::number(ui->sliderPressure->value()));
    }
}

void MainWindow::on_sliderTemperature_valueChanged(int value)
{
    ui->editTemperature->setText(QString::number(value));

    // Assuming pressure is at address 0 and temperature at address 1

    dataUnitInputRegisters.setValue(1, ui->sliderTemperature->value()); // Update temperature

    if (modbusServer->setData(dataUnitInputRegisters)) {
        // Update UI tree item for temperature (address 1)
        QTreeWidgetItem *item = treeInputRegistersItem->child(1);
        if (item) {
            item->setText(1, QString::number(value));
        }
    } else {
        // Handle error: Failed to set Modbus data
        qDebug() << "modbus error";
    }
}

void MainWindow::on_editTemperature_editingFinished()
{
    bool ok;
    int value = ui->editTemperature->text().toInt(&ok);
    if (ok && value >= ui->sliderTemperature->minimum()
        && value <= ui->sliderTemperature->maximum()) {
        ui->sliderTemperature->setValue(value);
    } else {
        ui->editTemperature->setText(QString::number(ui->sliderTemperature->value()));
    }
}

void MainWindow::on_buttonDiscrete_pressed()
{
    dataUnitDiscreteInputs.setValue(0, 1);

    QTreeWidgetItem *item = treeDiscreteInputItem->child(0);
    if (item) {
        item->setText(1, "1");
    }

    if (modbusServer->setData(dataUnitDiscreteInputs)) {
        //
    } else {
        // Handle error: Failed to set Modbus data
        qDebug() << "modbus error";
    }
}

void MainWindow::on_buttonDiscrete_released()
{
    dataUnitDiscreteInputs.setValue(0, 0);

    QTreeWidgetItem *item = treeDiscreteInputItem->child(0);
    if (item) {
        item->setText(1, "0");
    }
    if (modbusServer->setData(dataUnitDiscreteInputs)) {
        //
    } else {
        // Handle error: Failed to set Modbus data
        qDebug() << "modbus error";
    }
}

void MainWindow::on_action_About_triggered()
{
    (new AboutDialog(this))->show();
}

void MainWindow::on_action_Exit_triggered()
{
    QApplication::exit(0);
}

void MainWindow::onCoilsChanged()
{
    // Leer los valores de los coils uno por uno
    for (int i = 0; i < 3; ++i) {
        quint16 value;
        if (modbusServer->data(QModbusDataUnit::Coils, i, &value)) {
            dataUnitCoils.setValue(i, value); // Actualizar el valor en dataUnitCoils

            // Actualizar el valor en el árbol Modbus
            QTreeWidgetItem *coilItem = ui->treeWidgetModbus->topLevelItem(2)->child(
                i); // Coils están en el índice 2
            if (coilItem) {
                coilItem->setText(1, QString::number(value)); // Actualizar el valor en la columna 1
            } else {
                qDebug() << "No se encontró el ítem del coil" << i;
            }
        } else {
            qDebug() << "Error al leer el coil" << i;
        }
    }

    // Actualizar los LEDs
    updateLEDs();

    // Depuración: Imprimir los valores de los coils
    qDebug() << "Coils actualizados:"
             << "Rojo:" << dataUnitCoils.value(0) << "Azul:" << dataUnitCoils.value(1)
             << "Verde:" << dataUnitCoils.value(2);
}

void MainWindow::on_buttonDiscrete2_pressed()
{
    dataUnitDiscreteInputs.setValue(1, 1);

    QTreeWidgetItem *item = treeDiscreteInputItem->child(1);
    if (item) {
        item->setText(1, "1");
    }

    if (modbusServer->setData(dataUnitDiscreteInputs)) {
        //
    } else {
        qDebug() << "modbus error";
    }
}

void MainWindow::on_buttonDiscrete2_released()
{
    dataUnitDiscreteInputs.setValue(1, 0);

    QTreeWidgetItem *item = treeDiscreteInputItem->child(1);
    if (item) {
        item->setText(1, "0");
    }
    if (modbusServer->setData(dataUnitDiscreteInputs)) {
        //
    } else {
        qDebug() << "modbus error";
    }
}

void MainWindow::updateLEDs()
{
    // Obtener los valores de los coils
    bool redOn = dataUnitCoils.value(0);
    bool blueOn = dataUnitCoils.value(1);
    bool greenOn = dataUnitCoils.value(2);

    // Actualizar el color de los paneles
    ledRed->setStyleSheet(redOn ? "background-color: rgb(255, 0, 0);" : "background-color: black;");
    ledBlue->setStyleSheet(blueOn ? "background-color: rgb(0, 0, 255);"
                                  : "background-color: black;");
    ledGreen->setStyleSheet(greenOn ? "background-color: rgb(0, 255, 0);"
                                    : "background-color: black;");

    qDebug() << "LEDs actualizados:"
             << "Rojo:" << redOn << "Azul:" << blueOn << "Verde:" << greenOn;
}
