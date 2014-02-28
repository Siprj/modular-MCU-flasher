#ifndef READERPLUGININTERFACE_H
#define READERPLUGININTERFACE_H

#include <QtPlugin>

QT_BEGIN_NAMESPACE
class QString;
class QStringList;
class QObject;
class QByteArray;
QT_END_NAMESPACE

typedef struct
{
    QString groupName;
    QStringList suffixes;
}SuffixesStructure;

class ReaderPluginInterface : public QObject
{
public:
    virtual ~ReaderPluginInterface() {}
    virtual quint32 readData(QString fileName) = 0;
    virtual QList<SuffixesStructure> getSuffixesGroups() = 0;

signals:
    virtual void printProgressInfo(QString info) = 0;
    virtual void progressInPercentage(float progress) = 0;
    virtual void done(QByteArray data) = 0;
};

Q_DECLARE_INTERFACE(ReaderPluginInterface, "BootLoader.ReaderPluginInterface-1.0");

#endif // READERPLUGININTERFACE_H
