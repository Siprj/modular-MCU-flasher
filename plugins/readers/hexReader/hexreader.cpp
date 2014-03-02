#include "hexreader.h"

#include "globalStatic.h"

#include "hexReaderCore.h"
#include <QDebug>
#include <QThread>

HexReader::HexReader()
{
    hexReaderCore = new HexReaderCore;
    thread = new QThread;
    moveToThread(thread);
    thread->start();
}

HexReader::~HexReader()
{
    if(hexReaderCore)
        delete hexReaderCore;
    delete thread;
}

void HexReader::readData(QString fileName)       // TODO finish this function
{
    emit printProgressInfo(tr(openingFileText));
    if(open(fileName))
        return;

    emit printProgressInfo(tr(readingHexFileText));
    quint32 dataSize = hexReaderCore->readHex();

    qDebug()<<"Something is REALY WRONG dta to readed: "<<dataSize;
    QByteArray dataArray;
    dataArray.resize(dataSize);

    emit printProgressInfo(tr(gettingDataFromReaderText));
    hexReaderCore->getData(0x00, dataSize, dataArray.data());
    hexReaderCore->close();

    emit progressInPercentage(100);
    emit printProgressInfo(tr(readingCompletedText));
    emit done(dataArray);
}

QList<SuffixesStructure> HexReader::getSuffixesGroups()
{
    QList<SuffixesStructure> suffixesStructList;
    SuffixesStructure suffixesStruct;
    suffixesStruct.groupName = tr(hexGroupName);
    QStringList suffixesList;
    suffixesList.append(hexSuffixes);
    suffixesStruct.suffixes = suffixesList;
    suffixesStructList.append(suffixesStruct);

    return suffixesStructList;
}

qint32 HexReader::open(QString fileName)
{
    if(!hexReaderCore->open(fileName.toStdString().c_str()))
    {
        qCritical()<<cantOpenFileErrorText<<fileName;
        emit printProgressInfo(QString(tr(cantOpenFileErrorText)).append(fileName));
        emit done(QByteArray());
        return 1;
    }
    else
    {
        qDebug()<<fileOpenedMessageText<<fileName;
        emit printProgressInfo(QString(tr(fileOpenedMessageText)).append(fileName));
        return 0;
    }
}
