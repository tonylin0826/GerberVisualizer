#include "gerbercanvas.hpp"
#include "ui_gerbercanvas.h"

#include <QPainter>
#include <QWheelEvent>


GerberCanvas::GerberCanvas(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GerberCanvas)
    , offset(QPoint(0, 0))
{
    ui->setupUi(this);
    setMouseTracking(true);
}

GerberCanvas::~GerberCanvas()
{
    delete ui;
}

void GerberCanvas::paintEvent(QPaintEvent *e) {
    Q_UNUSED(e);
    if (layer.name().empty()) {
        return;
    }

    if (shouldPaintCached) {
        paintCached();
    } else {
        paintNodes();
    }


    if (isDebug) {
        QPainter painter(this);
        painter.setBrush(QBrush(Qt::cyan));
        painter.drawEllipse(rect().center(), 2, 2);
    }
}

void GerberCanvas::wheelEvent(QWheelEvent *e) {
    Q_UNUSED(e);
}

void GerberCanvas::resizeEvent(QResizeEvent *e) {
    Q_UNUSED(e);
    repaint(false);
}

void GerberCanvas::mousePressEvent(QMouseEvent *e) {
    if (rubberband.hasStarted()) {
        return;
    }

    rubberband.start(e->pos(), this);
}

void GerberCanvas::mouseMoveEvent(QMouseEvent *e) {
    if (rubberband.hasStarted()) {
        rubberband.move(e->pos());
    }

    if (pressedKey.find(Qt::Key_Meta) != pressedKey.end()) {

    }

}

void GerberCanvas::mouseReleaseEvent(QMouseEvent *e) {
    if (!rubberband.hasStarted()) {
        return;
    }

    // Calculate the selected rectangle
    auto rect = QRect(rubberband.origin(), e->pos()).normalized();

    // Ensure the rectangle has a minimum size to avoid extreme zooming
    if (rect.width() < 5 || rect.height() < 5) {
        rubberband.end();
        return;
    }

    // Calculate the zoom factor based on the selected rectangle and canvas
    double newZoomX = width() / static_cast<double>(rect.width());
    double newZoomY = height() / static_cast<double>(rect.height());

    // Choose the smaller zoom factor to maintain the aspect ratio
    double newZoom = std::min(newZoomX, newZoomY);

    zoom *= newZoom;

    offset *= newZoom;
    offset += (rect.topLeft() * newZoom);

    // Reset the rubberband and trigger a repaint
    rubberband.end();
    repaint(false);
}

void GerberCanvas::keyPressEvent(QKeyEvent *e) {
    pressedKey.emplace(e->key());

    if (e->key() == Qt::Key_Up) {
        moveUp();
        return;
    }


    if (e->key() == Qt::Key_Down) {
        moveDown();
        return;
    }


    if (e->key() == Qt::Key_Left) {
        moveLeft();
        return;
    }


    if (e->key() == Qt::Key_Right) {
        moveRight();
        return;
    }
}

void GerberCanvas::keyReleaseEvent(QKeyEvent *e) {
    pressedKey.erase(e->key());
}

void GerberCanvas::zoomIn() {
    double factor = 1.2;
    zoom *= factor;
    offset += (offset + QPoint(width() / 2, height() / 2)) * (factor - 1);

    repaint();
}

void GerberCanvas::zoomOut() {

    double factor = 1.2;
    zoom /= factor;

    auto center = (offset + QPoint(width() / 2, height() / 2));

    offset -= center - (center / factor);
    repaint();
}

void GerberCanvas::zoomToFit() {
    zoom = 1;
    offset = QPoint(0, 0);
    repaint();
}

void GerberCanvas::moveLeft() {
    offset.setX(offset.x() - 30);
    repaint(true);
}

void GerberCanvas::moveRight() {
    offset.setX(offset.x() + 30);
    repaint(true);
}

void GerberCanvas::moveUp() {
    offset.setY(offset.y() - 30);
    repaint(true);
}

void GerberCanvas::moveDown() {
    offset.setY(offset.y() + 30);
    repaint(true);
}

void GerberCanvas::paintNodes() {
    cached = QImage(width() * zoom, height() * zoom, QImage::Format_ARGB32);

    QPainter painter(&cached);
    painter.setBrush(QBrush(Qt::black, Qt::SolidPattern));
    painter.drawRect(cached.rect());

    for (auto &node : layer.getNodes()) {
        if (node->getNodeType() == Vector) {
            paintVector(painter, *(std::dynamic_pointer_cast<GerberVector>(node)));
            continue;
        }

        if (node->getNodeType() == Pad) {
            paintPad(painter, *(std::dynamic_pointer_cast<GerberPad>(node)));
            continue;
        }
    }

    paintCached();
    shouldPaintCached = true;
}

void GerberCanvas::paintCached() {
    QPainter(this).drawImage(
        0, 0,
        cached,
        offset.x(), offset.y(), width(), height());
}

void GerberCanvas::paintVector(QPainter &painter, GerberVector &vector) {
    auto &startPoint = vector.getStartPoint();
    auto &endPoint = vector.getEndPoint();

    double width = painter.device()->width();
    double height = painter.device()->height();

    double scale = defaultScale() * zoom;

    int x1 = round(width / 2.0 + (startPoint.getX() - layer.cx()) * scale);
    int x2 = round(width / 2.0 + (endPoint.getX() - layer.cx()) * scale);
    int y1 = round(height / 2.0 - (startPoint.getY() - layer.cy()) * scale);
    int y2 = round(height / 2.0 - (endPoint.getY() - layer.cy()) * scale);

    // int x1 = (startPoint.getX() - layer.minX()) * scale;
    // int x2 = (endPoint.getX() - layer.minX()) * scale;
    // int y1 = (startPoint.getY() - layer.minY()) * scale;
    // int y2 = (endPoint.getY() - layer.minY()) * scale;

    int w = round(vector.width() * scale);

    QPen pen(Qt::SolidLine);
    pen.setWidth(w);

    if (vector.getShape() == 'C') {
        pen.setCapStyle(Qt::RoundCap);
    } else {
        pen.setCapStyle(Qt::SquareCap);
    }

    pen.setColor(vector.getIsLpd() ? Qt::red : Qt::black);
    painter.setPen(pen);
    painter.drawLine(x1, y1, x2, y2);
}

void GerberCanvas::paintPad(QPainter &painter, GerberPad &pad) {
    auto startPoint = pad.getStartPoint();
    auto rcx = layer.cx();
    auto rcy = layer.cy();

    double width = painter.device()->width();
    double height = painter.device()->height();

    double scale = defaultScale() * zoom;

    int sx = round(width / 2.0 + (startPoint.getX() - rcx) * scale);
    int sy = round(height / 2.0 - (startPoint.getY() - rcy) * scale);

    // int sx = (startPoint.getX() - rcx) * scale;
    // int sy = (startPoint.getY() - rcy) * scale;

    int w = round(pad.width() * scale);
    int h = round(pad.height() * scale);
    int x = round(sx - w / 2.0);
    int y = round(sy - h / 2.0);

    painter.setBrush(QBrush(Qt::red, Qt::SolidPattern));
    painter.setPen(QPen(Qt::NoPen));

    if (pad.getIsLpd()) {
        QBrush brush(Qt::red, Qt::SolidPattern);
        painter.setBrush(brush);

        if (pad.getShape() == 'C') {
            if (w < 2) {
                painter.drawPoint(sx, sy);
            } else {
                painter.drawEllipse(sx - w / 2, sy - w / 2, w, w);
            }
            return;
        }

        if (pad.getShape() == 'R') {
            painter.drawRect(x, y, w, h);
            return;
        }

        if (pad.getShape() == 'O') {
            QPen newPen(Qt::SolidLine);
            newPen.setColor(Qt::red);
            newPen.setCapStyle(Qt::RoundCap);

            if (h > w) {
                newPen.setWidth(w);
                painter.setPen(newPen);
                painter.drawLine(sx, sy - h / 2, sx, sy + h / 2);
            } else {
                newPen.setWidth(h);
                painter.setPen(newPen);
                painter.drawLine(sx - w / 2, sy, sx + w / 2, sy);
            }
            return;
        }
    } else {
        QBrush brush(Qt::black, Qt::SolidPattern);
        painter.setBrush(brush);

        if (pad.getShape() == 'C') {
            if (w < 2) {
                painter.drawPoint(sx, sy);
            } else {
                painter.drawEllipse(sx - w / 2, sy - w / 2, w, w);
            }
            return;
        }

        if (pad.getShape() == 'R') {
            painter.drawRect(x, y, w, h);
            return;
        }

        if (pad.getShape() == 'O') {
            QPen newPen(Qt::SolidLine);
            newPen.setColor(Qt::red);

            if (h > w) {
                newPen.setWidth(w);
                painter.setPen(newPen);
                painter.drawLine(sx, sy - h / 2, sx, sy + h / 2);
            } else {
                newPen.setWidth(h);
                painter.setPen(newPen);
                painter.drawLine(sx - w / 2, sy, sx + w / 2, sy);
            }
            return;
        }
    }
}
