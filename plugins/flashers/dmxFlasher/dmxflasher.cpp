#include "dmxflasher.h"

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

#include "../../../app/interfaces/trace.h"

DmxFlasher::DmxFlasher()
{
    FUNCTION_ENTER_TRACE;

    qRegisterMetaType<DmxBootProtocol::Errors>("DmxBootProtocol::Errors");

    dmxFlasherWidget = new DmxFlasherWidget;


    thread = new QThread();
    thread->start();

    dmxBootProtocol = new DmxBootProtocol(this);
    connect(dmxBootProtocol, SIGNAL(done(bool)), this, SLOT(DmxBootProtocolDoneSlot(bool)), Qt::QueuedConnection);
    connect(dmxBootProtocol, SIGNAL(printProgressInfo(QString)), this, SLOT(DmxBootPrintProgressInfoSlot(QString)), Qt::QueuedConnection);
    connect(dmxBootProtocol, SIGNAL(progressInPercentage(qint32)), this, SLOT(DmxBootProgressInPercentageSlot(qint32)), Qt::QueuedConnection);
    connect(dmxBootProtocol, SIGNAL(errorOcured(DmxBootProtocol::Errors)), this, SLOT(DmxBootProtocolErrorOcured(DmxBootProtocol::Errors)), Qt::QueuedConnection);
    connect(this, SIGNAL(startFlashing(QString,unsigned char,QByteArray)), dmxBootProtocol, SLOT(startBootSequence(QString,unsigned char,QByteArray)), Qt::QueuedConnection);

    moveToThread(thread);
}

DmxFlasher::~DmxFlasher()
{
    FUNCTION_ENTER_TRACE;
    delete thread;
}

void DmxFlasher::putSettings(QSettings *settings)
{

}

QWidget *DmxFlasher::getPluginWidget()
{
    FUNCTION_ENTER_TRACE;
    return dmxFlasherWidget;
}

void DmxFlasher::DmxBootProtocolDoneSlot(bool success)
{
    FUNCTION_ENTER_TRACE;
    qDebug()<<"Flashin ended, successful?: "<<success;
    emit done(success);
}

void DmxFlasher::DmxBootPrintProgressInfoSlot(QString progress)
{
    FUNCTION_ENTER_TRACE;
    qDebug()<<"Progress info: "<<progress;
    emit printProgressInfo(progress);
}

void DmxFlasher::DmxBootProgressInPercentageSlot(qint32 progress)
{
    FUNCTION_ENTER_TRACE;
    qDebug()<<"Progress in percentage: "<<progress<<"%";
    emit progressInPercentage(progress);
}

void DmxFlasher::DmxBootProtocolErrorOcured(DmxBootProtocol::Errors error)
{
    FUNCTION_ENTER_TRACE;
    switch(error)
    {
    case DmxBootProtocol::CantOpenPort:
        emit showMessageBox(errorMessageCantOpen, errorMessageCantOpen, MESSAGE_BOX_CRITICIAL);
        break;
    case DmxBootProtocol::CantWrite:
        emit showMessageBox(errorMessageCantWrite, errorMessageCantWrite, MESSAGE_BOX_CRITICIAL);
        break;
    case DmxBootProtocol::ReadTimeout:
        emit showMessageBox(errorMessageTimeout, errorMessageTimeout, MESSAGE_BOX_CRITICIAL);
        break;
    default:        // unkown error ocured
        emit showMessageBox(errorMessageUnkown, errorMessageUnkown, MESSAGE_BOX_CRITICIAL);
        break;
    }
}

void DmxFlasher::flash(QByteArray data)
{
    FUNCTION_ENTER_TRACE;
    emit startFlashing(dmxFlasherWidget->getSerialPortName(), \
                       dmxFlasherWidget->getDeviceAddress(), data);
}
