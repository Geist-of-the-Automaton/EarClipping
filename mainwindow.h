#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QKeyEvent>
#include <QTimer>
#include <QColorDialog>
#include <iostream>
#include <vector>
#include <list>
#include <polygon.h>

#include <time.h>
#include <chrono>
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
using Qt::MouseButtons;
using Qt::MouseButton;
using std::find;
using std::cout;
using std::endl;

const int len = 700;
const int ptSize = 7;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

static long long getTime(long long initial = 0) {
    return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() - initial;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void paintEvent(QPaintEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void calcTri(Triangle t);
    void fillBTri(QPoint a, QPoint b, QPoint c);
    void fillTTri(QPoint a, QPoint b, QPoint c);

    QImage qi;
    int activePt, form, dragDraw;
    MouseButton lastButton;
    QTimer *timer;
    QRgb top, bottom;
    int dispDivs;
    bool shiftFlag, ctrlFlag, triSegFlag;

private:
    Ui::MainWindow *ui;
    vector <Polygon> gons;
    vector <int> activeGons;

public slots:
    void flashCPts();

};
#endif // MAINWINDOW_H
