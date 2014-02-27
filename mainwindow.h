#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <settingsdialog.h>

#include <DmxBootProtocol.h>
#include <HexReader.h>

namespace Ui {
class MainWindow;
}



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void printDebugInfo(QString text);

private slots:
    void on_actionOpen_File_triggered();

    void on_pushButton_clicked();

    void on_bootPushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;

    SettingsDialog *settingDialog;

    DmxBootProtocol dmxBootProtocol;

    HexReader reader;
    quint32 readerSize;
    QByteArray array;
};

#endif // MAINWINDOW_H
