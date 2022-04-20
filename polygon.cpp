#include "polygon.h"

Polygon::Polygon() {
    edgeSize = 0;
}

list <Triangle> Polygon::getTris() {
    return tris;
}

vector <QPoint> Polygon::getPts() {
    return pts;
}

void Polygon::setEdgeSize(int size) {
    if (size >= 0 && size <= maxEdgeSize)
        edgeSize = size;
}

int Polygon::getEdgeSize() {
    return edgeSize;
}

void Polygon::setPolyColor(QRgb qc) {
    pColor = qc;
}

void Polygon::setEdgeColor(QRgb qc) {
    eColor = qc;
}

QRgb Polygon::getPolyColor() {
    return pColor;
}

QRgb Polygon::getEdgeColor() {
    return eColor;
}

void Polygon::triangulate() {
    vector <QPoint> pts2 = pts;
    vector <QPoint> processed;
    int isCCW = -1;
    int ccw = 0, cw = 0;
    for (int i = 0; i < pts2.size(); ++i) {
        int i1 = (i + 1) % pts2.size();
        int i2 = (i + 2) % pts2.size();
        int d = det(pts2[i], pts2[i1], pts2[i2]);
        if (d < 0)
            ++ccw;
        else if (d > 0)
            ++cw;
    }
    isCCW = ccw > cw ? 1 : 0;
    tris.clear();
    while (pts2.size() >= 3) {
        int size = tris.size();
        for (int i = 0; i < pts2.size(); ++i) {
            int flag = 1;
            int i1 = (i + 1) % pts2.size();
            int i2 = (i + 2) % pts2.size();
            if ((det(pts2[i], pts2[i1], pts2[i2]) < 0 && isCCW) || (det(pts2[i], pts2[i1], pts2[i2]) > 0 && !isCCW)) {
                if (i2 < i) {
                    for (int j = i2 + 1; j < i; ++j)
                        if (inTri(pts2[j], pts2[i], pts2[i1], pts2[i2])) {
                            flag = 0;
                            break;
                        }
                }
                else {
                    for (int j = 0; j < i; ++j)
                        if (inTri(pts2[j], pts2[i], pts2[i1], pts2[i2])) {
                            flag = 0;
                            break;
                        }
                    if (flag)
                        for (int j = i2 + 1; j < pts2.size(); ++j)
                            if (inTri(pts2[j], pts2[i], pts2[i1], pts2[i2])) {
                                flag = 0;
                                break;
                            }
                }
                if (flag)
                    for (QPoint qp : processed)
                        if (inTri(qp, pts2[i], pts2[i1], pts2[i2])) {
                            flag = 0;
                            break;
                        }
                if (flag) {
                    tris.push_back(Triangle(pts2[i], pts2[i1], pts2[i2]));
                    processed.push_back(pts2[i1]);
                    pts2.erase(pts2.begin() + i1);
                    break;
                }
            }
        }
        if (tris.size() == size)
            break;
    }
}


