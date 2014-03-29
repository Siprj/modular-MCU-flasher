#include "widgetwithnotifi.h"
#include "ui_dmxFlasherWidget.h"

#include <QTimer>
#include <QSerialPortInfo>
#include <QMutex>
#include <QMutexLocker>

#include "../../../app/interfaces/trace.h"
#include <QDebug>
#include <QSettings>

// Device address is storen as unsigned int
#define SETTINGS_LAST_DEVICE_ADDRESS "plugin/flasher/dmxFlasher/deviceAddress"
#define SETTINGS_LAST_SERIAL_PORT    "plugin/flasher/dmxFlasher/serialPort"


DmxFlasherWidget::DmxFlasherWidget(QWidget *parent) :
    QWidget(parent), ui(new Ui::DmxFlasherWidget)
{
    FUNCTION_ENTER_TRACE;
    ui->setupUi(this);

    serialListRefreshTimer = new QTimer(this);  // will be automatically deleted
    serialListRefreshTimer->setInterval(1000);

    connect(serialListRefreshTimer, SIGNAL(timeout()), this, SLOT(refreshSerials()));
    connect(ui->serialPortComboBox, SIGNAL(popupExpanded(bool)), this, SLOT(serialComboboxExpanded(bool)));
    connect(ui->serialPortComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(publicPortInfo(qint32)));
    connect(ui->addressLineEdit, SIGNAL(textChanged(QString)), this, SLOT(publicDeviceAddress(QString)));
    connect(this, SIGNAL(widgetVisibilityChanged(bool)), this, SLOT(showWidgetVisiblityChanget(bool)));

    mutex = new QMutex;
    selectedSerialPortName = new QString;
}

DmxFlasherWidget::~DmxFlasherWidget()
{
    delete mutex;
    delete selectedSerialPortName;
}

QString DmxFlasherWidget::getSerialPortName()
{
    QMutexLocker locker(mutex);
    Q_UNUSED(locker);
    qDebug()<<"Returnet port name: "<<*selectedSerialPortName;
    return *selectedSerialPortName;
}

unsigned char DmxFlasherWidget::getDeviceAddress()
{
    QMutexLocker locker(mutex);
    Q_UNUSED(locker);
    qDebug()<<"Returnet device address: "<<deviceAddress;
    return deviceAddress;
}

void DmxFlasherWidget::setSettings(QSettings *settings) // this is called from GUI thread only
{
    this->settings = settings;
    deviceAddress = (unsigned char)settings->value(SETTINGS_LAST_DEVICE_ADDRESS, 0).toUInt();
    ui->addressLineEdit->setText(QString("%1").arg((quint32)deviceAddress, 0, 16));

    *selectedSerialPortName = settings->value(SETTINGS_LAST_SERIAL_PORT, "").toString();

    refreshSerials();
    ui->serialPortComboBox->setCurrentText(*selectedSerialPortName);
}

void DmxFlasherWidget::showEvent(QShowEvent *event)
{
    FUNCTION_ENTER_TRACE;
    emit widgetVisibilityChanged(true);
    QWidget::showEvent(event);
}

void DmxFlasherWidget::hideEvent(QHideEvent *event)
{
    FUNCTION_ENTER_TRACE;
    emit widgetVisibilityChanged(false);
    QWidget::hideEvent(event);
}

void DmxFlasherWidget::refreshSerials()
{
    FUNCTION_ENTER_TRACE;
    QString tmpSerialPortName;
    qDebug()<<"Refreshing serial port list";
    if(ui->serialPortComboBox->currentIndex() != -1)
    {
        tmpSerialPortName = ui->serialPortComboBox->currentText();
    }
    ui->serialPortComboBox->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QStringList list;
        list << info.portName()
             << info.description()
             << info.manufacturer()
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : QString())
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : QString());

        ui->serialPortComboBox->addItem(list.first(), list);
    }

    qint32 serialComboBoxIndex = ui->serialPortComboBox->findText(tmpSerialPortName);
    if(serialComboBoxIndex != -1)
    {
        ui->serialPortComboBox->setCurrentIndex(serialComboBoxIndex);

        QMutexLocker locker(mutex);
        Q_UNUSED(locker);

        *selectedSerialPortName = ui->serialPortComboBox->currentText();
        qDebug()<<"Save serial port name: "<<*selectedSerialPortName;
        settings->setValue(SETTINGS_LAST_SERIAL_PORT, *selectedSerialPortName);
    }
}

void DmxFlasherWidget::publicPortInfo(qint32 index)
{
    FUNCTION_ENTER_TRACE;
    if (index != -1) {
        QStringList list = ui->serialPortComboBox->itemData(index).toStringList();
        ui->descriptionLabel->setText(tr("Description: %1").arg(list.at(1)));
        ui->manufacturerLabel->setText(tr("Manufacturer: %1").arg(list.at(2)));
        ui->locationLabel->setText(tr("Location: %1").arg(list.at(3)));
        ui->vendorIDLabel->setText(tr("Vendor ID: %1").arg(list.at(4)));
        ui->productIDlabel->setText(tr("Product ID: %1").arg(list.at(5)));
    }
    else
    {
        *selectedSerialPortName = "";      // can operate with serial port name
    }
}

void DmxFlasherWidget::publicDeviceAddress(QString)
{
    QMutexLocker locker(mutex);
    Q_UNUSED(locker);
    deviceAddress = ui->addressLineEdit->text().toInt(0, 16);
    settings->setValue(SETTINGS_LAST_DEVICE_ADDRESS, (unsigned int) deviceAddress);
}

void DmxFlasherWidget::showWidgetVisiblityChanget(bool isVisible)
{
    FUNCTION_ENTER_TRACE;
    if(isVisible)
    {
        qDebug()<<"Starting refresh timer";
        serialListRefreshTimer->start();
        refreshSerials();
    }
    else
    {
        qDebug()<<"Stoping refresh timer";
        serialListRefreshTimer->stop();
    }
}

void DmxFlasherWidget::serialComboboxExpanded(bool isExpanded)
{
    FUNCTION_ENTER_TRACE;
    qDebug()<<"ComboBox expanded: "<<isExpanded;
    if(isExpanded)
    {
        qDebug()<<"Stoping refresh timer";
        serialListRefreshTimer->stop();
    }
    else
    {
        qDebug()<<"Starting refresh timer";
        serialListRefreshTimer->start();
    }
}
