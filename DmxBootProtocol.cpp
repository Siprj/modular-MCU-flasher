#include "DmxBootProtocol.h"

#include <QStringList>

#include <QTimer>
#include <QMessageBox>
#include <QState>
#include <QStateMachine>
#include <QFinalState>
#include <QtEndian>

#include <QDebug>

#define SERIAL_BAUD_RATE 250000
#define SERIAL_DATA_SIZE QSerialPort::Data8
#define SERIAL_PARITY QSerialPort::NoParity
#define SERIAL_STOPBITS QSerialPort::TwoStop
#define SERIAL_FLOW_CONTROL QSerialPort::NoFlowControl

#define WRITE_TIME_OUT 2000 //in ms

DmxBootProtocol::DmxBootProtocol(QString portName, QObject *parent) :
    QObject(parent)
{
    qDebug("DMX Boot Protocol Constructor");
    this->portName = portName;
    serialPort = new QSerialPort(this);
    sendTimeoutTimer = new QTimer;
    sendTimeoutTimer->setSingleShot(true);
    receiveTimeoutTimer = new QTimer;
    receiveTimeoutTimer->setSingleShot(true);

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

    connect(serialPort, SIGNAL(readyRead()), this, SLOT(dataReceived()));
    connect(serialPort, SIGNAL(bytesWritten(qint64)), this, SLOT(checkSendedDataSize(qint64)));
}

DmxBootProtocol::~DmxBootProtocol()
{
    qDebug("DMX Boot Protocol Destructor");
    delete serialPort;
    delete stateMachine;
    delete sendTimeoutTimer;
    delete receiveTimeoutTimer;
}

void DmxBootProtocol::startBootSequence(QString portName, unsigned char deviceAddress, QByteArray dataToSend)
{
    qDebug()<<"Starting flasing device";
    this->dataToSend = dataToSend;
    this->deviceAddress = deviceAddress;
    this->portName = portName;

    if(!openPort())
    {
        emit errorOcured(CantOpenPort);
        return;
    }

    stateMachine->start();
}

void DmxBootProtocol::testik(QString portName, unsigned char address)
{
    deviceAddress = address;
    this->portName = portName;
    testikPole.append(0);
    testikPole.append(0);
    testikPole.append(0);
    testikPole.append(1);
    testikPole.append(1);
    testikPole.append(1);
    testikPole.append(0);
    testikPole.append(0);
    testikPole.append(0);
    testikPole.append(0);
    testikPole.append(0);
    testikPole.append(0);
    testikPole.append(0);
    testikPole.append(0);
    qDebug()<<"testikPole size: "<<testikPole.size();
}

int DmxBootProtocol::openPort()
{
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
    qDebug()<<"Calcluate CRC from data: "<<array.toHex()<<" size of array: "<<size;
    qDebug()<<"True size of array: "<<array.size();
    char crc = 0;

    for(int i = 0; i < size; i++)
    {
        crc += array[i];
    }
    crc = (char)(0xFF - crc) + 1;

    qDebug()<<"Compiuted CRC is: "<<QString("%1").arg((int)crc, 0, 16);
#ifdef TEST
    if(testikPole.size() > 0)
    {
        static int y = 0;
        if(testikPole.at(y))
        {
            qWarning()<<"CRC Testik PRACUJE !!!!!!!!!!!!";
            crc += 2;
        }
        ++y;
        if(y > testikPole.size())
            y = 0;
    }
#endif
    return crc;
}

void DmxBootProtocol::sendData(QByteArray array)
{
    dataSizeWriten = 0;
    dataSizeToWrite = array.size();
    sendTimeoutTimer->start(WRITE_TIME_OUT);        // will be stoped

    if(sendErrorFlag)   // flag is se when we sending error respond
        --sendErrorFlag;        // value 2 means we are sending error mesage right now
                                // value 1 means we sended error message in previous cycle
    qDebug()<<"Sending data: "<<array.toHex();

    serialPort->write(array);
}

void DmxBootProtocol::sendCRCError()    // TODO set seq numbers, check packet length
{
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
            case ReadTypeAndSeq:        // TODO add seq number in to some variable
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
    qDebug()<<"Some serial error ocured";
    if (error == QSerialPort::ResourceError) {
        emit printProgressInfo(QString("serial error hendler") + serialPort->errorString());
        QMessageBox::critical(NULL, tr("Critical Error"), serialPort->errorString());
    }
}

void DmxBootProtocol::checkSendedDataSize(qint64 bytes)
{
    dataSizeWriten += bytes;
    qDebug()<<"Check if all data was sended, data sended up to now: "<<QString("%1").arg(dataSizeWriten)<<" size of data to be sended: "<<QString("%1").arg(dataSizeToWrite);
    if(!serialPort->bytesToWrite())
    {

        if(dataSizeWriten == dataSizeToWrite)
        {
            sendTimeoutTimer->stop();       // Started in sendStartBootRequest() function
            emit packetSended();
        }
        else
        {
            qWarning()<<"No data to be writen in to serianl port but all data not sended!!!!";
        }
    }
}

void DmxBootProtocol::storePageSize()
{
    pageSize = qFromBigEndian(*((qint16*)readedData.constData()));
    qDebug()<<"Storing page size: "<<pageSize;
}

void DmxBootProtocol::sendBootData()        // move data tu buffer and add 2 to lastSendedSeqNumber
{
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
            array[i] = 0xFF;    // fill the res of memory with NOP  // TODO check if nop i realy 0xFF
        }
    }
    array[i] = calculateCRC(array, pageSize+2);

    sendData(array);
}

void DmxBootProtocol::sendBootEnd()
{
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

    emit printProgressInfo("sending boot start packet");
    qDebug()<<"Sending start boot request packet";

    QByteArray array;
    array.resize(3);
    array[0] = deviceAddress;
    array[1] = BootEnterRequestPacket;        // seqNuber = 0x00, packet type 0x01
    array[2] = calculateCRC(array, 2);     // should be 0x1F if I'm not misstaken

    sendData(array);

}

void DmxBootProtocol::resendBootData()      // TODO send CRCError or set flags for data send function
{
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
    if(!sendErrorFlag || (sendErrorFlag && sendErrorSeq != readedData[0]))
    {
        --currentPageNumber;
        sendBootEnd();
    }
    else
        sendCRCError();
}

void DmxBootProtocol::_DBPSuccess()
{
    emit printProgressInfo("Flasing ended");
    qDebug()<<"Booting sucessfuly ended";
    serialPort->close();
}

void DmxBootProtocol::_sendError()
{
    qCritical()<<"Unable send data";
    serialPort->close();
}

void DmxBootProtocol::_receiveTimeout()
{
    qCritical()<<"Device is not responding";
    serialPort->close();
}

void DmxBootProtocol::createStateMachine()
{
    stateMachine = new QStateMachine;
    QState *startBootState = new QState;
    QState *receiveBootStartRespondSatate = new QState;
    QState *unableSendDataState = new QState;
    QState *sendBootDataState = new QState;
    QState *sendErrorReceiveBootEnterRespondState = new QState;
    QState *noDeviceRespondState = new QState;
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
    startBootState->addTransition(sendTimeoutTimer, SIGNAL(timeout()), unableSendDataState);
    // Send Start Boot End

    // Receive Boot Enter Respond Start
    receiveBootStartRespondSatate->addTransition(this, SIGNAL(bootEnteredRespondReceived()), sendBootDataState);
    receiveBootStartRespondSatate->addTransition(this, SIGNAL(CRCCalculateError()), sendErrorReceiveBootEnterRespondState);
    receiveBootStartRespondSatate->addTransition(this ,SIGNAL(receiveCRCError()), sendErrorReceiveBootEnterRespondState);
    receiveBootStartRespondSatate->addTransition(this, SIGNAL(noRespond()), noDeviceRespondState);
    connect(receiveBootStartRespondSatate, SIGNAL(exited()), this, SLOT(storePageSize()));
    // Receive Boot Enter Respond End

    // Receive Send Error Boot Enter Respond Start
    connect(sendErrorReceiveBootEnterRespondState, SIGNAL(entered()), this, SLOT(sendCRCError()));
    sendErrorReceiveBootEnterRespondState->addTransition(this, SIGNAL(CRCErrorSended()), receiveBootStartRespondSatate);
    sendErrorReceiveBootEnterRespondState->addTransition(sendTimeoutTimer, SIGNAL(timeout()), unableSendDataState);
    // Receive Send Error Boot Enter Respond End

    // Send Boot Data Start
    connect(sendBootDataState, SIGNAL(entered()), this, SLOT(sendBootData()));
    sendBootDataState->addTransition(this, SIGNAL(packetSended()), receiveDataRespondState);
    sendBootDataState->addTransition(sendTimeoutTimer, SIGNAL(timeout()), unableSendDataState);
    sendBootDataState->addTransition(this ,SIGNAL(allDataSended()), sendBootEndState);
    // Send Boot Data End

    // Receive Respond ACK/ERROR (receiveDataRespondState) Start
    connect(resendBootDateState, SIGNAL(entered()), this, SLOT(resendBootData()));
    receiveDataRespondState->addTransition(this ,SIGNAL(CRCCalculateError()), sendErrorReceiveDataRespondState);
    receiveDataRespondState->addTransition(this, SIGNAL(receiveACK()), sendBootDataState);
    receiveDataRespondState->addTransition(this ,SIGNAL(receiveCRCError()), resendBootDateState);
    receiveDataRespondState->addTransition(this, SIGNAL(noRespond()), noDeviceRespondState);
    // Receive Respond ACK/ERROR (receiveDataRespondState) End

    // Send Error Receive Data Respond Sate Start
    connect(sendErrorReceiveDataRespondState, SIGNAL(entered()), this, SLOT(sendCRCError()));
    sendErrorReceiveDataRespondState->addTransition(this, SIGNAL(packetSended()), receiveDataRespondState);
    sendErrorReceiveDataRespondState->addTransition(sendTimeoutTimer, SIGNAL(timeout()), unableSendDataState);
    // Send Error Receive Data Respond Sate End

    // Resend Boot Data Start
    resendBootDateState->addTransition(this, SIGNAL(packetSended()), receiveDataRespondState);
    resendBootDateState->addTransition(sendTimeoutTimer, SIGNAL(timeout()), unableSendDataState);
    // Resend Boot Data End

    // Send Boot End Start
    connect(sendBootEndState, SIGNAL(entered()), this, SLOT(sendBootEnd()));
    sendBootEndState->addTransition(sendTimeoutTimer, SIGNAL(timeout()), unableSendDataState);
    sendBootEndState->addTransition(this, SIGNAL(packetSended()), receiveBootEndRespondState);
    // Send Boot End End

    // Receive Respond ACK/ERROR Star
    receiveBootEndRespondState->addTransition(this, SIGNAL(CRCCalculateError()), sendBootEndState);
    receiveBootEndRespondState->addTransition(this, SIGNAL(receiveCRCError()), sendErrorreceiveRespondBootEndState);
    receiveBootEndRespondState->addTransition(this, SIGNAL(receiveACK()), regularFinalState);
    // Receive Respond ACK/ERROR End

    // Send Error Receive Respond Boot End State Start
    connect(sendErrorreceiveRespondBootEndState, SIGNAL(entered()), this, SLOT(resendBootEnd()));
    sendErrorreceiveRespondBootEndState->addTransition(sendTimeoutTimer, SIGNAL(timeout()), unableSendDataState);
    sendErrorreceiveRespondBootEndState->addTransition(this, SIGNAL(packetSended()), receiveBootEndRespondState);
    // Send Error Receive Respond Boot End State End

    // Unable Send Data Final State Start
    connect(unableSendDataFinalState, SIGNAL(entered()), this, SLOT(_sendError()));
    // Unable Send Data Final State End

    // No Device Respond Final State Start
    connect(noDeviceRespondFinalState, SIGNAL(entered()), this, SLOT(_receiveTimeout()));
    // No Device Respond Final State End

    // Regular Final State Start
    connect(regularFinalState, SIGNAL(entered()), this, SLOT(_DBPSuccess()));
    // Regular Final State Start


    // put all states in to machine
    stateMachine->addState(startBootState);
    stateMachine->addState(receiveBootStartRespondSatate);
    stateMachine->addState(unableSendDataState);
    stateMachine->addState(sendBootDataState);
    stateMachine->addState(sendErrorReceiveBootEnterRespondState);
    stateMachine->addState(noDeviceRespondState);
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
