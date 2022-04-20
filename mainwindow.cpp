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
    top = bottom = 0xFF000000;
    dispDivs = 0;
    edgeSize = 4;
    shiftFlag = false;
    ctrlFlag = false;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(flashCPts()));
    timer->start(1000);
}

void MainWindow::triangulate() {
    long long l = getTime();
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
    //cout << getTime(l) << endl;
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

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    QPoint qp = event->pos();
    if (lastButton == Qt::LeftButton) {
        if (shiftFlag) {
            QPoint change = qp - rPt1;
            for (int i = 0; i < pts.size(); ++i)
                pts[i] += change;
            rPt1 = qp;
        }
        else if (ctrlFlag) {
            pts = backup;
            for (int i = 0; i < pts.size(); ++i)
                pts[i] -= rPt1;
            if (rPt2.x() == 0 || rPt2.y() == 0)
                return;
            float xChange = static_cast<float>(qp.x()) / static_cast<float>(rPt2.x());
            float yChange = static_cast<float>(qp.y()) / static_cast<float>(rPt2.y());
            for (int i = 0; i < pts.size(); ++i)
                pts[i] = QPoint(static_cast<float>(pts[i].x()) * xChange, static_cast<float>(pts[i].y()) * yChange) + rPt1;
        }
        else if (activePt != -1)
            pts[activePt] = qp;
        else if (dragDraw)
            pts.push_back(qp);
    }
    else if (lastButton == Qt::RightButton) {
        if (shiftFlag) {
            pts = backup;
            float angle = -(atan2(rPt2.y() - rPt1.y(), rPt2.x() - rPt1.x()) - atan2(qp.y() - rPt1.y(), qp.x() - rPt1.x()));
            float s = sin(angle);
            float c = cos(angle);
            for (int i = 0; i < pts.size(); ++i) {
                pts[i] -= rPt1;
                pts[i] = QPoint(pts[i].x() * c - pts[i].y() * s, pts[i].x() * s + pts[i].y() * c);
                pts[i] += rPt1;
            }
        }
        else {
            int dist = (3 * ptSize) / 2;
            for (int i = 0; i < pts.size(); ++i) {
                QPoint pt = pts[i];
                if (abs(pt.x() - qp.x()) <= dist && abs(pt.y() - qp.y()) < dist) {
                    activePt = i;
                    break;
                }
            }
            if (activePt != -1) {
                pts.erase(pts.begin() + activePt);
                activePt = -1;
            }
        }
    }
    triangulate();
    repaint();
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    QPoint qp = event->pos();
    int dist = (3 * ptSize) / 2;
    lastButton = event->button();
    for (int i = 0; i < pts.size(); ++i) {
        QPoint pt = pts[i];
        if (abs(pt.x() - qp.x()) <= dist && abs(pt.y() - qp.y()) < dist) {
            activePt = i;
            break;
        }
    }
    if (lastButton == Qt::LeftButton) {
        if (shiftFlag)
            rPt1 = qp;
        else if (ctrlFlag && pts.size() > 0) {
            rPt1 = pts[0];
            for (QPoint pt : pts) {
                if (pt.x() < rPt1.x())
                    rPt1.setX(pt.x());
                else if (pt.y() < rPt1.y())
                    rPt1.setY(pt.y());
            }
            rPt2 = qp;
            backup = pts;
        }
        else {
            if (activePt == -1) {
                if (!dragDraw)
                    activePt = static_cast<int>(pts.size());
                pts.push_back(qp);
            }
            lastButton = Qt::LeftButton;
        }
    }
    else if (lastButton == Qt::RightButton) {
        if (shiftFlag) {
            int x = 0, y = 0;
            for (QPoint pt : pts) {
                x += pt.x();
                y += pt.y();
            }
            x /= pts.size();
            y /= pts.size();
            backup = pts;
            rPt1 = QPoint(x, y);
            rPt2 = qp;
        }
        else if (activePt != -1) {
            pts.erase(pts.begin() + activePt);
            activePt = -1;
        }
    }
    triangulate();
    repaint();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    activePt = -1;
    repaint();
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Space:
        dragDraw = !dragDraw;
        break;
    case Qt::Key_0:
        tris.clear();
        pts.clear();
        break;
    case Qt::Key_1:
        if (top == 0xFF222222) {
            top = 0xFFFF0000;
            bottom = 0xFF00FF00;
        }
        else
            top = bottom = 0xFF222222;
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
        if (edgeSize > 0)
            --edgeSize;
        break;
    case Qt::Key_Right:
        if (edgeSize < 100)
            ++edgeSize;
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
    }
}

void MainWindow::flashCPts() {
    form = !form;
    repaint();
}

void MainWindow::paintEvent(QPaintEvent *event) {
    long long l = getTime();
    int w = qi.width(), h = qi.height();
    qi.fill(0xFFFFFFFF);
    for (Triangle t : tris)
        calcTri(t);
    QPainter qp(&qi);
    qp.drawImage(0, 0, qi);
    if (dispDivs) {
        qp.setPen(QPen(QColor(128, 128, 128), 4, Qt::SolidLine, Qt::RoundCap));
        for (Triangle t : tris) {
            qp.drawLine(t.a, t.b);
            qp.drawLine(t.a, t.c);
            qp.drawLine(t.c, t.b);
        }
    }
    qp.setPen(QPen(QColor(128, 128, 128), edgeSize, Qt::SolidLine, Qt::RoundCap));
    if (edgeSize != 0 && pts.size() >= 2) {
        QVector <QLine> lines;
        for (int i = 1; i < pts.size(); ++i)
            lines.push_back(QLine(pts[i - 1], pts[i]));
        lines.push_back(QLine(pts[0], pts[pts.size() - 1]));
        qp.drawLines(lines);
    }
    for (QPoint qp : pts) {
        for (int j = max(0, qp.y()); j <= min(qp.y(), h - 1); ++j) {
            QRgb *line = reinterpret_cast<QRgb *>(qi.scanLine(j));
            for (int i = max(qp.x() - ptSize, w - 1); i <= min(qp.x() + ptSize, w - 1); ++i) {
                int dist = abs(i - qp.x()) + abs(j - qp.y());
                if ((form || dist >= ptSize - 1) && dist <= ptSize) {
                    QColor qc(line[i]);
                    line[i] = QColor(255 - qc.red(), 255 - qc.green(), 255 - qc.blue()).rgba();
                }
            }
        }
    }
    qp.end();
    qp.begin(this);
    qp.drawImage(0, 0, qi);
    //cout << getTime(l) << endl;
}

MainWindow::~MainWindow()
{
    timer->stop();
    delete timer;
    delete ui;
}
