#ifndef DMXFLASHER_H
#define DMXFLASHER_H

#include "../../../app/interfaces/FlasherPluginInterface.h"
#include "DmxBootProtocol.h"

namespace Ui {
class DmxFlasher;
}

class QThread;
class QWidget;
class QTimer;
class WidgetWithNotifi;

class DmxFlasher : public FlasherPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "BootLoader.flesher.inteface-0-1" FILE "dmxFlasher.json")
    Q_INTERFACES(FlasherPluginInterface)
public:
    DmxFlasher();
    ~DmxFlasher();
    virtual void flash(QByteArray data);
    virtual QWidget* getPluginWidget();

signals:
    void showMessageBox(QString title, QString text, qint32 type);
    void startFlashing(QString portName, unsigned char deviceAddress, QByteArray dataToSend);
    void printProgressInfo(QString info);
    void progressInPercentage(qint32 progress);
    void done(bool success);

private slots:
    void refreshSerials();
    void showWidgetVisiblityChanget(bool isVisible);
    void serialComboboxExpanded(bool isExpanded);
    void showPortInfo(qint32 index);

    void DmxBootProtocolDoneSlot(bool success);
    void DmxBootPrintProgressInfoSlot(QString progress);
    void DmxBootProgressInPercentageSlot(qint32 progress);
    void DmxBootProtocolErrorOcured(DmxBootProtocol::Errors error);

private:
    WidgetWithNotifi *dmxFlasherWidget;
    Ui::DmxFlasher *ui;
    QTimer *serialListRefreshTimer;
    DmxBootProtocol *dmxBootProtocol;

    QThread *thread;
};

#endif // DMXFLASHER_H