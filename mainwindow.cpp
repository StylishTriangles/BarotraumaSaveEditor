#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    gse = new GameSessionEditor(ui->centralWidget);
    this->setCentralWidget(gse);
//    gse->setFocus();
    QFile styleSheet(":/style/stylesheet.css");
    if (styleSheet.open(QFile::ReadOnly))
        this->setStyleSheet(styleSheet.readAll());
    else {
        throw 0;
    }
    // connect toolbar buttons with editor widget(s)
    connect(gse, SIGNAL(sessionLoaded(bool)), ui->actionSave, SLOT(setEnabled(bool)));
    connect(ui->actionSave, SIGNAL(triggered()), gse, SLOT(saveFile()));
    connect(ui->actionOpen_saved_game, SIGNAL(triggered()), gse, SLOT(openFile()));
}

MainWindow::~MainWindow()
{
    delete gse;
    delete ui;
}
