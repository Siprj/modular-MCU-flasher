#ifndef BASEINTERFACE_H
#define BASEINTERFACE_H

#include <QtPlugin>
#include <QSettings>

#define MESSAGE_BOX_WARNING         0
#define MESSAGE_BOX_INFORMATION     1
#define MESSAGE_BOX_CRITICIAL       2

class BaseInterface : public QObject
{
public:
    virtual ~BaseInterface() {}
    virtual void putSettings(QSettings *settings) = 0;
signals:
    virtual void showMessageBox(QString title, QString text, qint32 type) = 0;
    virtual void printProgressInfo(QString info) = 0;
    virtual void progressInPercentage(qint32 progress) = 0;
};

Q_DECLARE_INTERFACE(BaseInterface, "BootLoader.BaseInterface-1.1");

#endif // BASEINTERFACE_H
