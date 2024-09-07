#include "mainwindow.hpp"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QPoint>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "GerberVisualizer_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    QPoint offset(-166,-176);
    QPoint newCenter(212,264);
    QPoint oldCenter(399,286);
    double zoom = 1;

    qInfo() << "a" << (newCenter - oldCenter) / zoom;
    offset -= (newCenter - oldCenter) / zoom;
    qInfo() << "b" << offset;

    MainWindow w;
    w.show();
    return a.exec();
}
