#include "comboboxwithpopupnotifi.h"

ComboBoxWithPopupNotifi::ComboBoxWithPopupNotifi(QWidget *parent) :
    QComboBox(parent)
{
}

void ComboBoxWithPopupNotifi::showPopup()
{
    emit popupExpanded(true);
    QComboBox::showPopup();
}

void ComboBoxWithPopupNotifi::hidePopup()
{
    emit popupExpanded(false);
    QComboBox::hidePopup();
}
