#include "mainwindow.h"
#include <QAbstractSocket>
#include <QColor>
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    modbusServer = new QModbusTcpServer(this);
    modbusServer->setConnectionParameter(QModbusDevice::NetworkAddressParameter, "0.0.0.0");
    modbusServer->setConnectionParameter(QModbusDevice::NetworkPortParameter, 1502);
    modbusServer->setServerAddress(0);

    QModbusDataUnitMap dataMap;
    dataMap.insert(QModbusDataUnit::Coils, QModbusDataUnit(QModbusDataUnit::Coils, 0, 3));
    dataMap.insert(QModbusDataUnit::DiscreteInputs,
                   QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 0, 2));
    dataMap.insert(QModbusDataUnit::InputRegisters,
                   QModbusDataUnit(QModbusDataUnit::InputRegisters, 0, 2));
    dataMap.insert(QModbusDataUnit::HoldingRegisters,
                   QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 0, 3));

    if (!modbusServer->setMap(dataMap)) {
        qDebug() << "Failed to set data map:" << modbusServer->errorString();
        exit(1);
    }

    QList<QFrame *> binaryLeds = {ui->binaryLedRed, ui->binaryLedGreen, ui->binaryLedBlue};
    QList<QFrame *> pwmLeds = {ui->pwmLedRed, ui->pwmLedGreen, ui->pwmLedBlue};
    const int binaryLedStartAddr = 0;
    const int pwmLedStartAddr = 0;

    QObject::connect(
        modbusServer,
        &QModbusTcpServer::dataWritten,
        [=](QModbusDataUnit::RegisterType table, int address, int size) {
            qDebug() << "Written type: " << table << " [" << address << "] sz==" << size;
            switch (table) {
            case QModbusDataUnit::InputRegisters:
                for (int reg = address; reg < address + size; ++reg) {
                    quint16 value;
                    if (modbusServer->data(table, reg, &value)) {
                        joystickWidgetItemAxis[reg]->setText(1, QString::number(value));
                    } else {
                        qFatal() << "Could not extract data";
                    }
                }
                break;
            case QModbusDataUnit::DiscreteInputs:
                for (int reg = address; reg < address + size; ++reg) {
                    quint16 value;
                    if (modbusServer->data(table, reg, &value)) {
                        discreteInputsWidgetItems[reg]->setText(1, QString::number(value));
                    } else {
                        qFatal() << "Could not extract data";
                    }
                }
                break;
            case QModbusDataUnit::Coils:
                qDebug() << "Coil written at address" << address << "size:" << size;
                for (int reg = address; reg < address + size; ++reg) {
                    if (reg < binaryLedStartAddr || reg >= binaryLedStartAddr + pwmLeds.size())
                        continue;

                    quint16 value;
                    uchar rgb[3]{};

                    for (int color = 0; color < 3; ++color) {
                        if (modbusServer->data(table, binaryLedStartAddr + color, &value)) {
                            coilsWidgetItems[color]->setText(1, QString::number(value));
                            rgb[color] = value == 1 ? 255 : 0;
                        } else {
                            qFatal() << "Could not extract data";
                        }
                    }

                    for (int color = 0; color < 3; ++color) {
                        auto style = QString("background-color: rgb(%1, %2, %3);")
                                         .arg(color == 0 ? rgb[0] : 0)
                                         .arg(color == 1 ? rgb[1] : 0)
                                         .arg(color == 2 ? rgb[2] : 0);
                        binaryLeds[color]->setStyleSheet(style);
                    }
                }
                break;

            case QModbusDataUnit::HoldingRegisters:
                qDebug() << "HoldingRegisters written at address" << address << "size:" << size;

                for (int reg = address; reg < address + size; ++reg) {
                    if (reg < pwmLedStartAddr || reg >= pwmLedStartAddr + pwmLeds.size())
                        continue;
                    quint16 rgb[3]{};

                    for (int color = 0; color < 3; ++color) {
                        if (!modbusServer->data(table, pwmLedStartAddr + color, &rgb[color])) {
                            qFatal() << "Could not extract data";
                        }

                        holdingRegistersWidgetItems[color]->setText(1, QString::number(rgb[color]));
                    }

                    for (int color = 0; color < 3; ++color) {
                        auto style = QString("background-color: rgb(%1, %2, %3);")
                                         .arg(color == 0 ? rgb[0] : 0)
                                         .arg(color == 1 ? rgb[1] : 0)
                                         .arg(color == 2 ? rgb[2] : 0);
                        pwmLeds[color]->setStyleSheet(style);
                    }
                }
                break;
            default:
                qDebug() << "Ignoring unknown write: " << table;
                break;
            }
        });

    if (!modbusServer->connectDevice()) {
        qDebug() << "Modbus server could not start:" << modbusServer->errorString();
        exit(1);
    }

    qDebug() << "Modbus TCP slave running";

    joystickView = new GraphicsView(ui->joystickFrame_2);
    joystickView->setRenderHint(QPainter::Antialiasing);

    joystickView->setMinimumSize(250, 250);
    joystickView->resize(240, 240);
    joystickView->setSceneRect(-120, -120, 220, 220);
    joystickView->setAlignment(Qt::AlignCenter);

    QObject::connect(joystickView->getPointer(), &MovableCircle::positionChanged, [&](int x, int y) {
        modbusServer->setData(QModbusDataUnit::InputRegisters, 0, x);
        modbusServer->setData(QModbusDataUnit::InputRegisters, 1, y);
        joystickWidgetItemAxis[0]->setText(1, QString::number(x));
        joystickWidgetItemAxis[1]->setText(1, QString::number(y));
    });

    QList<QPushButton *> buttons = {ui->pushButton0, ui->pushButton1};

    for (int i = 0; i < buttons.size(); ++i) {
        QPushButton *btn = buttons[i];
        int coilIndex = i;
        QObject::connect(btn, &QPushButton::pressed, [=]() {
            modbusServer->setData(QModbusDataUnit::DiscreteInputs, coilIndex, 1);
        });
        QObject::connect(btn, &QPushButton::released, [=]() {
            modbusServer->setData(QModbusDataUnit::DiscreteInputs, coilIndex, 0);
        });
    }

    ui->treeWidget->setHeaderLabels(QStringList()
                                    << "Modbus Register Id" << "Value" << "Description");

    QTreeWidgetItem *discreteInputsWidgetItem = new QTreeWidgetItem();
    discreteInputsWidgetItem->setText(0, "DiscreteInputs");

    QTreeWidgetItem *coilsWidgetItem = new QTreeWidgetItem();
    coilsWidgetItem->setText(0, "Coils");
    QTreeWidgetItem *inputRegistersWidgetItem = new QTreeWidgetItem();
    inputRegistersWidgetItem->setText(0, "InputRegisters");

    joystickWidgetItem = new QTreeWidgetItem();
    joystickWidgetItem->setText(0, "Joystick");
    inputRegistersWidgetItem->addChild(joystickWidgetItem);

    QStringList axis = {"X", "Y"};
    for (int i = 0; i < 2; ++i) {
        joystickWidgetItemAxis[i] = new QTreeWidgetItem();
        joystickWidgetItemAxis[i]->setText(0, QString::number(i));
        joystickWidgetItemAxis[i]->setText(1, "-");
        joystickWidgetItemAxis[i]->setText(2, "Joystick " + axis[i]);
        joystickWidgetItem->addChild(joystickWidgetItemAxis[i]);
    }

    QTreeWidgetItem *holdingRegistersWidgetItem = new QTreeWidgetItem();
    holdingRegistersWidgetItem->setText(0, "HoldingRegisters");

    QStringList colorNames = {"Red", "Green", "Blue"};
    QTreeWidgetItem *pwmLedsWidgetItem = new QTreeWidgetItem();
    pwmLedsWidgetItem->setText(0, "PWM leds");
    holdingRegistersWidgetItem->addChild(pwmLedsWidgetItem);

    for (int i = 0; i < colorNames.size(); ++i) {
        holdingRegistersWidgetItems[i] = new QTreeWidgetItem();
        holdingRegistersWidgetItems[i]->setText(0, QString::number(i));
        holdingRegistersWidgetItems[i]->setText(1, "-");
        holdingRegistersWidgetItems[i]->setText(2, "PWM led " + colorNames[i]);
        pwmLedsWidgetItem->addChild(holdingRegistersWidgetItems[i]);
    }

    for (int i = 0; i < 2; ++i) {
        discreteInputsWidgetItems[i] = new QTreeWidgetItem();
        discreteInputsWidgetItems[i]->setText(0, QString::number(i));
        discreteInputsWidgetItems[i]->setText(1, "-");
        discreteInputsWidgetItems[i]->setText(2, "Button " + QString::number(i));
        discreteInputsWidgetItem->addChild(discreteInputsWidgetItems[i]);
    }

    for (int i = 0; i < 3; ++i) {
        coilsWidgetItems[i] = new QTreeWidgetItem();
        coilsWidgetItems[i]->setText(0, QString::number(i));
        coilsWidgetItems[i]->setText(1, "-");
        coilsWidgetItems[i]->setText(2, colorNames[i]);
        coilsWidgetItem->addChild(coilsWidgetItems[i]);
    }

    ui->treeWidget->addTopLevelItem(discreteInputsWidgetItem);
    ui->treeWidget->addTopLevelItem(coilsWidgetItem);
    ui->treeWidget->addTopLevelItem(inputRegistersWidgetItem);
    ui->treeWidget->addTopLevelItem(holdingRegistersWidgetItem);

    ui->treeWidget->expandAll();
    ui->treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    for (int i = 0; i < 2; i++) {
        modbusServer->setData(QModbusDataUnit::InputRegisters, i, 8191);
    }
    for (int i = 0; i < 2; i++) {
        modbusServer->setData(QModbusDataUnit::DiscreteInputs, i, 1);
        modbusServer->setData(QModbusDataUnit::DiscreteInputs, i, 0);
    }
    for (int i = 0; i < 3; i++) {
        modbusServer->setData(QModbusDataUnit::Coils, i, 255);
        modbusServer->setData(QModbusDataUnit::HoldingRegisters, i, 255);
    }

    ui->statusbar->showMessage("Not connected");
}

MainWindow::~MainWindow()
{
    delete ui;
}
