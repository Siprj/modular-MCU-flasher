#ifndef FLASHERPLUGIN_H
#define FLASHERPLUGIN_H

#include "BaseInterface.h"

QT_BEGIN_NAMESPACE
class QString;
class QStringList;
class QObject;
class QByteArray;
class QWidget;
QT_END_NAMESPACE

class FlasherPluginInterface : public BaseInterface
{
public:
    virtual ~FlasherPluginInterface() {}
    virtual void flash(QByteArray data) = 0;
    virtual QWidget* getPluginWidget() = 0;

signals:
    virtual void done(bool success) = 0;
};

Q_DECLARE_INTERFACE(FlasherPluginInterface, "BootLoader.FlasherPluginInterface-1.0");

#endif // FLASHERPLUGIN_H
