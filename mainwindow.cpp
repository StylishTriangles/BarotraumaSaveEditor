#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->editorWidget, SIGNAL(sessionLoaded(bool)), ui->subTab, SLOT(setEnabled(bool)));
    connect(ui->editorWidget, SIGNAL(sessionLoaded(bool)), ui->actionSave, SLOT(setEnabled(bool)));
}

MainWindow::~MainWindow()
{
    delete ui;
}
