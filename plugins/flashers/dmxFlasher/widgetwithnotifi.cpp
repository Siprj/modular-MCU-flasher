#include "widgetwithnotifi.h"

#include "../../../app/interfaces/trace.h"

WidgetWithNotifi::WidgetWithNotifi(QWidget *parent) :
    QWidget(parent)
{
    FUNCTION_ENTER_TRACE;
}

void WidgetWithNotifi::showEvent(QShowEvent *event)
{
    FUNCTION_ENTER_TRACE;
    emit widgetVisibilityChanged(true);
    QWidget::showEvent(event);
}

void WidgetWithNotifi::hideEvent(QHideEvent *event)
{
    FUNCTION_ENTER_TRACE;
    emit widgetVisibilityChanged(false);
    QWidget::hideEvent(event);
}
