#include "hexreader.h"

#include "globalStatic.h"

#include "hexReaderCore.h"
#include <QDebug>
#include <QThread>
#include <QMessageBox>

#include "../../../app/interfaces/trace.h"

HexReader::HexReader()
{
    FUNCTION_ENTER_TRACE;
    hexReaderCore = new HexReaderCore;
    thread = new QThread;
    moveToThread(thread);
    thread->start();
}

HexReader::~HexReader()
{
    FUNCTION_ENTER_TRACE;
    if(hexReaderCore)
        delete hexReaderCore;
    delete thread;
}

void HexReader::readData(QString fileName)
{
    FUNCTION_ENTER_TRACE;
    emit printProgressInfo(tr(openingFileText));
    if(open(fileName))
        return;

    emit printProgressInfo(tr(readingHexFileText));
    quint32 dataSize = hexReaderCore->readHex();

    if(dataSize == 0)
    {
        qDebug()<<"Redadni HEX faild: "<<hexReaderCore->getErrorStr().c_str();
        QMessageBox::warning(NULL, "asdfasdf", "asdfasdf");
        done(QByteArray());
        return;
    }

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
    FUNCTION_ENTER_TRACE;
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
    FUNCTION_ENTER_TRACE;
    if(!hexReaderCore->open(fileName.toStdString().c_str()))
    {
        qCritical()<<cantOpenFileErrorText<<fileName;
        emit printProgressInfo(QString(tr(cantOpenFileErrorText)).append(fileName));
        QMessageBox::warning(NULL, tr(cantOpenFileErrorText), QString(tr(cantOpenFileErrorText)).append(fileName));
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
