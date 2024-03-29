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
#include <math.h>
#include <QPainterPath>
#include <QLinearGradient>

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

#define MEMHIS_POINT_COUNT_MAX      100

SwapAndMemoryChart::SwapAndMemoryChart(QWidget *parent):QWidget(parent)
  ,m_memoryColor(QColor("#cc72ff"))
  ,m_swapColor(QColor("#26c3a1"))
{
    this->setMinimumSize(640,90);
    this->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Expanding);
    m_pointsCount = MEMHIS_POINT_COUNT_MAX;
    m_memoryDataList.clear();
    m_swapDataList.clear();
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
    //Mem
    m_upLineSeriesMem = new QLineSeries(this);
    m_lowLineSeriesMem = new QLineSeries(this);
    m_areaSeriesMem = new QAreaSeries(this);
    m_areaSeriesMem->setPen(m_memoryColor);
    QLinearGradient linearGrad(QPointF(0, 0), QPointF(0, 100)); 
    linearGrad.setColorAt(0, QColor("#A6B472F2")); 
    linearGrad.setColorAt(1, QColor("#A6DC72F7"));
    m_areaSeriesMem->setBrush(QBrush(linearGrad));
    m_areaSeriesMem->setUpperSeries(m_upLineSeriesMem);
    m_areaSeriesMem->setLowerSeries(m_lowLineSeriesMem);
    m_chart->addSeries(m_areaSeriesMem);
    //Swap
    m_upLineSeriesSwap = new QLineSeries(this);
    m_lowLineSeriesSwap = new QLineSeries(this);
    m_areaSeriesSwap = new QAreaSeries(this);
    m_areaSeriesSwap->setPen(m_swapColor);
    linearGrad.setColorAt(0, QColor("#A65BD564"));
    linearGrad.setColorAt(1, QColor("#A626C3A1"));
    m_areaSeriesSwap->setBrush(QBrush(linearGrad));
    m_areaSeriesSwap->setUpperSeries(m_upLineSeriesSwap);
    m_areaSeriesSwap->setLowerSeries(m_lowLineSeriesSwap);
    m_chart->addSeries(m_areaSeriesSwap);

    m_valueAxisX = new QValueAxis(this);
    m_valueAxisY = new QValueAxis(this);
    m_valueAxisX->setRange(0, 100.0);
    m_valueAxisX->setVisible(false);
    m_valueAxisX->setReverse();
    m_valueAxisY->setRange(0, 100.0);
    m_valueAxisY->setVisible(false);

    m_chart->addAxis(m_valueAxisX, Qt::AlignBottom);
    m_areaSeriesMem->attachAxis(m_valueAxisX);
    m_areaSeriesSwap->attachAxis(m_valueAxisX);
    m_chart->addAxis(m_valueAxisY, Qt::AlignLeft);
    m_areaSeriesMem->attachAxis(m_valueAxisY);
    m_areaSeriesSwap->attachAxis(m_valueAxisY);

    m_chart->setBackgroundVisible(false);
    setAttribute(Qt::WA_TranslucentBackground); 
    m_chartView->setStyleSheet("background: transparent");

    this->setLayout(chartLayout);
}

SwapAndMemoryChart::~SwapAndMemoryChart()
{
}

void SwapAndMemoryChart::onUpdateMemoryAndSwapData(float memoryUsed,double memoryTotal,float swapUsed,double swapTotal)
{
    refreshMemoryAndSwapData(memoryUsed,memoryTotal,swapUsed,swapTotal);
}

inline qreal getMemMaxSpace(qreal lfRealTotal)
{
    qreal lfMemTotalG = ceil(lfRealTotal / 1024 / 1024);
    // 向2的n次方取整
    int nPow = ceil(log(lfMemTotalG)/log(2.0));
    lfMemTotalG = pow(2.0, nPow);
    return (lfMemTotalG*1024*1024);
}

void SwapAndMemoryChart::refreshMemoryAndSwapData(float memoryUsed, double memoryTotal, float swapUsed, double swapTotal)
{
    m_memoryDataList.append(memoryUsed);
    while (m_memoryDataList.size() > m_pointsCount) {
        m_memoryDataList.pop_front();
    }
    m_swapDataList.append(swapUsed);
    while (m_swapDataList.size() > m_pointsCount) {
        m_swapDataList.pop_front();
    }

    if (memoryTotal > m_curMaxMemSpace) {
        m_curMaxMemSpace = memoryTotal;
    }
    if (swapTotal > m_curMaxMemSpace) {
        m_curMaxMemSpace = swapTotal;
    }
    m_curMaxMemSpace = getMemMaxSpace(m_curMaxMemSpace);
    emit this->spaceToDynamicMax(m_curMaxMemSpace);

    QList<QPointF> listUp;
    QList<QPointF> listLow;
    // Mem
    for (int n = 0; n < m_memoryDataList.size(); n++) {
        QPointF pointUp;
        QPointF pointLow;
        pointUp.setX(n);
        pointUp.setY(m_memoryDataList[m_memoryDataList.size()-1-n]*100/m_curMaxMemSpace);
        listUp.append(pointUp);
        pointLow.setX(n);
        pointLow.setY(0);
        listLow.append(pointLow);
    }

    m_upLineSeriesMem->clear();
    m_upLineSeriesMem->replace(listUp);

    m_lowLineSeriesMem->clear();
    m_lowLineSeriesMem->replace(listLow);

    m_areaSeriesMem->setUpperSeries(m_upLineSeriesMem);
    m_areaSeriesMem->setLowerSeries(m_lowLineSeriesMem);

    // Swap
    listUp.clear();
    listLow.clear();
    for (int n = 0; n < m_swapDataList.size(); n++) {
        QPointF pointUp;
        QPointF pointLow;
        pointUp.setX(n);
        pointUp.setY(m_swapDataList[m_swapDataList.size()-1-n]*100/m_curMaxMemSpace);
        listUp.append(pointUp);
        pointLow.setX(n);
        pointLow.setY(0);
        listLow.append(pointLow);
    }

    m_upLineSeriesSwap->clear();
    m_upLineSeriesSwap->replace(listUp);

    m_lowLineSeriesSwap->clear();
    m_lowLineSeriesSwap->replace(listLow);

    m_areaSeriesSwap->setUpperSeries(m_upLineSeriesSwap);
    m_areaSeriesSwap->setLowerSeries(m_lowLineSeriesSwap);
}

void SwapAndMemoryChart::setBgColor(QColor bgColor)
{
    m_bgColor = bgColor;
}

void SwapAndMemoryChart::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
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

// calculate memory path
#if 0
    qreal lfPointSpace = (qreal)width()/m_pointsCount;
    qreal lfPointYP = (qreal)height()/100;
    QList<QPointF> memoryPoints;
    for (int i = 0; i < m_memoryDataList.size(); i++)
    {
        memoryPoints.append(QPointF((m_memoryDataList.size()-i-1) * lfPointSpace, m_memoryDataList[i]*100*lfPointYP/m_curMaxMemSpace));
    }
    if (!memoryPoints.isEmpty()) {
        QPainterPath pathTmp;
        m_memoryPath.swap(pathTmp);
        m_memoryPath.moveTo(memoryPoints[0]);
        for (int n = 1; n < memoryPoints.size(); n++) {
            #if 1
            QPointF ctlPoint[2];
            ctlPoint[0].setX((memoryPoints[n-1].x()+memoryPoints[n].x())/2);
            ctlPoint[0].setY(memoryPoints[n-1].y());
            ctlPoint[1].setX((memoryPoints[n-1].x()+memoryPoints[n].x())/2);
            ctlPoint[1].setY(memoryPoints[n].y());
            m_memoryPath.cubicTo(ctlPoint[0], ctlPoint[1], memoryPoints[n]);
            #else
            m_memoryPath.lineTo(memoryPoints[n]);
            #endif
        }
    }
// calculate swap path
    QList<QPointF> swapPoints;
    for (int i = 0; i < m_swapDataList.size(); i++)
    {
        swapPoints.append(QPointF((m_swapDataList.size()-i-1) * lfPointSpace, m_swapDataList[i]*100*lfPointYP/m_curMaxMemSpace));
    }
    if (!swapPoints.isEmpty()) {
        QPainterPath pathTmp;
        m_swapPath.swap(pathTmp);
        m_swapPath.moveTo(swapPoints[0]);
        for (int n = 1; n < swapPoints.size(); n++) {
            #if 1
            QPointF ctlPoint[2];
            ctlPoint[0].setX((swapPoints[n-1].x()+swapPoints[n].x())/2);
            ctlPoint[0].setY(swapPoints[n-1].y());
            ctlPoint[1].setX((swapPoints[n-1].x()+swapPoints[n].x())/2);
            ctlPoint[1].setY(swapPoints[n].y());
            m_swapPath.cubicTo(ctlPoint[0], ctlPoint[1], swapPoints[n]);
            #else
            m_swapPath.lineTo(swapPoints[n]);
            #endif
        }
    }
//draw memory line
    painter.save();
    painter.translate(rect().right(),rect().bottom()-1);//将坐标的原点移动到该点
    painter.rotate(180);//将横纵坐标顺时针旋转180°
    QPen pen(this->m_memoryColor,2);
    painter.setPen(pen);
    painter.setBrush(QBrush());
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawPath(m_memoryPath);//绘制前面创建的path:m_downloadPath
//draw swap line
    painter.translate(0,-1);
    QPen penSwap(this->m_swapColor,2);
    painter.setPen(penSwap);
    painter.drawPath(m_swapPath);//绘制前面创建的path:m_downloadPath
    painter.restore();
#endif
    QWidget::paintEvent(event);
}
