#ifndef WIDGETWITHNOTIFI_H
#define WIDGETWITHNOTIFI_H

#include <QWidget>

namespace Ui {
class DmxFlasherWidget;
}

class QTimer;
class QMutex;

class DmxFlasherWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DmxFlasherWidget(QWidget *parent = 0);
    virtual ~DmxFlasherWidget();

    QString getSerialPortName();
    unsigned char getDeviceAddress();

signals:
    void widgetVisibilityChanged(bool isVisible);

private slots:
    void refreshSerials();
    void showWidgetVisiblityChanget(bool isVisible);
    void serialComboboxExpanded(bool isExpanded);
    void publicPortInfo(qint32 index);
    void publicDeviceAddress(QString);

protected:
    virtual void showEvent(QShowEvent *event);
    virtual void hideEvent(QHideEvent *event);

private:
    QTimer *serialListRefreshTimer;


    Ui::DmxFlasherWidget *ui;

    QMutex *mutex;
    QString *selectedSerialPortName;
    unsigned char deviceAddress;
};

#endif // WIDGETWITHNOTIFI_H
