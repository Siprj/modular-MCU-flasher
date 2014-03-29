#ifndef READERPLUGININTERFACE_H
#define READERPLUGININTERFACE_H

#include "BaseInterface.h"
#include <QStringList>

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

class ReaderPluginInterface : public BaseInterface
{
public:
    virtual ~ReaderPluginInterface() {}

    /**
     * \brief Function readData starts asynchronous read implemented by plugin. If the read is not short it can be synchronous but it is not recomended.
     * \warning Main app is counting that the function readData is not blocking!!!!
     * \note Plugins shoul emit done(QByteArray data) signal when is finishes and if seome error ocure it shoul put empty array in to th signal.
     * \param fileName - file which is to be readed by plugin.
     *
     */
    virtual void readData(QString fileName) = 0;
    virtual QList<SuffixesStructure> getSuffixesGroups() = 0;

signals:
    virtual void done(QByteArray data) = 0;
};

Q_DECLARE_INTERFACE(ReaderPluginInterface, "BootLoader.ReaderPluginInterface-1.1");

#endif // READERPLUGININTERFACE_H
