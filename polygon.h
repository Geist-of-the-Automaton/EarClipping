#ifndef POLYGON_H
#define POLYGON_H

#include <QPoint>
#include <QColor>

#include <list>
#include <vector>

using std::list;
using std::vector;
using std::min;
using std::max;

const int maxEdgeSize = 100;
static bool drawType = false;


struct Triangle {
    QPoint a, b, c;

    Triangle(QPoint A, QPoint B, QPoint C) {
        a = A;
        b = B;
        c = C;
    }

    Triangle(const Triangle &t) {
        *this = t;
    }

    Triangle& operator = (const Triangle &t) {
        a = t.a;
        b = t.b;
        c = t.c;
        return *this;
    }
};

static int sign (QPoint p1, QPoint p2, QPoint p3) {
    return (p1.x() - p3.x()) * (p2.y() - p3.y()) - (p2.x() - p3.x()) * (p1.y() - p3.y());
}

static int inTri (QPoint qp, QPoint a, QPoint b, QPoint c) {
    int d1 = sign(qp, a, b);
    int d2 = sign(qp, b, c);
    int d3 = sign(qp, c, a);
    return ((d1 >= 0) && (d2 >= 0) && (d3 >= 0)) || ((d1 <= 0) && (d2 <= 0) && (d3 <= 0));
}

static int det (QPoint a, QPoint b, QPoint c) {
    QPoint u = b - a;
    QPoint v = c - b;
    return u.x() * v.y() - u.y() * v.x();
}


class Polygon
{
public:
    Polygon();
    Polygon(const Polygon &gon);
    Polygon& operator = (const Polygon &gon);
    list <Triangle> getTris();
    vector <QPoint> getPts();
    void setEdgeSize(int size);
    int getEdgeSize();
    void setPolyColor(QRgb qc);
    void setEdgeColor(QRgb qc);
    QRgb getPolyColor();
    QRgb getEdgeColor();
    void removePt(int index);
    void addPt(QPoint qp, int index = -1);
    void movePt(QPoint loc, size_t index);
    void translate(QPoint qp);
    void scale(QPoint qp);
    void rotate(QPoint qp);
    void cleanup();
    int inPoly(QPoint qp);
    QPoint getCenter();
    void setRPt1(QPoint qp);
    void setRPt2(QPoint qp);
    void makeBackup();
    void reducePts();

   /*
    * filter
    * filter strength
    * mode
    * transparency
    */

private:
    void triangulate();

    list <Triangle> tris;
    vector <QPoint> pts, backup;
    int edgeSize;
    QRgb pColor, eColor;
    QPoint rPt1, rPt2;

};

#endif // POLYGON_H
