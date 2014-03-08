#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QMap>
#include <QWidget>


#include "interfaces/trace.h"
#include "interfaces/FlasherPluginInterface.h"
#include "interfaces/ReaderPluginInterface.h"
#include "interfaces/BaseInterface.h"

namespace Ui {
class MainWindow;
}

class QPluginLoader;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    enum FlashinSate{
        ReadingDataState = 0,
        FlashingDeviceState = 1,
        NumberOfStates
    };

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void printProgressInfo(QString text);

private slots:
    void on_actionOpen_File_triggered();
    void on_flashButton_clicked();

    void done(QByteArray data);        // finish slot for Reder Plugins
    void done(bool success);                        // finish slot for Flashing Plugins
    void progressInPercentage(qint32 percentageProgress);
    void showMessageBox(QString title, QString text, qint32 type);

private:
    void loadPlugins();
    void publishPlugin(QObject *plugin, QStringList &composedGroupSuffixesList);
    void unknownFileSuffixMssage();
    void noFileSelected();

    Ui::MainWindow *ui;

    QByteArray dataArray;

    QStringList listOfSuffexesGroups;
    // <wodget of plugin, flasher plugin>
    QMap<QWidget*, QObject*> flasherMap;
    QString suffixesFilters;

    QString fileToRead;

    // <siffixes, reader plugin>
    QMap<QString, QObject*> readersSuffixesMap;
    ReaderPluginInterface *currentReader;
    FlasherPluginInterface *currentFlasher;

    FlashinSate flashingState;
};

#endif // MAINWINDOW_H
