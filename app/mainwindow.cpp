#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGridLayout>
#include <QWidget>
#include <QLabel>

#include <QFileDialog>

#include <QDebug>
#include <QPluginLoader>
#include <QMessageBox>

#include <QSettings>

#define LAST_READ_PATH_SETTINGS "mainApp/lastReadFolderPosition"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    FUNCTION_ENTER_TRACE;
    ui->setupUi(this);
    ui->progressBar->setVisible(false);



    currentReader = NULL;
    currentFlasher = NULL;

    flashingState = ReadingDataState;

    connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()), Qt::QueuedConnection);
    connect(ui->actionProgram, SIGNAL(triggered()), this, SLOT(on_programChipButton_clicked()), Qt::QueuedConnection);
    connect(ui->openFilePushButton, SIGNAL(clicked()), this, SLOT(on_actionOpen_File_triggered()), Qt::QueuedConnection);

    settingsGlobal = new QSettings(QSettings::UserScope, "BootLoaderPC", "BootLoaderPC");
    loadSettings();

    loadPlugins();
}

MainWindow::~MainWindow()
{
    FUNCTION_ENTER_TRACE;
    delete ui;
}

void MainWindow::printProgressInfo(QString text)
{
    FUNCTION_ENTER_TRACE;
    qDebug()<<"Printing progress info message in to status bar: "<<text;
    ui->statusBar->showMessage(text, 10000);
}

void MainWindow::on_actionOpen_File_triggered()
{
    FUNCTION_ENTER_TRACE;
    currentReader = NULL;
    fileToRead = QFileDialog::getOpenFileName(this, "Source file", lastReadFolderPosition /*"Z:\\Yrid\\elektro\\ATmega8 soutez adc\\ATmega8 soutez adc\\Debug"*/, suffixesFilters);
    qDebug()<<"File to open"<<fileToRead;
    if(!fileToRead.isEmpty())
    {
        QFileInfo fileInfo(fileToRead);

        lastReadFolderPosition = fileInfo.absoluteDir().path();
        settingsGlobal->setValue(LAST_READ_PATH_SETTINGS, lastReadFolderPosition);

        ui->openFileLineEdit->setText(fileToRead);
        if(readersSuffixesMap.contains(fileInfo.suffix()))
        {
            currentReader = qobject_cast<ReaderPluginInterface*>(readersSuffixesMap[fileInfo.suffix()]);
        }
        else
            unknownFileSuffixMssage();
    }

    qDebug()<<"currentReader: "<<currentReader;
}

void MainWindow::loadPlugins()
{
    FUNCTION_ENTER_TRACE;
    qDebug()<<"Loading Plugins";

    QDir pluginsDir = QDir(qApp->applicationDirPath());
    pluginsDir.cd("plugins");

    qDebug()<<"Try to load plugins in folder "<<pluginsDir.path();

    QStringList composedGroupSuffixesList;          // Suffixes filters
    composedGroupSuffixesList.append("All (*)");    // Add filter for all files

    foreach(QString fileName, pluginsDir.entryList(QDir::Files))
    {
        qDebug()<<"Testing: "<<pluginsDir.absoluteFilePath(fileName);
        QPluginLoader *loader = new QPluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = loader->instance();
        if (plugin)
        {
            qDebug()<<"Loading: "<<fileName;
            // load
            publishPlugin(plugin, composedGroupSuffixesList);
        }
        else
        {
            qDebug()<<"Can't load plugin";
            qDebug()<<loader->errorString();
        }
    }

    composedGroupSuffixesList.sort();
    suffixesFilters = composedGroupSuffixesList.join(";;"); // compose all filter in to one string



    qDebug()<<"Loadnig of plugins completed";
}

void MainWindow::publishPlugin(QObject *plugin, QStringList &composedGroupSuffixesList)
{
    FUNCTION_ENTER_TRACE;
    BaseInterface *basicInterface = dynamic_cast<BaseInterface*>(plugin);
    if(basicInterface)
    {
        qDebug()<<"plugins is derivated from BaseInterface";
        connect(basicInterface, SIGNAL(printProgressInfo(QString)), this, SLOT(printProgressInfo(QString)), Qt::QueuedConnection);
        connect(basicInterface, SIGNAL(progressInPercentage(qint32)), this, SLOT(progressInPercentage(qint32)), Qt::QueuedConnection);
        connect(basicInterface, SIGNAL(showMessageBox(QString,QString,qint32)), this, SLOT(showMessageBox(QString,QString,qint32)), Qt::QueuedConnection);
        basicInterface->putSettings(settingsGlobal);
    }


    FlasherPluginInterface *flasherPlugin = dynamic_cast<FlasherPluginInterface*>(plugin);
    if(flasherPlugin)
    {
        qDebug()<<"plugins is derivated from FlasherPluginInterface";
        connect(flasherPlugin, SIGNAL(done(bool)), this, SLOT(done(bool)), Qt::QueuedConnection);
        QWidget *widget = flasherPlugin->getPluginWidget();
        ui->flashingTabWidget->addTab(widget, widget->windowTitle());
        flasherMap[widget] = plugin;
    }

    ReaderPluginInterface *readerPlugin = dynamic_cast<ReaderPluginInterface*>(plugin);
    if(readerPlugin)
    {
        qDebug()<<"plugins is derivated from ReaderPluginInterface";

        connect(readerPlugin, SIGNAL(done(QByteArray)), this, SLOT(done(QByteArray)), Qt::QueuedConnection);

        qDebug()<<"Printing suffix group";
        QList<SuffixesStructure> suffixesListHelp = readerPlugin->getSuffixesGroups();
        for(int i = 0; i < suffixesListHelp.size(); i++)
        {
            for(int y = 0; y < suffixesListHelp.at(i).suffixes.size(); y++)
            {
                const QString removeWildCard = QString("*.");
                QString messUpSuffix = suffixesListHelp.at(i).suffixes.at(y);
                QString suffix = messUpSuffix.remove(removeWildCard);
                readersSuffixesMap[suffix] = plugin;
            }
            QString groupeSuffixes;
            groupeSuffixes.append(suffixesListHelp.at(i).groupName).append(" (");
            groupeSuffixes.append(suffixesListHelp.at(i).suffixes.join(" ")).append(")");
            composedGroupSuffixesList.append(groupeSuffixes);

            qDebug()<<suffixesListHelp.at(i).groupName;
            qDebug()<<suffixesListHelp.at(i).suffixes;
        }
    }
}

void MainWindow::unknownFileSuffixMssage()
{
    FUNCTION_ENTER_TRACE;
    QMessageBox::warning(this, tr("Unknown file format"), tr("Unknown file format"));
}

void MainWindow::noFileSelected()
{
    FUNCTION_ENTER_TRACE;
    QMessageBox::warning(this, tr("No file selected"), tr("You mast select some file first"));
}

void MainWindow::loadSettings()
{
    FUNCTION_ENTER_TRACE;
    lastReadFolderPosition = settingsGlobal->value(LAST_READ_PATH_SETTINGS, "").toString();
}

void MainWindow::on_programChipButton_clicked()
{
    FUNCTION_ENTER_TRACE;
    flashingState = ReadingDataState;
    if(currentReader)
    {
        ui->progressBar->setVisible(true);
        ui->progressBar->setValue(0);
        repaint();
        currentReader->readData(fileToRead);
        currentFlasher = qobject_cast<FlasherPluginInterface*>(flasherMap[ui->flashingTabWidget->currentWidget()]);
    }
    else
        noFileSelected();
}

void MainWindow::done(QByteArray data)
{
    FUNCTION_ENTER_TRACE;
    flashingState = FlashingDeviceState;
    dataArray = data;
    qDebug()<<"Reading plugin finished";
    if(dataArray.isEmpty())
    {
        ui->progressBar->setVisible(false);
        ui->statusBar->showMessage(tr("File readin FAILED!!!"), 0);
        qCritical()<<"No data readed, read plugin faild";
    }
    else
    {
        qDebug()<<"Readed data: "<<dataArray.toHex();
        qDebug()<<"Starting flashing";
        currentFlasher->flash(dataArray);
    }
}

void MainWindow::done(bool success)
{
    FUNCTION_ENTER_TRACE;
    if(success)
        QMessageBox::information(this, tr("Flasnig completed"), tr("Flasnig completed"));
    else
        ui->statusBar->showMessage(tr("Flashing FAILED!!!"), 0);
    ui->progressBar->setVisible(false);

    flashingState = ReadingDataState;
    ui->progressBar->setValue(0);
}

void MainWindow::progressInPercentage(qint32 percentageProgress)
{
    FUNCTION_ENTER_TRACE;
    qreal percentagePerSate = 100/NumberOfStates;
    qreal percentagPhase = percentagePerSate*flashingState;
    qreal currentStateProgress = (percentagePerSate/100)*percentageProgress;
    ui->progressBar->setValue(percentagPhase+currentStateProgress);
    qDebug()<<"PROGRESS: "<<(qint32)percentagPhase+currentStateProgress<<"% !!!!!!";
}

void MainWindow::showMessageBox(QString title, QString text, qint32 type)
{
    FUNCTION_ENTER_TRACE;
    switch(type)
    {
    case MESSAGE_BOX_CRITICIAL:
        QMessageBox::critical(this, title, text);
        break;
    case MESSAGE_BOX_INFORMATION:
        QMessageBox::information(this, title, text);
        break;
    case MESSAGE_BOX_WARNING:
        QMessageBox::warning(this, title, text);
        break;
    default:
        QMessageBox::critical(this, title, text);
        break;
    }
}
