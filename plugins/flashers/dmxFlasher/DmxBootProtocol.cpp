#include "DmxBootProtocol.h"

#include <QStringList>

#include <QTimer>
#include <QMessageBox>
#include <QState>
#include <QStateMachine>
#include <QFinalState>
#include <QtEndian>

#include <QDebug>

#include "../../../app/interfaces/trace.h"

#define SERIAL_BAUD_RATE 250000
#define SERIAL_DATA_SIZE QSerialPort::Data8
#define SERIAL_PARITY QSerialPort::NoParity
#define SERIAL_STOPBITS QSerialPort::TwoStop
#define SERIAL_FLOW_CONTROL QSerialPort::NoFlowControl

#define WRITE_TIME_OUT 2000 //in ms
#define READ_TIME_OUT 2000 //in ms

DmxBootProtocol::DmxBootProtocol(QObject *parent) :
    QObject(parent)
{
    FUNCTION_ENTER_TRACE;
    qDebug("DMX Boot Protocol Constructor");
    this->portName = portName;
    serialPort = new QSerialPort(this);

    sendTimeoutTimer = new QTimer(this);
    sendTimeoutTimer->setSingleShot(true);
    sendTimeoutTimer->setInterval(WRITE_TIME_OUT);

    receiveTimeoutTimer = new QTimer(this);
    receiveTimeoutTimer->setSingleShot(true);
    receiveTimeoutTimer->setInterval(READ_TIME_OUT);
    connect(receiveTimeoutTimer, SIGNAL(timeout()), this, SLOT(receiveTimeoutTimerTimeout()), Qt::QueuedConnection);

    dataSizeToWrite = 0;
    dataSizeWriten = 0;
    deviceAddress = 0;
    lastSendedSeqNumber = 0;

    readState = ReadAddres;
    dataSizeReaded = 0;
    readPacketType = 0;
    readPacketSeq = 0;

    currentPageNumber = 0;

    sendErrorFlag = 0;
    sendErrorSeq  = 0;

    createStateMachine();

    connect(serialPort, SIGNAL(readyRead()), this, SLOT(dataReceived()), Qt::QueuedConnection);
    connect(serialPort, SIGNAL(bytesWritten(qint64)), this, SLOT(checkSendedDataSize(qint64)), Qt::QueuedConnection);
}

DmxBootProtocol::~DmxBootProtocol()
{
    FUNCTION_ENTER_TRACE;
    qDebug("DMX Boot Protocol Destructor");
    delete serialPort;
    delete stateMachine;
}

void DmxBootProtocol::startBootSequence(QString portName, unsigned char deviceAddress, QByteArray dataToSend)
{
    FUNCTION_ENTER_TRACE;
    qDebug()<<"Starting flasing device";
    this->dataToSend = dataToSend;
    this->deviceAddress = deviceAddress;
    this->portName = portName;

    if(!openPort())
    {
        qDebug()<<"canot open port: "<<serialPort->errorString();
        emit errorOcured(CantOpenPort);
        return;
    }

    stateMachine->start();
}

void DmxBootProtocol::startReceiveTimeoutTimer()
{
    FUNCTION_ENTER_TRACE;
    qDebug()<<"Starting receive timeout Timer";
    receiveTimeoutTimer->start();
}

int DmxBootProtocol::openPort()
{
    FUNCTION_ENTER_TRACE;
    emit printProgressInfo("Opening serial port");
    qDebug()<<"Opening serial port";
    serialPort->setPortName(portName);
    if (serialPort->open(QIODevice::ReadWrite))
    {
        if (serialPort->setBaudRate(SERIAL_BAUD_RATE)
                    && serialPort->setDataBits(SERIAL_DATA_SIZE)
                    && serialPort->setParity(SERIAL_PARITY)
                    && serialPort->setStopBits(SERIAL_STOPBITS)
                    && serialPort->setFlowControl(SERIAL_FLOW_CONTROL))
        {
        }
        else
        {
            qCritical()<<"Serial port canot be set to needed settings";
            serialPort->close();
            emit printProgressInfo(serialPort->errorString());

            return 0;
        }
    }
    else
    {
        qDebug()<<"Serial port can't be opened, prabebly wring port name";
        return 0;
    }


    return 1;
}

char DmxBootProtocol::calculateCRC(QByteArray array, qint32 size)
{
    FUNCTION_ENTER_TRACE;
    qDebug()<<"Calcluate CRC from data: "<<array.toHex()<<" size of array: "<<size;
    qDebug()<<"True size of array: "<<array.size();
    char crc = 0;

    for(int i = 0; i < size; i++)
    {
        crc += array[i];
    }
    //crc = (char)(0xFF - crc) + 1;
    crc = (char)(~crc) + 1;

    qDebug()<<"Compiuted CRC is: "<<QString("%1").arg((int)crc, 0, 16);
    return crc;
}

void DmxBootProtocol::sendData(QByteArray array)
{
    FUNCTION_ENTER_TRACE;
    dataSizeWriten = 0;
    dataSizeToWrite = array.size();
    sendTimeoutTimer->start();        // will be stoped

    if(sendErrorFlag)   // flag is se when we sending error respond
        --sendErrorFlag;        // value 2 means we are sending error mesage right now
                                // value 1 means we sended error message in previous cycle
    qDebug()<<"Sending data: "<<array.toHex();

    serialPort->write(array);
}

qint32 DmxBootProtocol::progress()
{
    FUNCTION_ENTER_TRACE;
    quint32 dataIndex = currentPageNumber*pageSize;
    quint32 maxDataIndex = qMin(dataIndex+pageSize, (quint32)dataToSend.size());
    return (100*((qreal)(maxDataIndex/dataToSend.size())));
}

void DmxBootProtocol::sendCRCError()
{
    FUNCTION_ENTER_TRACE;
    qDebug()<<"Sending RCR error";
    sendErrorFlag = 2;
    QByteArray array;
    array.resize(4);
    array[0] = deviceAddress;
    sendErrorSeq = (lastSendedSeqNumber + 2) & 0x0f;     // seqention number is 4 bits value so we have to cut it in to 4 bits.
    array[1] = sendErrorSeq<<4 | CRCErrorPacket;
    array[2] = lastSendedSeqNumber+1;
    array[3] = calculateCRC(array, 3);

    sendData(array);
}

void DmxBootProtocol::dataReceived()
{
    FUNCTION_ENTER_TRACE;
    qDebug()<<"Some data received";
    char byte = 0;
    while(serialPort->bytesAvailable())
    {
        if(serialPort->read(&byte, 1) == 1)
        {
            switch(readState)
            {
            case ReadAddres:
                readedData.clear();
                qDebug()<<"Reading addres";
                readState = ReadTypeAndSeq;
                break;
            case ReadTypeAndSeq:
                readPacketType = byte & 0x0F;
                readPacketSeq = ((byte & 0xF0) >> 4);
                switch (readPacketType)
                {
                case BootModeEnteredRespontPacket:
                    toBeReceived = 3;
                    break;
                case ACKPacket:
                    toBeReceived = 1;
                    break;
                case CRCErrorPacket:
                    toBeReceived = 2;
                    break;
                default:
                    break;
                }
                qDebug()<<"Reading type: "<<QString("%1").arg(readPacketType)<<" and sequentional number: "<<QString("%1").arg(readPacketSeq);
                qDebug()<<"Data to be readed: "<<QString("%1").arg(toBeReceived);
                readState = ReadData;
                break;
            case ReadData:
                qDebug()<<"Reading byte: "<<QString("0x%1").arg((quint8)byte, 0, 16);
                --toBeReceived;
                break;
            }
        }
        else            // read error
        {
            qDebug()<<"Read error";
            emit noRespond();
        }

        if(toBeReceived == 0 && readState == ReadData)
        {
            qDebug()<<"Stoping receive timeout timer";
            receiveTimeoutTimer->stop();
            emit printProgressInfo(QString("Data readed: ")+readedData.toHex());
            qDebug()<<"All bytes of packet received";
            if(calculateCRC(readedData, readedData.size()) == byte)
            {
                switch (readedData[1] & 0x0F)
                {
                case BootModeEnteredRespontPacket:
                    readedData = readedData.mid(2, 2);
                    qDebug()<<"Boot Mode Entered Respond received";
                    emit bootEnteredRespondReceived();
                    break;
                case ACKPacket:
                        qDebug()<<"ACK received";
                        emit receiveACK();
                    break;
                case CRCErrorPacket:
                    readedData = readedData.mid(2, 1);
                    qDebug()<<"CRCError Respond received";
                    emit receiveCRCError();
                    break;
                default:
                    break;
                }
            }
            else
            {
                qDebug()<<"CRC calculation error";
                qDebug()<<"Received CRC value: "<<QString("%1").arg((int)byte, 0, 16);
                emit CRCCalculateError();
            }
            readState = ReadAddres;
        }
        else
        {
            readedData.append(byte);
        }
    }
}


void DmxBootProtocol::handleSerialError(QSerialPort::SerialPortError error)
{
    FUNCTION_ENTER_TRACE;
    qDebug()<<"Some serial error ocured";
    if (error == QSerialPort::ResourceError) {
        emit printProgressInfo(QString("serial error hendler") + serialPort->errorString());
        QMessageBox::critical(NULL, tr("Critical Error"), serialPort->errorString());
    }
}

void DmxBootProtocol::checkSendedDataSize(qint64 bytes)
{
    FUNCTION_ENTER_TRACE;
    dataSizeWriten += bytes;
    qDebug()<<"Check if all data was sended, data sended up to now: "<<QString("%1").arg(dataSizeWriten)<<" size of data to be sended: "<<QString("%1").arg(dataSizeToWrite);
    if(!serialPort->bytesToWrite())
    {

        if(dataSizeWriten == dataSizeToWrite)
        {
            sendTimeoutTimer->stop();       // Started in sendStartBootRequest() function
            emit packetSended();
            emit progressInPercentage(progress());
        }
        else
        {
            qWarning()<<"No data to be writen in to serianl port but all data not sended!!!!";
        }
    }
}

void DmxBootProtocol::storePageSize()
{
    FUNCTION_ENTER_TRACE;
    pageSize = qFromBigEndian(*((qint16*)readedData.constData()));
    qDebug()<<"Storing page size: "<<pageSize;
}

void DmxBootProtocol::sendBootData()        // move data tu buffer and add 2 to lastSendedSeqNumber
{
    FUNCTION_ENTER_TRACE;
    if(currentPageNumber*pageSize > (quint32)dataToSend.size())
    {
        qDebug()<<"All data sended";
        emit allDataSended();
        return;
    }

    qDebug()<<"Sending boot data";
    QByteArray array;
    array.resize(3 + pageSize);     // 2 bytes - header, pageSize bytes - data, 1 byte CRC

    array[0] = deviceAddress;
    lastSendedSeqNumber = (lastSendedSeqNumber + 2) & 0x0f;     // seqention number is 4 bits value so we have to cut it in to 4 bits.
    array[1] = lastSendedSeqNumber<<4 | BootDataTransferePacket;   // seq nuber and packet type

    qDebug()<<"Current page number: "<<currentPageNumber;
    quint32 dataIndex = currentPageNumber*pageSize;
    ++currentPageNumber;
    quint32 maxDataIndex = qMin(dataIndex+pageSize, (quint32)dataToSend.size());
    qDebug()<<"maxDataIndex"<<maxDataIndex;
    qDebug()<<"(quint32)dataToSend.size()"<<(quint32)dataToSend.size();
    qDebug()<<"dataToSend.size()"<<dataToSend.size();
    qDebug()<<"Sending progress: "<<(100*((qreal)(maxDataIndex/dataToSend.size())))<<"% will be sended after this transmition";
    int i = 2;
    for(; dataIndex < maxDataIndex; dataIndex++, i++)
    {
        array[i] = dataToSend[dataIndex];
    }
    if(i < pageSize+2)
    {
        qDebug()<<"fill rest of packet with nop";
        for(; (qint32)i < pageSize+2; i++)
        {
            array[i] = 0xFF;
        }
    }
    array[i] = calculateCRC(array, pageSize+2);

    sendData(array);
}

void DmxBootProtocol::sendBootEnd()
{
    FUNCTION_ENTER_TRACE;
    // resolv conflict if sendErrorCRC and sendBootEnd

    if(!sendErrorFlag || (sendErrorFlag && sendErrorSeq != readedData[0]))
    {
        qDebug()<<"Sending boot end";
        QByteArray array;
        array.resize(3);
        array[0] = deviceAddress;
        lastSendedSeqNumber = (lastSendedSeqNumber + 2) & 0x0f;     // seqention number is 4 bits value so we have to cut it in to 4 bits.
        array[1] = lastSendedSeqNumber<<4 | EndBootModePacket;
        array[2] = calculateCRC(array, 2);

        sendData(array);
    }
    else
        sendCRCError();
}

void DmxBootProtocol::sendStartBootRequest()
{
    FUNCTION_ENTER_TRACE;

    emit printProgressInfo("sending boot start packet");
    qDebug()<<"Sending start boot request packet";

    QByteArray array;
    array.resize(3);
    array[0] = deviceAddress;
    array[1] = BootEnterRequestPacket;        // seqNuber = 0x00, packet type 0x01
    array[2] = calculateCRC(array, 2);     // should be 0x1F if I'm not misstaken

    sendData(array);

}

void DmxBootProtocol::resendBootData()
{
    FUNCTION_ENTER_TRACE;
    if(!sendErrorFlag || (sendErrorFlag && sendErrorSeq != readedData[0]))
    {
        --currentPageNumber;
        sendBootData();
    }
    else
        sendCRCError();
}

void DmxBootProtocol::resendBootEnd()
{
    FUNCTION_ENTER_TRACE;
    qDebug()<<"Resend boot end";
    if(!sendErrorFlag || (sendErrorFlag && sendErrorSeq != readedData[0]))
    {
        --currentPageNumber;
        sendBootEnd();
    }
    else
        sendCRCError();
}

void DmxBootProtocol::receiveTimeoutTimerTimeout()
{
    FUNCTION_ENTER_TRACE;
    qDebug()<<"Receive timeout read time timeouted";
    serialPort->close();
    emit noRespond();
}

void DmxBootProtocol::_DBPSuccess()
{
    FUNCTION_ENTER_TRACE;
    emit printProgressInfo("Flasing ended");
    qDebug()<<"Booting sucessfuly ended";
    serialPort->close();
    emit done(true);
}

void DmxBootProtocol::_sendError()
{
    FUNCTION_ENTER_TRACE;
    qCritical()<<"Unable send data";
    serialPort->close();
    emit errorOcured(CantWrite);
    emit done(false);
}

void DmxBootProtocol::_receiveTimeout()
{
    FUNCTION_ENTER_TRACE;
    qCritical()<<"Device is not responding";
    serialPort->close();
    emit errorOcured(ReadTimeout);
    emit done(false);
}

void DmxBootProtocol::createStateMachine()
{
    FUNCTION_ENTER_TRACE;
    stateMachine = new QStateMachine;
    QState *startBootState = new QState;
    QState *receiveBootStartRespondSatate = new QState;
    QState *sendBootDataState = new QState;
    QState *sendErrorReceiveBootEnterRespondState = new QState;
    QState *receiveDataRespondState = new QState;
    QState *sendErrorReceiveDataRespondState  = new QState;
    QState *resendBootDateState = new QState;
    QState *sendBootEndState = new QState;
    QState *receiveBootEndRespondState = new QState;
    QState *sendErrorreceiveRespondBootEndState = new QState;
    // Final states
    QFinalState *unableSendDataFinalState = new QFinalState;
    QFinalState *noDeviceRespondFinalState = new QFinalState;
    QFinalState *regularFinalState = new QFinalState;

    // Send Start Boot Start
    connect(startBootState, SIGNAL(entered()), this, SLOT(sendStartBootRequest()));        //state machine is started from startBootSequence
    startBootState->addTransition(this, SIGNAL(packetSended()), receiveBootStartRespondSatate);
    startBootState->addTransition(sendTimeoutTimer, SIGNAL(timeout()), unableSendDataFinalState);
    // Send Start Boot End

    // Receive Boot Enter Respond Start
    connect(receiveBootStartRespondSatate, SIGNAL(entered()), this, SLOT(startReceiveTimeoutTimer()), Qt::QueuedConnection);
    receiveBootStartRespondSatate->addTransition(this, SIGNAL(bootEnteredRespondReceived()), sendBootDataState);
    receiveBootStartRespondSatate->addTransition(this, SIGNAL(CRCCalculateError()), sendErrorReceiveBootEnterRespondState);
    receiveBootStartRespondSatate->addTransition(this ,SIGNAL(receiveCRCError()), sendErrorReceiveBootEnterRespondState);
    receiveBootStartRespondSatate->addTransition(this, SIGNAL(noRespond()), noDeviceRespondFinalState);
    connect(receiveBootStartRespondSatate, SIGNAL(exited()), this, SLOT(storePageSize()), Qt::QueuedConnection);
    // Receive Boot Enter Respond End

    // Receive Send Error Boot Enter Respond Start
    connect(sendErrorReceiveBootEnterRespondState, SIGNAL(entered()), this, SLOT(sendCRCError()), Qt::QueuedConnection);
    sendErrorReceiveBootEnterRespondState->addTransition(this, SIGNAL(CRCErrorSended()), receiveBootStartRespondSatate);
    sendErrorReceiveBootEnterRespondState->addTransition(sendTimeoutTimer, SIGNAL(timeout()), unableSendDataFinalState);
    // Receive Send Error Boot Enter Respond End

    // Send Boot Data Start
    connect(sendBootDataState, SIGNAL(entered()), this, SLOT(sendBootData()), Qt::QueuedConnection);
    sendBootDataState->addTransition(this, SIGNAL(packetSended()), receiveDataRespondState);
    sendBootDataState->addTransition(sendTimeoutTimer, SIGNAL(timeout()), unableSendDataFinalState);
    sendBootDataState->addTransition(this ,SIGNAL(allDataSended()), sendBootEndState);
    // Send Boot Data End

    // Receive Respond ACK/ERROR (receiveDataRespondState) Start
    connect(receiveDataRespondState, SIGNAL(entered()), this, SLOT(startReceiveTimeoutTimer()), Qt::QueuedConnection);
    receiveDataRespondState->addTransition(this ,SIGNAL(CRCCalculateError()), sendErrorReceiveDataRespondState);
    receiveDataRespondState->addTransition(this, SIGNAL(receiveACK()), sendBootDataState);
    receiveDataRespondState->addTransition(this ,SIGNAL(receiveCRCError()), resendBootDateState);
    receiveDataRespondState->addTransition(this, SIGNAL(noRespond()), noDeviceRespondFinalState);
    // Receive Respond ACK/ERROR (receiveDataRespondState) End

    // Send Error Receive Data Respond Sate Start
    connect(sendErrorReceiveDataRespondState, SIGNAL(entered()), this, SLOT(sendCRCError()), Qt::QueuedConnection);
    sendErrorReceiveDataRespondState->addTransition(this, SIGNAL(packetSended()), receiveDataRespondState);
    sendErrorReceiveDataRespondState->addTransition(sendTimeoutTimer, SIGNAL(timeout()), unableSendDataFinalState);
    // Send Error Receive Data Respond Sate End

    // Resend Boot Data Start
    connect(resendBootDateState, SIGNAL(entered()), this, SLOT(resendBootData()), Qt::QueuedConnection);
    resendBootDateState->addTransition(this, SIGNAL(packetSended()), receiveDataRespondState);
    resendBootDateState->addTransition(sendTimeoutTimer, SIGNAL(timeout()), unableSendDataFinalState);
    // Resend Boot Data End

    // Send Boot End Start
    connect(sendBootEndState, SIGNAL(entered()), this, SLOT(sendBootEnd()), Qt::QueuedConnection);
    sendBootEndState->addTransition(sendTimeoutTimer, SIGNAL(timeout()), unableSendDataFinalState);
    sendBootEndState->addTransition(this, SIGNAL(packetSended()), /*receiveBootEndRespondState*/regularFinalState);     // TODO Uncoment this commnet and find out why
                                                                                                                        // it receive wrong  CRC
    // Send Boot End End

    // Receive Respond ACK/ERROR Star
    connect(receiveBootStartRespondSatate, SIGNAL(entered()), this, SLOT(startReceiveTimeoutTimer()), Qt::QueuedConnection);
    receiveBootEndRespondState->addTransition(this, SIGNAL(CRCCalculateError()), sendBootEndState);
    receiveBootEndRespondState->addTransition(this, SIGNAL(receiveCRCError()), sendErrorreceiveRespondBootEndState);
    receiveBootEndRespondState->addTransition(this, SIGNAL(receiveACK()), regularFinalState);
    // Receive Respond ACK/ERROR End

    // Send Error Receive Respond Boot End State Start
    connect(sendErrorreceiveRespondBootEndState, SIGNAL(entered()), this, SLOT(resendBootEnd()), Qt::QueuedConnection);
    sendErrorreceiveRespondBootEndState->addTransition(sendTimeoutTimer, SIGNAL(timeout()), unableSendDataFinalState);
    sendErrorreceiveRespondBootEndState->addTransition(this, SIGNAL(packetSended()), receiveBootEndRespondState);
    // Send Error Receive Respond Boot End State End

    // Unable Send Data Final State Start
    connect(unableSendDataFinalState, SIGNAL(entered()), this, SLOT(_sendError()), Qt::QueuedConnection);
    // Unable Send Data Final State End

    // No Device Respond Final State Start
    connect(noDeviceRespondFinalState, SIGNAL(entered()), this, SLOT(_receiveTimeout()), Qt::QueuedConnection);
    // No Device Respond Final State End

    // Regular Final State Start
    connect(regularFinalState, SIGNAL(entered()), this, SLOT(_DBPSuccess()), Qt::QueuedConnection);
    // Regular Final State Start


    // put all states in to machine
    stateMachine->addState(startBootState);
    stateMachine->addState(receiveBootStartRespondSatate);
    stateMachine->addState(sendBootDataState);
    stateMachine->addState(sendErrorReceiveBootEnterRespondState);
    stateMachine->addState(receiveDataRespondState);
    stateMachine->addState(sendErrorReceiveDataRespondState);
    stateMachine->addState(resendBootDateState);
    stateMachine->addState(sendBootEndState);
    stateMachine->addState(receiveBootEndRespondState);
    stateMachine->addState(sendErrorreceiveRespondBootEndState);
    stateMachine->addState(unableSendDataFinalState);
    stateMachine->addState(noDeviceRespondFinalState);
    stateMachine->addState(regularFinalState);

    // set starting point in state machine
    stateMachine->setInitialState(startBootState);
}
