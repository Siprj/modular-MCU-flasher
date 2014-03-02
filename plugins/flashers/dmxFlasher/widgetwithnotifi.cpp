#include "widgetwithnotifi.h"

WidgetWithNotifi::WidgetWithNotifi(QWidget *parent) :
    QWidget(parent)
{
}

void WidgetWithNotifi::showEvent(QShowEvent *event)
{
    emit widgetVisibilityChanged(true);
    QWidget::showEvent(event);
}

void WidgetWithNotifi::hideEvent(QHideEvent *event)
{
    emit widgetVisibilityChanged(false);
    QWidget::hideEvent(event);
}
