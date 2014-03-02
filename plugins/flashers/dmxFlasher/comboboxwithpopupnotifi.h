#ifndef COMBOBOXWITHPOPUPNOTIFI_H
#define COMBOBOXWITHPOPUPNOTIFI_H

#include <QComboBox>

class ComboBoxWithPopupNotifi : public QComboBox
{
    Q_OBJECT
public:
    explicit ComboBoxWithPopupNotifi(QWidget *parent = 0);
    void showPopup();
    void hidePopup();

signals:
    void popupExpanded(bool isExpanded);

public slots:
};

#endif // COMBOBOXWITHPOPUPNOTIFI_H
