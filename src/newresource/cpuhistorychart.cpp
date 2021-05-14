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

#define CPUHIS_POINT_COUNT_MAX      100

CpuHistoryChart::CpuHistoryChart(QWidget *parent):QWidget(parent)
  ,m_outsideBorderColor("transparent")
  ,m_cpuHistoryColor(QColor("#F64F61"))
{
    this->setMinimumSize(660,90);
    this->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Expanding);
    m_pointsCount = CPUHIS_POINT_COUNT_MAX;
    m_CpuHistoryList.clear();
    m_bgColor = (QColor("#131414"));
}

CpuHistoryChart::~CpuHistoryChart()
{
    m_CpuHistoryList.clear();
}

// 绘制cpu历史占比背景及前景线
void CpuHistoryChart::paintEvent(QPaintEvent *event)
{
///*****************************原来的折线图绘制法**********************************
    qreal lfPointSpace = (qreal)width()/m_pointsCount;
    qreal lfPointYP = (qreal)height()/100;
    QPainter painter(this);
    painter.save();

//background of rect
    painter.setOpacity(0.08);
    QPainterPath framePath;
    QStyleOption opt;
    opt.init(this);
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
    painter.rotate(180);//将横纵坐标顺时针旋转180°

    QPen pen(this->m_cpuHistoryColor,2);
    QList<QPointF> cpuHistoryPoints;
    for (int i = 0; i < m_CpuHistoryList.size(); i++)
    {
        cpuHistoryPoints.append(QPointF((m_CpuHistoryList.size()-i-1) * lfPointSpace, m_CpuHistoryList[i]*lfPointYP));
    }
    QPainterPath pathPoint;
    if (!cpuHistoryPoints.isEmpty()) {
        pathPoint.moveTo(cpuHistoryPoints[0]);
        for (int n = 1; n < cpuHistoryPoints.size(); n++) {
            #if 1
            QPointF ctlPoint[2];
            ctlPoint[0].setX((cpuHistoryPoints[n-1].x()+cpuHistoryPoints[n].x())/2);
            ctlPoint[0].setY(cpuHistoryPoints[n-1].y());
            ctlPoint[1].setX((cpuHistoryPoints[n-1].x()+cpuHistoryPoints[n].x())/2);
            ctlPoint[1].setY(cpuHistoryPoints[n].y());
            pathPoint.cubicTo(ctlPoint[0], ctlPoint[1], cpuHistoryPoints[n]);
            #else
            pathPoint.lineTo(cpuHistoryPoints[n]);
            #endif
        }
    }
    painter.setPen(pen);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawPath(pathPoint);
    painter.restore();
    QWidget::paintEvent(event);
}

void CpuHistoryChart::setBgColor(QColor bgColor)
{
    m_bgColor = bgColor;
}

// 记录cpu历史占比
void CpuHistoryChart::refreshCpuHistoryData(double cpu)
{
    m_CpuHistoryList.append(cpu);
    while (m_CpuHistoryList.size() > m_pointsCount)    {
        m_CpuHistoryList.pop_front();
    }
}

void CpuHistoryChart::onUpdateCpuPercent(double value)
{
    this->refreshCpuHistoryData(value);                  //about the cpurateForm showing
}
