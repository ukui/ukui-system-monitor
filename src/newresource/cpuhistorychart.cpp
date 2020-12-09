/*
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntukylin.com/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
}

CpuHistoryChart::~CpuHistoryChart()
{
    if(m_CpuHistoryList)
    {
        delete m_CpuHistoryList;
    }
}


void CpuHistoryChart::paintEvent(QPaintEvent *event)
{
///*****************************原来的折线图绘制法**********************************
    m_pointsCount = rect().width() / POINTSPACE;
    QPainter painter(this);
    painter.save();

//background of rect
    painter.setOpacity(0.08);
    QPainterPath framePath;
    QStyleOption opt;
    opt.init(this);
    m_bgColor = (QColor("#131414"));
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
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawPath(m_cpuHistoryPath);//绘制前面创建的path:m_downloadPath
    painter.restore();
    update();
    QWidget::paintEvent(event);
//    **********************************现在的折线图写法********************************/

   /* QPainter painter(this);
    painter.setOpacity(0.08);
    QPainterPath framePath;
    QStyleOption opt;
    opt.init(this);
    m_bgColor = (QColor("#131414"));

    framePath.addRoundedRect(QRectF(this->rect().x(), this->rect().y(), this->rect().width(), this->rect().height()),4,4);
    painter.fillPath(framePath, this->m_bgColor);//painter.drawPath(framePath);

    QPainter p(this);
    p.save();
    p.setRenderHint(QPainter::Antialiasing, true);  //设置折线反锯齿
    p.scale(1,1);
    QColor colorCpuHistory = QColor("#F64F61");
    QPen pen(colorCpuHistory,1);
    pen.setWidth(2);
    p.setPen(pen);
    for(int j = 2; j<i-1; j++)
    {
        point.setX(xList[j+1]);
        point.setY(yDrawList[j+1]);
        p.drawLine(QPointF(xList[j],yDrawList[j]),point);
    } */
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
