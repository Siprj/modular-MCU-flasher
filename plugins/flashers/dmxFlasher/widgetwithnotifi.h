#ifndef WIDGETWITHNOTIFI_H
#define WIDGETWITHNOTIFI_H

#include <QWidget>

class WidgetWithNotifi : public QWidget
{
    Q_OBJECT
public:
    explicit WidgetWithNotifi(QWidget *parent = 0);

signals:
    void widgetVisibilityChanged(bool isVisible);

public slots:

protected:
    virtual void showEvent(QShowEvent *event);
    virtual void hideEvent(QHideEvent *event);

};

#endif // WIDGETWITHNOTIFI_H
