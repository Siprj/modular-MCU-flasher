#include "dmxflasher.h"
#include "ui_dmxFlasher.h"

#include "globalStatic.h"

#include <QWidget>
#include <QPushButton>
#include <QGridLayout>
#include <QThread>
#include <QGroupBox>
#include <QTimer>
#include <QDebug>
#include <QMessageBox>

#include <QtSerialPort/QSerialPortInfo>

#include "widgetwithnotifi.h"
#include "DmxBootProtocol.h"

DmxFlasher::DmxFlasher():
    ui(new Ui::DmxFlasher)
{
    dmxFlasherWidget = new WidgetWithNotifi;
    ui->setupUi(dmxFlasherWidget);
    connect(dmxFlasherWidget, SIGNAL(widgetVisibilityChanged(bool)), this, SLOT(showWidgetVisiblityChanget(bool)), Qt::QueuedConnection);

    thread = new QThread();
    thread->start();

    serialListRefreshTimer = new QTimer(this);  // will be automatically deleted
    serialListRefreshTimer->setInterval(1000);
    connect(serialListRefreshTimer, SIGNAL(timeout()), this, SLOT(refreshSerials()), Qt::QueuedConnection);
    connect(ui->serialPortComboBox, SIGNAL(popupExpanded(bool)), this, SLOT(serialComboboxExpanded(bool)), Qt::QueuedConnection);
    connect(ui->serialPortComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(showPortInfo(qint32)), Qt::QueuedConnection);


    dmxBootProtocol = new DmxBootProtocol(this);
    connect(dmxBootProtocol, SIGNAL(done(bool)), this, SLOT(DmxBootProtocolDoneSlot(bool)), Qt::QueuedConnection);
    connect(dmxBootProtocol, SIGNAL(printProgressInfo(QString)), this, SLOT(DmxBootPrintProgressInfoSlot(QString)), Qt::QueuedConnection);
    connect(dmxBootProtocol, SIGNAL(progressInPercentage(qint32)), this, SLOT(DmxBootProgressInPercentageSlot(qint32)), Qt::QueuedConnection);
    connect(this, SIGNAL(startFlashing(QString,unsigned char,QByteArray)), dmxBootProtocol, SLOT(startBootSequence(QString,unsigned char,QByteArray)), Qt::QueuedConnection);

    moveToThread(thread);
}

DmxFlasher::~DmxFlasher()
{
    delete ui;
    delete thread;
}

QWidget *DmxFlasher::getPluginWidget()
{
    return dmxFlasherWidget;
}

void DmxFlasher::refreshSerials()
{
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
    }
}

void DmxFlasher::showWidgetVisiblityChanget(bool isVisible)
{
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

void DmxFlasher::serialComboboxExpanded(bool isExpanded)
{
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

void DmxFlasher::showPortInfo(qint32 index)
{
    if (index != -1) {
        QStringList list = ui->serialPortComboBox->itemData(index).toStringList();
        ui->descriptionLabel->setText(tr("Description: %1").arg(list.at(1)));
        ui->manufacturerLabel->setText(tr("Manufacturer: %1").arg(list.at(2)));
        ui->locationLabel->setText(tr("Location: %1").arg(list.at(3)));
        ui->vendorIDLabel->setText(tr("Vendor ID: %1").arg(list.at(4)));
        ui->productIDlabel->setText(tr("Product ID: %1").arg(list.at(5)));
    }
}

void DmxFlasher::DmxBootProtocolDoneSlot(bool success)
{
    qDebug()<<"Flashin ended, successful?: "<<success;
    emit done(success);
}

void DmxFlasher::DmxBootPrintProgressInfoSlot(QString progress)
{
    qDebug()<<"Progress info: "<<progress;
    emit printProgressInfo(progress);
}

void DmxFlasher::DmxBootProgressInPercentageSlot(qint32 progress)
{
    qDebug()<<"Progress in percentage: "<<progress<<"%";
    emit progressInPercentage(progress);
}

void DmxFlasher::flash(QByteArray data)
{
    emit startFlashing(ui->serialPortComboBox->currentText(), \
                       ui->addressLineEdit->text().toInt(0, 16), data);
}
