#ifndef GERBERFILEPARSER_HPP
#define GERBERFILEPARSER_HPP

#include <list>
#include <sstream>
#include <QPainter>

enum GerberNodeType {
    Vector = 0,
    Pad = 1
};

class GerberPoint2D {
private:
    double x;
    double y;
public:
    GerberPoint2D(double x, double y) {
        this->x = x;
        this->y = y;
    }

    double getX() const {
        return x;
    }

    double getY() const {
        return y;
    }

    std::string toString() const {
        std::stringstream ss;
        ss << "(" << this->x << ", " << this->y << ")";
        return ss.str();
    }
};

class GerberNode {
private:
    bool isLpd;
    char shape;

    GerberNodeType nodeType;

public:
    GerberNode(char shape, bool isLpd, GerberNodeType nodeType): nodeType(nodeType) {
        setIsLpd(isLpd);
        setShape(shape);
    }

    GerberNodeType getNodeType() {
        return nodeType;
    }

    void setIsLpd(bool isLpd) {
        this->isLpd = isLpd;
    }

    bool getIsLpd() const {
        return isLpd;
    }

    void setShape(char shape) {
        this->shape = shape;
    }

    char getShape() const {
        return shape;
    }

    virtual std::string toString() const {
        std::stringstream ss;
        ss << "shape: " << this->getShape()
           << ", isLpd: " << this->getIsLpd()
           << ", nodeType: " << this->nodeType;
        return ss.str();
    }
};

class GerberPad : public GerberNode {
private:
    GerberPoint2D startPoint;
    double _width;
    double _height;

public:
    GerberPad(GerberPoint2D startPoint, double width, double height, char shape, bool isLpd)
        : GerberNode(shape, isLpd, Pad), startPoint(startPoint) {
        this->_width = width;
        this->_height = height;
    }

    GerberPoint2D getStartPoint() const {
        return startPoint;
    }

    double width() const {
        return _width;
    }

    double height() const {
        return _height;
    }

    std::string toString() const override {
        std::stringstream ss;
        ss << "startPoint: " << startPoint.toString()
           << ", width: " << _width
           << ", height: " << _height
           << ", " << GerberNode::toString();

        return ss.str();
    }
};


class GerberVector : public GerberNode {

private:
    GerberPoint2D startPoint;
    GerberPoint2D endPoint;
    double _width;

public:
    GerberVector(GerberPoint2D startPoint, GerberPoint2D endPoint, double width, char shape, bool isLpd)
        : GerberNode(shape, isLpd, Vector), startPoint(startPoint), endPoint(endPoint) {
        this->_width = width;
    }

    GerberPoint2D& getStartPoint() {
        return startPoint;
    }

    GerberPoint2D& getEndPoint() {
        return endPoint;
    }

    double width() const {
        return this->_width;
    }

    std::string toString() const override {
        std::stringstream ss;
        ss << "startPoint: " << startPoint.toString()
           << ", endPoint: " << endPoint.toString()
           << ", width: " << width()
           << ", " << GerberNode::toString();

        return ss.str();
    }
};

class GerberLayer {
private:
    std::list<std::shared_ptr<GerberNode>> nodes;
    std::string _name;

    double _maxX = -1000;
    double _maxY = -1000;
    double _minX = 1000;
    double _minY = 1000;

public:

    std::string name() {
        return _name;
    }

    void setName(std::string name) {
        _name = name;
    }

    std::list<std::shared_ptr<GerberNode>> getNodes() {
        return this->nodes;
    }

    void clear() {
        this->nodes.clear();
    }

    void appendNode(GerberPad pad) {
        this->nodes.emplace_back(std::make_shared<GerberPad>(pad));
    }

    void appendNode(GerberVector vector) {
        this->nodes.emplace_back(std::make_shared<GerberVector>(vector));
    }

    void calculateMinMax(double x, double y) {
        if (x >= _maxX){
            _maxX = x;
        }

        if (x <= _minX) {
            _minX = x;
        }


        if (y >= _maxY) {
            _maxY = y;
        }

        if (y <= _minY) {
            _minY = y;
        }
    }

    double cx() {
        return (_maxX + _minX) / 2;
    }

    double cy() {
        return (_maxY + _minY) / 2;
    }

    double height() {
        return abs(_maxY - _minY);
    }

    double width() {
        return abs(_maxX - _minX);
    }

    double maxX() {
        return _maxX;
    }

    double minX() {
        return _minX;
    }

    double maxY() {
        return _maxY;
    }

    double minY() {
        return _minY;
    }

    std::string toString() {
        std::stringstream ss;
        ss << "maxX:" << _maxX << "minX:" << _minX << "\n";
        ss << "maxY:" << _maxY << "minY:" << _minY << "\n";

        return ss.str();
    }
};

class GerberFileParser
{
public:
    GerberFileParser();

    GerberLayer parse(const std::string &fileContent);
};

#endif // GERBERFILEPARSER_HPP
