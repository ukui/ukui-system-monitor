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
#include <QPainterPath>
#include <QLinearGradient>

#define CPUHIS_POINT_COUNT_MAX      100

CpuHistoryChart::CpuHistoryChart(QWidget *parent):QWidget(parent)
  ,m_cpuHistoryColor(QColor("#F64F61"))
{
    this->setMinimumSize(640,90);
    this->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Expanding);
    m_pointsCount = CPUHIS_POINT_COUNT_MAX;
    m_CpuHistoryList.clear();
    m_bgColor = (QColor("#131414"));

    QHBoxLayout *chartLayout = new QHBoxLayout();
    chartLayout->setMargin(0);
    chartLayout->setSpacing(0);
    chartLayout->setContentsMargins(0,0,0,0);
    m_chart = new QChart();
    m_chart->legend()->hide();
    m_chart->setMargins(QMargins(-8,-8,-8,-8));
    m_chartView = new QChartView(this);
    m_chartView->setChart(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    m_chartView->setContentsMargins(0,0,0,0);
    chartLayout->addWidget(m_chartView);
    m_upLineSeriesCpu = new QLineSeries(this);
    m_lowLineSeriesCpu = new QLineSeries(this);
    m_areaSeriesCpu = new QAreaSeries(this);
    m_areaSeriesCpu->setPen(m_cpuHistoryColor);
    QLinearGradient linearGrad(QPointF(0, 0), QPointF(0, 100)); 
    linearGrad.setColorAt(0, QColor("#A6F1A55A"));
    linearGrad.setColorAt(1, QColor("#A6F64F61"));
    m_areaSeriesCpu->setBrush(QBrush(linearGrad));
    m_areaSeriesCpu->setUpperSeries(m_upLineSeriesCpu);
    m_areaSeriesCpu->setLowerSeries(m_lowLineSeriesCpu);
    m_chart->addSeries(m_areaSeriesCpu);
    m_valueAxisX = new QValueAxis(this);
    m_valueAxisY = new QValueAxis(this);
    m_valueAxisX->setRange(0, 100.0);
    m_valueAxisX->setVisible(false);
    m_valueAxisX->setReverse();
    m_valueAxisY->setRange(0, 100.0);
    m_valueAxisY->setVisible(false);

    m_chart->addAxis(m_valueAxisX, Qt::AlignBottom);
    m_areaSeriesCpu->attachAxis(m_valueAxisX);
    m_chart->addAxis(m_valueAxisY, Qt::AlignLeft);
    m_areaSeriesCpu->attachAxis(m_valueAxisY);

    m_chart->setBackgroundVisible(false);
    setAttribute(Qt::WA_TranslucentBackground); 
    m_chartView->setStyleSheet("background: transparent");

    this->setLayout(chartLayout);
}

CpuHistoryChart::~CpuHistoryChart()
{
    m_CpuHistoryList.clear();
}

// 绘制cpu历史占比背景及前景线
void CpuHistoryChart::paintEvent(QPaintEvent *event)
{
///*****************************原来的折线图绘制法**********************************
#if 0
    qreal lfPointSpace = (qreal)width()/m_pointsCount;
    qreal lfPointYP = (qreal)height()/100;
#endif
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
#if 0
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
#endif
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
    QList<QPointF> listUp;
    QList<QPointF> listLow;
    for (int n = 0; n < m_CpuHistoryList.size(); n++) {
        QPointF pointUp;
        QPointF pointLow;
        pointUp.setX(n);
        pointUp.setY(m_CpuHistoryList[m_CpuHistoryList.size()-1-n]);
        listUp.append(pointUp);
        pointLow.setX(n);
        pointLow.setY(0);
        listLow.append(pointLow);
    }

    m_upLineSeriesCpu->clear();
    m_upLineSeriesCpu->replace(listUp);

    m_lowLineSeriesCpu->clear();
    m_lowLineSeriesCpu->replace(listLow);

    m_areaSeriesCpu->setUpperSeries(m_upLineSeriesCpu);
    m_areaSeriesCpu->setLowerSeries(m_lowLineSeriesCpu);
}

void CpuHistoryChart::onUpdateCpuPercent(double value)
{
    this->refreshCpuHistoryData(value);                  //about the cpurateForm showing
}
