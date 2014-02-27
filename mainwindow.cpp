#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGridLayout>
#include <QWidget>
#include <QLabel>

#include <QFileDialog>

#include <QDebug>




MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qDebug()<<"STARTTING DEBUG";
    qDebug()<<"STARTTING DEBUG";
    qDebug()<<"STARTTING DEBUG";

    settingDialog = new SettingsDialog;

    connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

//    QWidget *widget = new QWidget;
//    QLabel *label = new QLabel("blabla");
//    QGridLayout *grid = new QGridLayout;
//    grid->addWidget(label);
//    widget->setLayout(grid);
//    ui->statusBar->addPermanentWidget(widget);

//    QWidget *widget2 = new QWidget;
//    QLabel *label2 = new QLabel("blabla 22222");
//    QGridLayout *grid2 = new QGridLayout;
//    grid2->addWidget(label2);
//    widget2->setLayout(grid2);
//    ui->statusBar->addPermanentWidget(widget2);
//    ui->statusBar->showMessage("blablaasasdfasdf ");
    connect(&dmxBootProtocol, SIGNAL(printProgressInfo(QString)), this, SLOT(printDebugInfo(QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::printDebugInfo(QString text)
{
    ui->debugInfoTextEdit->appendPlainText(text+"\n");
}

void MainWindow::on_actionOpen_File_triggered()
{
    QString name = QFileDialog::getOpenFileName(this, "Hex File", "Z:\\Yrid\\elektro\\ATmega8 soutez adc\\ATmega8 soutez adc\\Debug", tr("Hex (*.hex)"));

    if(reader.isOpen())
        reader.close();
    if(!name.isEmpty())
    {
        if(reader.open(name.toStdString().c_str()))
        {
            qDebug()<<"File is opened";
        }



        readerSize = reader.readHex();
        array.clear();
        char pole[500000];
        reader.getData(0, qMin((quint32)50000, readerSize), pole);
        array.append(pole, readerSize);
    }
}

void MainWindow::on_pushButton_clicked()
{
    settingDialog->show();
}

void MainWindow::on_bootPushButton_clicked()
{
//    dmxBootProtocol.open(settingDialog);

}

void MainWindow::on_pushButton_2_clicked()
{
    dmxBootProtocol.startBootSequence(settingDialog->settings().name, 0xE0, array/*QByteArray("Asdfasdfasdfasdf")*/);
//    dmxBootProtocol.send();
}
