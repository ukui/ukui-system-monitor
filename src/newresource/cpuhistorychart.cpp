#include <QPainter>
#include <QDebug>
#include "cpuhistorychart.h"
#include "../shell/macro.h"
#include "smoothlinegenerator.h"

CpuHistoryChart::CpuHistoryChart(QWidget *parent):QWidget(parent)
  ,m_outsideBorderColor("transparent")
  ,m_cpuHistoryColor(QColor("#F64F61"))
{
    this->setMinimumSize(680,90);
    this->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Expanding);
    qDebug()<<"cpu.chart.size"<<this->width();
    m_pointsCount = int((this->width()) /10);
    m_cpuMaxHeight = 50;
    m_CpuHistoryList = new QList<double>();

//    for (int i = 0; i < m_pointsCount; i++)
//    {
//        m_CpuHistoryList->append(0);
//    }
//    xList<<0;
//    yDrawList<<0;
}

CpuHistoryChart::~CpuHistoryChart()
{

}


void CpuHistoryChart::paintEvent(QPaintEvent *event)
{
///*****************************原来的折线图绘制法**********************************
    m_pointsCount = rect().width() / POINTSPACE;
    QPainter painter(this);
    painter.save();
//    int gridX = rect().x() + PENSIZE;
//    int gridY = rect().y() + YEXSIZE;
//    int gridWidth = rect().width() - PENSIZE * 2;
    //int gridHeight = 90;

//background of rect
    painter.setOpacity(0.08);
    QPainterPath framePath;
    QStyleOption opt;
    opt.init(this);
    m_bgColor = (QColor("#131414"));

//    framePath.addRoundedRect(QRectF(gridX, gridY, gridWidth, GRIDHEIGHT),4,4);
    framePath.addRoundedRect(rect(), 4, 4);
    painter.fillPath(framePath, this->m_bgColor);//painter.drawPath(framePath);
    painter.restore();

//draw separate lines
    painter.save();
    painter.setOpacity(0.5);
    int distance = this->rect().height()/4;
    painter.setPen(this->palette().color(QPalette::Base));
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(rect().x(), rect().y() + distance, rect().right(), rect().y() + distance);
    painter.drawLine(rect().x(), rect().y() + distance * 2, rect().right(), rect().y() + distance * 2);
    painter.drawLine(rect().x(), rect().y() + distance * 3, rect().right(), rect().y() + distance * 3);
    painter.restore();

//draw Cpu history line
    painter.save();
    painter.translate(rect().right(),rect().bottom());
    painter.scale(-1, -1);//将横坐标扩大1倍,将纵坐标缩小1倍

//    painter.translate((rect().width() - m_pointsCount * POINTSPACE - 2) / 2 + 6, 89);//将坐标的原点移动到该点
    QPen pen(this->m_cpuHistoryColor,2);
    painter.setPen(pen);

//    QLinearGradient gradient(0,0,0,0);
//    gradient.setColorAt(0.2, QColor::fromRgbF(0, 1, 0, 1));
//    gradient.setColorAt(0.6, QColor::fromRgbF(1, 0, 0, 1));
//    gradient.setColorAt(1.0, QColor::fromRgbF(1, 1, 1, 1));
//    painter.setBrush(gradient);//painter.setBrush(QBrush(QColor("#f4f2f4")));

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawPath(m_cpuHistoryPath);//绘制前面创建的path:m_downloadPath
    painter.restore();
    update();
//    **********************************现在的折线图写法********************************/

//    QPainter painter(this);
//    painter.setOpacity(0.08);
//    QPainterPath framePath;
//    QStyleOption opt;
//    opt.init(this);
//    m_bgColor = (QColor("#131414"));

//    framePath.addRoundedRect(QRectF(this->rect().x(), this->rect().y(), this->rect().width(), this->rect().height()),4,4);
//    painter.fillPath(framePath, this->m_bgColor);//painter.drawPath(framePath);

//    QPainter p(this);
//    p.save();
//    p.setRenderHint(QPainter::Antialiasing, true);  //设置折线反锯齿
//    p.scale(1,1);
//    QColor colorCpuHistory = QColor("#F64F61");
//    QPen pen(colorCpuHistory,1);
//    pen.setWidth(2);
//    p.setPen(pen);
//    for(int j = 2; j<i-1; j++)
//    {
//        point.setX(xList[j+1]);
//        point.setY(yDrawList[j+1]);
//        p.drawLine(QPointF(xList[j],yDrawList[j]),point);
//    }
}

void CpuHistoryChart::refreshCpuHistoryData(double cpu)
{
    m_Cpu = cpu  / 90 * rect().height();
    QList<QPointF> cpuHistoryPoints;
    m_CpuHistoryList->append(cpu);
    while (m_CpuHistoryList->size() > m_pointsCount)    {
        m_CpuHistoryList->pop_front();
    }

    //计算出Cpu历史占用率的最大的值
    double cpuHistoryMaxHeight = 0.0;
    for (int i = 0; i < m_CpuHistoryList->size(); i++)
    {
        if (m_CpuHistoryList->at(i) > cpuHistoryMaxHeight)
        {
            cpuHistoryMaxHeight = m_CpuHistoryList->at(i);
        }
    }
    for (int i = 0; i < m_CpuHistoryList->size(); i++)
    {
        qDebug()<<"m_CpuHistoryList.size"<<m_CpuHistoryList->size();
        if (cpuHistoryMaxHeight < m_cpuMaxHeight)
        {
            cpuHistoryPoints.append(QPointF((m_CpuHistoryList->size() - i -2) * POINTSPACE, m_CpuHistoryList->at(i)));
        }
        else
        {
            cpuHistoryPoints.append(QPointF((m_CpuHistoryList->size() - i -2) * POINTSPACE, m_CpuHistoryList->at(i) * m_cpuMaxHeight /cpuHistoryMaxHeight));
        }
    }
    m_cpuHistoryPath = SmoothLineGenerator::generateSmoothCurve(cpuHistoryPoints);
//    update();

//    int x = int(m_pointsCount *i * 0.1 + 0.5);
//    qDebug()<<"xxx"<<x;
//    showValue = int(cpu) + 0.5;

//    qDebug()<<"showvalue---"<<showValue;
//    xList<< x;
//    yDrawList << showValue;
//    update();
//    i++;
}

void CpuHistoryChart::onUpdateCpuPercent(double value)
{
    qDebug()<<"----onUpdateCpuPercent----";
    this->refreshCpuHistoryData(value);                  //about the cpurateForm showing
}
