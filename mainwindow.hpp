#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "gerberfileparser.hpp"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionOpen_File_triggered();

    void on_actionClose_Current_File_triggered();

    void on_actionZoom_In_triggered();

    void on_actionZoom_Out_triggered();

    void on_actionZoom_to_Fit_triggered();

    void on_actionDebug_toggled(bool arg1);

signals:
    void onGerberFileParsed(GerberLayer layer);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_HPP
