#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setFixedSize(1700, 1000);
    activePt = -1;
    form = 1;
    dragDraw = 0;
    qi = QImage(1700, 1000, QImage::Format_ARGB32_Premultiplied);
    qi.fill(0xFFFFFFFF);
    triSegFlag = false;
    top = bottom = 0xFF000000;
    dispDivs = 0;
    shiftFlag = false;
    ctrlFlag = false;
    altFlag = false;
    cPt = QPoint(qi.width() / 2, qi.height() / 2);
    gons.push_back(Polygon());
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(flashCPts()));
    timer->start(1000);
}

void MainWindow::calcTri(Triangle t) {
    QPoint a = t.a, b = t.b, c = t.c;
    if (a.y() > b.y()) {
        QPoint tmp = a;
        a = b;
        b = tmp;
    }
    if (b.y() > c.y()) {
        QPoint tmp = b;
        b = c;
        c = tmp;
        if (a.y() > b.y()) {
            tmp = a;
            a = b;
            b = tmp;
        }
    }
    if (b.y() == a.y())
        fillBTri(c, b, a);
    else if (b.y() == c.y())
        fillTTri(a, b, c);
    else {
        QPoint d (a.x() + static_cast<float>(c.x() - a.x()) * (static_cast<float>(b.y() - a.y()) / static_cast<float>(c.y() - a.y())) , b.y());
        fillBTri(c, b, d);
        fillTTri(a, b, d);
    }
}

void MainWindow::fillBTri(QPoint a, QPoint b, QPoint c) {
    if (b.x() > c.x()) {
        QPoint tmp = b;
        b = c;
        c = tmp;
    }
    float invslope1 = static_cast<float>(b.x() - a.x()) / static_cast<float>(b.y() - a.y());
    if (abs(invslope1) > len * len)
        return;
    float invslope2 = static_cast<float>(c.x() - a.x()) / static_cast<float>(c.y() - a.y());
    if (abs(invslope2) > len * len)
        return;
    float curx1 = static_cast<float>(a.x());
    float curx2 = static_cast<float>(a.x());
    int h = qi.height() - 1;
    float w = qi.width() - 1;
    float offset = a.y() > h ? a.y() - h : 0;
    curx1 -= invslope1 * offset;
    curx2 -= invslope2 * offset;
    for (int y = min(h, a.y()); y >= max(0, b.y()); --y) {
        QRgb *line = reinterpret_cast<QRgb *>(qi.scanLine(y));
        for (int x = static_cast<int>(max(curx1, 0.0f)); x <= static_cast<int>(min(curx2, w)); ++x)
            line[x] = bottom;
        curx1 -= invslope1;
        curx2 -= invslope2;
    }
}

void MainWindow::fillTTri(QPoint a, QPoint b, QPoint c) {
    if (b.x() > c.x()) {
        QPoint tmp = b;
        b = c;
        c = tmp;
    }
    float invslope1 = static_cast<float>(b.x() - a.x()) / static_cast<float>(b.y() - a.y());
    if (abs(invslope1) > len * len)
        return;
    float invslope2 = static_cast<float>(c.x() - a.x()) / static_cast<float>(c.y() - a.y());
    if (abs(invslope2) > len * len)
        return;
    float curx1 = static_cast<float>(a.x());
    float curx2 = static_cast<float>(a.x());
    float offset = a.y() < 0 ? -a.y() : 0;
    curx1 += invslope1 * offset;
    curx2 += invslope2 * offset;
    int h = qi.height();
    float w = qi.width() - 1;
    for (int y = max(0, a.y()); y < min(h, b.y()); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(qi.scanLine(y));
        for (int x = static_cast<int>(max(curx1, 0.0f)); x <= static_cast<int>(min(curx2, w)); ++x)
            line[x] = top;
        curx1 += invslope1;
        curx2 += invslope2;
    }
}

void MainWindow::createEllipse() {
    int xrad = cPt.x() - 2;
    int yrad = cPt.y() - 2;
    gons.push_back(Polygon());
    int index = gons.size() - 1;
    for (float angle = 180.0; angle > -180.0; angle -= 1.0) {
        float radAng = angle * 0.0174533;
        float s = sin(radAng);
        float c = cos(radAng);
        QPoint qp = QPoint(xrad * c + xrad, yrad * s + yrad);
        gons[index].addPt(qp);
    }
    activeGons.push_back(index);
    while (gons[index].getPts().size() - 2 != gons[index].getTris().size())
        gons[index].reducePts();
    repaint();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    QPoint qp = event->pos();
    if (altFlag) {
        if (qp.x() > 2 && qp.y() > 2)
            cPt = qp;
        repaint();
        return;
    }
    if (lastButton == Qt::LeftButton) {
        if (shiftFlag)
            for (int activeGon : activeGons)
                gons[activeGon].translate(qp);
        else if (ctrlFlag)
            for (int activeGon : activeGons)
                gons[activeGon].scale(qp);
        else if (activeGons.size() == 1) {
            if (activePt != -1)
                gons[activeGons[0]].movePt(qp, activePt);
            else if (dragDraw)
                gons[activeGons[0]].addPt(qp);
        }
    }
    else if (lastButton == Qt::RightButton) {
        if (shiftFlag)
            for (int activeGon : activeGons)
                gons[activeGon].rotate(qp);
        else if (activeGons.size() == 1) {
            int dist = (3 * ptSize) / 2;
            vector <QPoint> pts = gons[activeGons[0]].getPts();
            for (int i = 0; i < pts.size(); ++i) {
                QPoint pt = pts[i];
                if (abs(pt.x() - qp.x()) <= dist && abs(pt.y() - qp.y()) < dist) {
                    activePt = i;
                    break;
                }
            }
            if (activePt != -1) {
                gons[activeGons[0]].removePt(activePt);
                activePt = -1;
                if (pts.size() - 1 == 0) {
                    gons.erase(gons.begin() + activeGons[0]);
                    activeGons.clear();
                }
            }
        }
    }
    repaint();
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        int index = -1;
        if (activeGons.size() == 1 && gons[activeGons[0]].getPts().size() < 3)
            return;
        for (int i = 0; i < gons.size(); ++i)
            if (gons[i].inPoly(event->pos())) {
                index = i;
                break;
            }
        if (index != -1) {
            int index2 = -1;
            for (int i = 0; i < activeGons.size(); ++i)
                if (activeGons[i] == index) {
                    index2 = i;
                    break;
                }
            if (index2 == -1)
                activeGons.push_back(index);
            else
                activeGons.erase(activeGons.begin() + index2);
        }
        else
            activeGons.clear();
    }
    repaint();
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    QPoint qp = event->pos();
    if (altFlag) {
        if (qp.x() > 2 && qp.y() > 2)
            cPt = qp;
        repaint();
        return;
    }
    int dist = (3 * ptSize) / 2;
    lastButton = event->button();
    if (activeGons.size() == 1) {
        vector <QPoint> pts = gons[activeGons[0]].getPts();
        for (int i = 0; i < pts.size(); ++i) {
            QPoint pt = pts[i];
            if (abs(pt.x() - qp.x()) <= dist && abs(pt.y() - qp.y()) < dist) {
                activePt = i;
                break;
            }
        }
    }
    else if (activeGons.size() == 0 && lastButton == Qt::LeftButton) {
        activeGons.push_back(gons.size());
        gons.push_back(Polygon());
    }
    if (lastButton == Qt::LeftButton) {
        if (shiftFlag)
            for (int activeGon : activeGons)
                gons[activeGon].setRPt1(qp);
        else if (ctrlFlag && activeGons.size() >= 1) {
            QPoint corner = gons[activeGons[0]].getPts()[0];
            for (int activeGon : activeGons) {
                for (QPoint pt : gons[activeGon].getPts()) {
                    if (pt.x() < corner.x())
                        corner.setX(pt.x());
                    if (pt.y() < corner.y())
                        corner.setY(pt.y());
                }
            }
            for (int activeGon : activeGons) {
                gons[activeGon].setRPt2(qp);
                gons[activeGon].setRPt1(corner);
                gons[activeGon].makeBackup();
            }
        }
        else if (activeGons.size() == 1 && activePt == -1) {
            lastButton = Qt::LeftButton;
            gons[activeGons[0]].addPt(qp);
        }
    }
    else if (lastButton == Qt::RightButton) {
        if (shiftFlag) {
            QPoint center(0, 0);
            for (int activeGon : activeGons) {
                center += gons[activeGon].getCenter();
                gons[activeGon].makeBackup();
                gons[activeGon].setRPt2(qp);
            }
            center = QPoint(center.x() / activeGons.size(), center.y() / activeGons.size());
            for (int activeGon : activeGons)
                gons[activeGon].setRPt1(center);
        }
        else if (activePt != -1 && activeGons.size() == 1) {
            gons[activeGons[0]].removePt(activePt);
            activePt = -1;
            if (gons[activeGons[0]].getPts().size() == 0) {
                gons.erase(gons.begin() + activeGons[0]);
                activeGons.clear();
            }
        }
    }
    repaint();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    activePt = -1;
    for (int activeGon : activeGons)
        gons[activeGon].cleanup();
    repaint();
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Space:
        dragDraw = !dragDraw;
        break;
    case Qt::Key_0:
        gons.clear();
        activeGons.clear();
        activePt = -1;
        break;
    case Qt::Key_1:
        triSegFlag = !triSegFlag;
        if (triSegFlag) {
            top = 0xFFFF0000;
            bottom = 0xFF00FF00;
        }
        break;
    case Qt::Key_2:
        dispDivs = !dispDivs;
        break;
    case Qt::Key_Shift:
        if (!ctrlFlag)
            shiftFlag = true;
        break;
    case Qt::Key_Control:
        if (!shiftFlag)
            ctrlFlag = true;
        break;
    case Qt::Key_Left:
        for (int activeGon : activeGons)
            gons[activeGon].setEdgeSize(gons[activeGon].getEdgeSize() - 1);
        break;
    case Qt::Key_Right:
        for (int activeGon : activeGons)
            gons[activeGon].setEdgeSize(gons[activeGon].getEdgeSize() + 1);
        break;
    case Qt::Key_Up:
        if (activeGons.size() == 1)
            gons[activeGons[0]].setPolyColor(QColorDialog::getColor(gons[activeGons[0]].getPolyColor(), this).rgba());
        break;
    case Qt::Key_Down:
        if (activeGons.size() == 1)
            gons[activeGons[0]].setEdgeColor(QColorDialog::getColor(gons[activeGons[0]].getEdgeColor(), this).rgba());
        break;
    case Qt::Key_Minus:
        if (activeGons.size() == 1)
            gons[activeGons[0]].reducePts();
        break;
    case Qt::Key_Alt:
        if (activeGons.size() == 0)
            altFlag = true;
        break;
    case Qt::Key_C:
        copyGons();
        break;
    case Qt::Key_V:
        pasteGons();
        break;
    case Qt::Key_X:
        copyGons();
        deleteGons();
        break;
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        deleteGons();
        break;
    case Qt::Key_Period:
        drawType = !drawType;
        break;
    }
    repaint();
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Shift:
        shiftFlag = false;
        break;
    case Qt::Key_Control:
        ctrlFlag = false;
        break;
    case Qt::Key_Alt:
        altFlag = false;
        createEllipse();
    }
}

void MainWindow::copyGons() {
    if (ctrlFlag) {
        copy.clear();
        for (int activeGon : activeGons)
            copy.push_back(gons[activeGon]);
    }
}

void MainWindow::pasteGons() {
    if (ctrlFlag) {
        activeGons.clear();
        vector <int> newGons;
        for (Polygon p : copy) {
            newGons.push_back(gons.size());
            gons.push_back(p);
        }
        activeGons = newGons;
    }
}

void MainWindow::deleteGons() {
    std::sort(activeGons.begin(), activeGons.end());
    vector <int> gonNums = activeGons;
    activeGons.clear();
    for (int i = gonNums.size() - 1; i >= 0; --i)
        gons.erase(gons.begin() + gonNums[i]);
}

void MainWindow::flashCPts() {
    form = !form;
    repaint();
}

void MainWindow::paintEvent(QPaintEvent *event) {
    long long l = getTime();
    int w = qi.width(), h = qi.height();
    qi.fill(0xFFFFFFFF);
    QPainter qp(&qi);
    int index = 0;
    for (Polygon gon : gons) {
        list <Triangle> tris = gon.getTris();
        vector <QPoint> pts = gon.getPts();
        if (!triSegFlag)
            bottom = top = gon.getPolyColor();
        for (Triangle &t : tris)
            calcTri(t);
        if (dispDivs) {
            QColor lineColor;
            int size;
            if (top == bottom) {
                lineColor = QColor(200, 200, 200);
                size = 1;
            }
            else {
                lineColor = QColor(20, 20, 20);
                size = 2;
            }
            qp.setPen(QPen(lineColor, size, Qt::SolidLine, Qt::RoundCap));
            for (Triangle &t : tris) {
                qp.drawLine(t.a, t.b);
                qp.drawLine(t.a, t.c);
                qp.drawLine(t.c, t.b);
            }
        }
        else {
            int edgeSize = gon.getEdgeSize();
            qp.setPen(QPen(QColor(gon.getEdgeColor()), edgeSize, Qt::SolidLine, Qt::RoundCap));
            if (edgeSize != 0 && pts.size() >= 2) {
                QVector <QLine> lines;
                for (int i = 1; i < pts.size(); ++i)
                    lines.push_back(QLine(pts[i - 1], pts[i]));
                lines.push_back(QLine(pts[0], pts[pts.size() - 1]));
                qp.drawLines(lines);
            }
        }
        if (std::find(activeGons.begin(), activeGons.end(), index) != activeGons.end()) {
            for (QPoint pt : pts) {
                for (int j = max(0, pt.y() - ptSize); j <= min(pt.y() + ptSize, h - 1); ++j) {
                    QRgb *line = reinterpret_cast<QRgb *>(qi.scanLine(j));
                    for (int i = max(pt.x() - ptSize, 0); i <= min(pt.x() + ptSize, w - 1); ++i) {
                        int dist = abs(i - pt.x()) + abs(j - pt.y());
                        if ((form || dist >= ptSize - 1) && dist <= ptSize) {
                            QColor qc(line[i]);
                            line[i] = QColor(255 - qc.red(), 255 - qc.green(), 255 - qc.blue()).rgba();
                        }
                    }
                }
            }
        }
        ++index;
    }
    qp.end();
    qp.begin(this);
    qp.drawImage(0, 0, qi);
    if (altFlag) {
        qp.setPen(QPen(QColor(128, 128, 128), 5));
        qp.drawEllipse(cPt, cPt.x() - 2, cPt.y() - 2);
    }
    cout << getTime(l) << endl;
}

MainWindow::~MainWindow()
{
    timer->stop();
    delete timer;
    delete ui;
}
