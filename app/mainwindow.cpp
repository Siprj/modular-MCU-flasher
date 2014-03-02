#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGridLayout>
#include <QWidget>
#include <QLabel>

#include <QFileDialog>

#include <QDebug>
#include <QPluginLoader>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->progressBar->setVisible(false);

    loadPlugins();

    currentReader = NULL;
    currentFlasher = NULL;

    flashingState = ReadingDataState;

    connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui->actionProgram, SIGNAL(triggered()), this, SLOT(on_flashButton_clicked()));
    connect(ui->openFilePushButton, SIGNAL(clicked()), this, SLOT(on_actionOpen_File_triggered()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::printProgressInfo(QString text)
{
    qDebug()<<"Printing progress info message i tu status bar: "<<text;
    ui->statusBar->showMessage(text, 10000);
}

void MainWindow::on_actionOpen_File_triggered()
{
    currentReader = NULL;
    fileToRead = QFileDialog::getOpenFileName(this, "Hex File", "Z:\\Yrid\\elektro\\ATmega8 soutez adc\\ATmega8 soutez adc\\Debug", suffixesFilters);
    qDebug()<<"File to open"<<fileToRead;
    QFileInfo fileInfo(fileToRead);
    ui->openFileLineEdit->setText(fileToRead);
    if(readersSuffixesMap.contains(fileInfo.suffix()))
    {
        currentReader = qobject_cast<ReaderPluginInterface*>(readersSuffixesMap[fileInfo.suffix()]);
    }
    else
        unknownFileSuffixMssage();

    qDebug()<<"currentReader: "<<currentReader;
}

void MainWindow::loadPlugins()
{
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
    FlasherPluginInterface *flasherPlugin = qobject_cast<FlasherPluginInterface*>(plugin);
    if(flasherPlugin)
    {
        connect(flasherPlugin, SIGNAL(done(bool)), this, SLOT(done(bool)), Qt::QueuedConnection);
        connect(flasherPlugin, SIGNAL(printProgressInfo(QString)), this, SLOT(printProgressInfo(QString)), Qt::QueuedConnection);
        connect(flasherPlugin, SIGNAL(progressInPercentage(qint32)), this, SLOT(progressInPercentage(qint32)), Qt::QueuedConnection);
        QWidget *widget = flasherPlugin->getPluginWidget();
        ui->flashingTabWidget->addTab(widget, widget->windowTitle());
        flasherMap[widget] = plugin;
    }

    ReaderPluginInterface *readerPlugin = qobject_cast<ReaderPluginInterface*>(plugin);
    if(readerPlugin)
    {
        connect(readerPlugin, SIGNAL(done(QByteArray)), this, SLOT(done(QByteArray)), Qt::QueuedConnection);
        connect(readerPlugin, SIGNAL(printProgressInfo(QString)), this, SLOT(printProgressInfo(QString)), Qt::QueuedConnection);
        connect(readerPlugin, SIGNAL(progressInPercentage(qint32)), this, SLOT(progressInPercentage(qint32)), Qt::QueuedConnection);
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
    QMessageBox::warning(this, tr("Unknown file format"), tr("Unknown file format"));
}

void MainWindow::noFileSelected()
{
    QMessageBox::warning(this, tr("No file selected"), tr("You mast select some file first"));
}

void MainWindow::on_flashButton_clicked()
{
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
    flashingState = FlashingDeviceState;
    dataArray = data;
    qDebug()<<"Reading plugin finished";
    if(dataArray.isEmpty())
    {
        ui->progressBar->setVisible(false);
        QMessageBox::warning(this, "File Read Error", "Couldn't read file. Prabelby wrong format");
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
    if(success)
        QMessageBox::information(this, tr("Flasnig completed"), tr("Flasnig completed"));
    else
    {
        QMessageBox::warning(this, tr("Flashing failed"), tr("Flashing faild"));
    }
    ui->progressBar->setVisible(false);

    flashingState = ReadingDataState;
}

void MainWindow::progressInPercentage(qint32 percentageProgress)
{
    qreal percentagePerSate = 100/NumberOfStates;
    qreal percentagPase = percentagePerSate*flashingState;
    qreal currentStateProgress = (percentagePerSate/100)*percentageProgress;
    ui->progressBar->setValue(percentagPase+currentStateProgress);
    qDebug()<<"PROGRESS: "<<(qint32)percentagPase+currentStateProgress<<"% !!!!!!";
}
