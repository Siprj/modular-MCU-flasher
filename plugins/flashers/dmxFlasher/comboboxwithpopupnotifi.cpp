#include "comboboxwithpopupnotifi.h"

#include "../../../app/interfaces/trace.h"

ComboBoxWithPopupNotifi::ComboBoxWithPopupNotifi(QWidget *parent) :
    QComboBox(parent)
{
    FUNCTION_ENTER_TRACE;
}

void ComboBoxWithPopupNotifi::showPopup()
{
    FUNCTION_ENTER_TRACE;
    if(this->count())
        emit popupExpanded(true);
    QComboBox::showPopup();
}

void ComboBoxWithPopupNotifi::hidePopup()
{
    FUNCTION_ENTER_TRACE;
    emit popupExpanded(false);
    QComboBox::hidePopup();
}
