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


class FlasherPluginInterface : public QObject
{
public:
    virtual ~FlasherPluginInterface() {}
    virtual void flash(QByteArray data) = 0;
    virtual QWidget* getPluginWidget() = 0;

signals:
    virtual void printProgressInfo(QString info) = 0;
    virtual void progressInPercentage(float progress) = 0;
    virtual void done() = 0;
};

Q_DECLARE_INTERFACE(FlasherPluginInterface, "BootLoader.FlasherPluginInterface-1.0");

#endif // FLASHERPLUGIN_H
