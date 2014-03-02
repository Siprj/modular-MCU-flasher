#ifndef DMXBOOTPROTOCOL_H
#define DMXBOOTPROTOCOL_H

#include <QObject>

#include <QtSerialPort/QSerialPort>

#include <QTimer>

#include <QStateMachine>

class QStateMachine;

class DmxBootProtocol : public QObject
{
    Q_OBJECT
public:
    explicit DmxBootProtocol(QObject *parent = 0);
    ~DmxBootProtocol();

    enum Errors{
        CantOpenPort,
        CantWrite,
        ReadTimeout
    };

    enum ReadStates{
        ReadAddres,
        ReadTypeAndSeq,
        ReadData
    };

    enum PacketType{
        BootEnterRequestPacket = 1,
        BootModeEnteredRespontPacket = 2,
        BootDataTransferePacket = 3,
        ACKPacket = 4,
        CRCErrorPacket = 5,
        EndBootModePacket = 6
    };

signals:
    void done(bool success);
    void printProgressInfo(QString text);
    void progressInPercentage(qint32 percentage);

    // DmxBootProtocol
    void bootEnteredRespondReceived();
    void sendErrorOcurred();
    void packetSended();
    void allDataSended();
    void CRCErrorSended();
    void receiveCRCError();
    void CRCCalculateError();
    void noRespond();
    void receiveACK();

    // DBP internal error signals
    void errorOcured(DmxBootProtocol::Errors error);
    void sendError();
    void receiveTimeout();

    
public slots:
    void startBootSequence(QString portName, unsigned char deviceAddress, QByteArray dataToSend);

private slots:
    void startReceiveTimeoutTimer();
    void dataReceived();        // privately used for receive data from serial port
    void handleSerialError(QSerialPort::SerialPortError error);

    void checkSendedDataSize(qint64 bytes); // check if data was written and if there is
                                            // nothing to write (calculate right length of packets .....)

    void storePageSize();       // store received page size from receivedData bufer in to local variable
    void sendBootData();
    void sendBootEnd();
    void sendCRCError();

    void sendStartBootRequest();

    void resendBootData();
    void resendBootEnd();

    void receiveTimeoutTimerTimeout();

    //! internal slots used by state machine

    // DBP state machine ended secesfully
    void _DBPSuccess();

    // DBP error slots for state machine
    void _sendError();
    void _receiveTimeout();

private:
    void createStateMachine();
    int openPort();
    char calculateCRC(QByteArray array, qint32 size);
    void sendData(QByteArray array);
    qint32 progress();

    QString portName;
    qint8 deviceAddress;

    QSerialPort *serialPort;
    QStateMachine *stateMachine;

    QTimer *sendTimeoutTimer;
    QTimer *receiveTimeoutTimer;

    QByteArray readedData;
    ReadStates readState;
    quint8 lastSendedSeqNumber;
    quint32 currentPageNumber;
    qint64 dataSizeReaded;
    qint32 toBeReceived;
    quint8 readPacketType;
    quint8 readPacketSeq;
    qint32 pageSize;   // received from MCU

    qint32 sendErrorFlag;
    qint32 sendErrorSeq;


    QByteArray dataToSend;
    qint64 dataSizeToWrite;
    qint64 dataSizeWriten;

    int pokus;
//    QTimer *timer;
};

#endif // DMXBOOTPROTOCOL_H
