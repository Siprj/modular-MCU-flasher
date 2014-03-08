#ifndef FLASHERPLUGIN_H
#define FLASHERPLUGIN_H

#include <QtPlugin>

QT_BEGIN_NAMESPACE
class QString;
class QStringList;
class QObject;
class QByteArray;
class QWidget;
QT_END_NAMESPACE

#ifndef MESSAGE_BOX_WARNING
#define MESSAGE_BOX_WARNING         0
#endif

#ifndef MESSAGE_BOX_INFORMATION
#define MESSAGE_BOX_INFORMATION     1
#endif

#ifndef MESSAGE_BOX_CRITICIAL
#define MESSAGE_BOX_CRITICIAL       2
#endif

class FlasherPluginInterface : public QObject
{
public:
    virtual ~FlasherPluginInterface() {}
    virtual void flash(QByteArray data) = 0;
    virtual QWidget* getPluginWidget() = 0;

signals:
    virtual void showMessageBox(QString title, QString text, qint32 type) = 0;
    virtual void printProgressInfo(QString info) = 0;
    virtual void progressInPercentage(qint32 progress) = 0;
    virtual void done(bool success) = 0;
};

Q_DECLARE_INTERFACE(FlasherPluginInterface, "BootLoader.FlasherPluginInterface-1.0");

#endif // FLASHERPLUGIN_H
