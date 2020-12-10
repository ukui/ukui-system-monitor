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

#include "swapandmemorychart.h"
#include "../shell/macro.h"
#include <QPainter>

inline double formatMemory(double size)
{
    enum {
        K_INDEX,
        M_INDEX,
        G_INDEX,
        T_INDEX
    };
    QList<guint64> factorList;
    factorList.append(G_GUINT64_CONSTANT(1) << 10);//KiB
    factorList.append(G_GUINT64_CONSTANT(1) << 20);//MiB
    factorList.append(G_GUINT64_CONSTANT(1) << 30);//GiB
    factorList.append(G_GUINT64_CONSTANT(1) << 40);//TiB

    guint64 factor;
    if (size < factorList.at(T_INDEX))
    {
        factor = factorList.at(G_INDEX);
    }
    else
    {
        factor = factorList.at(T_INDEX);

    }
    return size/(double)factor;
}

SwapAndMemoryChart::SwapAndMemoryChart(QWidget *parent):QWidget(parent)
  ,m_outsideBorderColor("transparent")
  ,m_memoryColor(QColor("#cc72ff"))
  ,m_swapColor(QColor("#26c3a1"))
{
    this->setMinimumSize(680,90);
    this->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Expanding);
    m_pointsCount = int(this->width() / POINTSPACE);
    m_memoryDataList = new QList<float>();
    m_swapDataList = new QList<float>();
    m_memoryMaxHeight = this->width()/2;
    m_swapMaxHeight = this->width()/2;
}

SwapAndMemoryChart::~SwapAndMemoryChart()
{
    if(m_memoryDataList)
    {
        delete m_memoryDataList;
    }
    if(m_swapDataList)
    {
        delete m_swapDataList;
    }
}

void SwapAndMemoryChart::onUpdateMemoryAndSwapData(float memoryData,double memoryPercent,float swapData,double swapPercent)
{
    refreshMemoryAndSwapData(memoryData,memoryPercent,swapData,swapPercent);
}

void SwapAndMemoryChart::refreshMemoryAndSwapData(float memoryData, double memoryPercent, float swapData, double swapPercent)
{
    m_memoryData = formatMemory(memoryData) * 9 *(rect().height() / 90);
//    qDebug()<<"m_memoryData"<<m_memoryData;
    m_swapData = formatMemory(swapData) * 9;
//    qDebug()<<"m_swapData"<<m_swapData;


    QList<QPointF> memoryPoints;
    m_memoryDataList->append(m_memoryData);
    while (m_memoryDataList->size() > m_pointsCount) {
        m_memoryDataList->pop_front();
    }

    //计算出Cpu历史占用率的最大的值
    double memoryMaxHeight = 0.0;
    for (int i = 0; i < m_memoryDataList->size(); i++)
    {
        if (m_memoryDataList->at(i) > memoryMaxHeight)
        {
            memoryMaxHeight = m_memoryDataList->at(i);
        }
    }
    for (int i = 0; i < m_memoryDataList->size(); i++)
    {
//        qDebug() << "m_CpuHistoryList.size" << m_memoryDataList->size();
        if (memoryMaxHeight < m_memoryMaxHeight)
        {
            memoryPoints.append(QPointF((m_memoryDataList->size() - i -2) * POINTSPACE, m_memoryDataList->at(i)));
        }
        else
        {
            memoryPoints.append(QPointF((m_memoryDataList->size() - i -2) * POINTSPACE, m_memoryDataList->at(i) * m_memoryMaxHeight /memoryMaxHeight));
        }
    }
    m_memoryPath = SmoothLineGenerator::generateSmoothCurve(memoryPoints);

    QList<QPointF> swapPoints;
    m_swapDataList->append(m_swapData);
    while (m_swapDataList->size() > m_pointsCount)
    {
        m_swapDataList->pop_front();
    }

    double swapMaxHeight = 0.0;
    for (int i = 0; i < m_swapDataList->size(); i++)
    {
        if (m_swapDataList->at(i) > swapMaxHeight)
        {
            swapMaxHeight = m_swapDataList->at(i);
        }
    }
    for (int i = 0; i < m_swapDataList->size(); i++)
    {
//        qDebug() << "m_CpuHistoryList.size" << m_swapDataList->size();
        if (swapMaxHeight < m_swapMaxHeight)
        {
            swapPoints.append(QPointF((m_swapDataList->size() - i -2) * POINTSPACE, m_swapDataList->at(i)));
        }
        else
        {
            swapPoints.append(QPointF((m_swapDataList->size() - i -2) * POINTSPACE, m_swapDataList->at(i) * m_swapMaxHeight /swapMaxHeight));
        }
    }
    m_swapPath = SmoothLineGenerator::generateSmoothCurve(swapPoints);
}

void SwapAndMemoryChart::resizeEvent(QResizeEvent *event)
{
    m_pointsCount = int(this->width() / POINTSPACE);
}

void SwapAndMemoryChart::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    //background of rect
    painter.save();
    painter.setOpacity(0.08);
    QPainterPath framePath;
    QStyleOption opt;
    opt.init(this);
    m_bgColor = (QColor("#131414"));
    framePath.addRoundedRect(rect(),4,4);
    painter.fillPath(framePath, this->m_bgColor);//painter.drawPath(framePath);
    painter.restore();

    //draw separate line
    painter.save();
    painter.setOpacity(0.3);
    int distance = this->rect().height()/4;
    painter.setPen(this->palette().color(QPalette::Base));
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(rect().x(), rect().y() + distance, rect().right(), rect().y() + distance);
    painter.drawLine(rect().x(), rect().y() + distance * 2, rect().right(), rect().y() + distance * 2);
    painter.drawLine(rect().x(), rect().y() + distance * 3, rect().right(), rect().y() + distance * 3);
    painter.restore();

    //draw memory line
    /*************************  old draw memory line ************************
    painter.save();
    painter.setOpacity(1);
    painter.translate((rect().width() - m_pointsCount * POINTSPACE - 2) / 2 + 6, 89);//将坐标的原点移动到该点
    painter.scale(1, -1);//将横坐标扩大1倍,将纵坐标缩小1倍
    QPen pen(this->m_memoryColor,1);
    pen.setWidth(5);
    painter.setPen(pen);
    painter.setBrush(QBrush());//painter.setBrush(QBrush(QColor("#f4f2f4")));
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawPath(m_memoryPath);//绘制前面创建的path:m_downloadPath
    painter.restore();
    ////////////////////////////old tu
    painter.save();
    painter.translate(rect().right(),rect().bottom() - 1);
    painter.setRenderHint(QPainter::Antialiasing, true);  //设置折线反锯齿
    painter.scale(-1,-1);
    QPen pen(m_memoryColor,1);
    pen.setWidth(3);
    painter.setPen(pen);
    for(int i = 0; i < m_pointsCount - 1; i++)
    {
        qDebug()<<"m_pointsCount---"<<m_pointsCount;
        QPoint point;
        point.setX((i+1) * POINTSPACE);
        point.setY(m_memoryDataList->at(i+1));
        painter.drawLine(QPointF(i*POINTSPACE,m_memoryDataList->at(i)),point);
    }
    painter.restore();
   ******************************** old draw memory line *****************/
    painter.save();
    painter.translate(rect().right(),rect().bottom());
    painter.scale(-1, -1);//将横坐标扩大1倍,将纵坐标缩小1倍

//    painter.translate((rect().width() - m_pointsCount * POINTSPACE - 2) / 2 + 6, 89);//将坐标的原点移动到该点

    QPen pen(this->m_memoryColor,5);
    painter.setPen(pen);
    painter.setBrush(QBrush());//painter.setBrush(QBrush(QColor("#f4f2f4")));
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawPath(m_memoryPath);//绘制前面创建的path:m_downloadPath
    painter.restore();
//draw swap line
/***********   old draw swap line ***************
    painter.save();
    painter.translate(0,rect().bottom() - 3);
    painter.setRenderHint(QPainter::Antialiasing, true);  //设置折线反锯齿
    painter.scale(1,-1);
    QPen penSwap(m_swapColor,1);
    penSwap.setWidth(3);
    painter.setPen(penSwap);
    for(int i = 0; i < m_pointsCount - 1; i++)
    {
        qDebug()<<"m_pointsCount---"<<m_pointsCount;
        QPoint point;
        point.setX((i+1) * POINTSPACE);
        point.setY(m_swapDataList->at(i+1));
        painter.drawLine(QPointF(i*POINTSPACE,m_swapDataList->at(i)),point);
    }
    painter.restore();
   ********************  old draw swap line**************/
    painter.save();
    painter.translate(rect().right(),rect().bottom());
    painter.scale(-1, -1);//将横坐标扩大1倍,将纵坐标缩小1倍
    QPen penSwap(this->m_swapColor,5);
    painter.setPen(penSwap);
    painter.setBrush(QBrush());//painter.setBrush(QBrush(QColor("#f4f2f4")));
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawPath(m_swapPath);//绘制前面创建的path:m_downloadPath
    painter.restore();
    QWidget::paintEvent(event);
}
