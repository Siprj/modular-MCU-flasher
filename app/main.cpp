#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <stdio.h>
#include <QDebug>

void initLogger()
{
    qSetMessagePattern("[%{if-debug}D%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}] %{file}:%{line} %{function} - %{message}");
    freopen("trace.log", "w", stderr);
}



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //initLogger();
    a.setWindowIcon(QIcon(":/ico/img/programicon.png"));
    MainWindow w;
    w.show();

    return a.exec();
}
