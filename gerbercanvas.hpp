#ifndef GERBERCANVAS_H
#define GERBERCANVAS_H

#include "gerberfileparser.hpp"

#include <QWidget>
#include <QRubberBand>
#include <unordered_set>


namespace Ui {


class GerberRubberBand {
private:
    std::shared_ptr<QRubberBand> _rubberband;
    QPoint _origin;
public:

    void start(QPoint origin, QWidget *parent) {
        _origin = origin;
        _rubberband = std::make_shared<QRubberBand>(QRubberBand::Rectangle, parent);
        _rubberband->setGeometry(QRect(_origin, QSize()));
        _rubberband->show();
    }

    void move(QPoint position) {
        _rubberband->setGeometry(QRect(_origin, position).normalized());
    }

    void end() {
        _rubberband->hide();
        _rubberband->close();

        _rubberband = nullptr;
        _origin = QPoint(0, 0);
    }

    bool hasStarted() {
        return _rubberband != nullptr;
    }

    QPoint origin() {
        return _origin;
    }

};

class GerberCanvas;
}

class GerberCanvas : public QWidget
{
    Q_OBJECT

public:
    explicit GerberCanvas(QWidget *parent = nullptr);
    ~GerberCanvas();

    void paintEvent(QPaintEvent *e);
    void wheelEvent(QWheelEvent *e);
    void resizeEvent(QResizeEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);

    void zoomIn();
    void zoomOut();
    void zoomToFit();

    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();

    void repaint(bool paintCached = false) {
        shouldPaintCached = paintCached;
        update();
    }

public slots:
    void onLayerChanged(GerberLayer newLayer) {
        layer = newLayer;
        offset = QPoint(0, 0);
        repaint();
    }

    void setDebug(bool debug) {
        isDebug = debug;
        repaint();
    }

private:
    Ui::GerberCanvas *ui;
    GerberLayer layer;
    QImage cached;
    Ui::GerberRubberBand rubberband;

    std::unordered_set<int> pressedKey;

    bool shouldPaintCached = false;
    bool isDebug = false;
    double zoom = 1;
    QPoint offset;

    void paintNodes();
    void paintCached();
    void paintVector(QPainter &painter, GerberVector &vector);
    void paintPad(QPainter &painter, GerberPad &pad);

    double defaultScale() {
        return ((layer.height() / layer.width()) > (height() / width())) ?
                   height() / layer.height()  : width() / layer.width();
    }
};

#endif // GERBERCANVAS_H
