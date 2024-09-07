#include "gerberfileparser.hpp"
#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QFileDialog>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(this, SIGNAL(onGerberFileParsed(GerberLayer)), ui->gerberCanvas, SLOT(onLayerChanged(GerberLayer)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_File_triggered()
{
    auto fileTypes = tr("Gerber Files (*.gbr *.BAK)");
    QFileDialog::getOpenFileContent(fileTypes, [this](const QString &fileName, const QByteArray &fileContent) {
        if (fileName.isEmpty()) {
            return;
        }

        qInfo() << fileName.toStdString();
        // qInfo() << fileContent.toStdString();

        GerberFileParser parser;
        auto layer = parser.parse(fileContent.toStdString());
        layer.setName(fileName.toStdString());
        emit onGerberFileParsed(layer);
    });
}


void MainWindow::on_actionClose_Current_File_triggered()
{

}


void MainWindow::on_actionZoom_In_triggered()
{
    ui->gerberCanvas->zoomIn();
}


void MainWindow::on_actionZoom_Out_triggered()
{
    ui->gerberCanvas->zoomOut();
}


void MainWindow::on_actionZoom_to_Fit_triggered()
{
    ui->gerberCanvas->zoomToFit();
}


void MainWindow::on_actionDebug_toggled(bool debug)
{
    ui->gerberCanvas->setDebug(debug);
}

