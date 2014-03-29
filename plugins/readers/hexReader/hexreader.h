#ifndef HEXREADER_H
#define HEXREADER_H

#include "../../../app/interfaces/ReaderPluginInterface.h"

class HexReaderCore;
class QThread;

class HexReader : public ReaderPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "BootLoader.reader.HexReader-0-1" FILE "hexReader.json")
    Q_INTERFACES(ReaderPluginInterface)
public:
    HexReader();
    virtual ~HexReader();
    virtual void putSettings(QSettings *settings);
    virtual void readData(QString fileName);
    virtual QList<SuffixesStructure> getSuffixesGroups();
signals:
    void showMessageBox(QString title, QString text, qint32 type);
    void printProgressInfo(QString info);
    void progressInPercentage(qint32 progress);
    void done(QByteArray data);

private:
    qint32 open(QString fileName);

    HexReaderCore *hexReaderCore;
    QThread *thread;
};

#endif // HEXREADER_H
