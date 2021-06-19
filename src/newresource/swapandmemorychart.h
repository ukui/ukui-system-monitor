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

#ifndef _SWAPANDMEMORYCHART_H
#define _SWAPANDMEMORYCHART_H

#include <QWidget>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include <glibtop/mem.h>
#include <glibtop/swap.h>
#include <QPainterPath>
#include "smoothlinegenerator.h"
#include <QtCharts>

QT_CHARTS_USE_NAMESPACE

class SwapAndMemoryChart : public QWidget
{
    Q_OBJECT
public:
    SwapAndMemoryChart(QWidget *parent = 0);
    ~SwapAndMemoryChart();
    void refreshMemoryAndSwapData(float,double,float,double);
    void setBgColor(QColor bgColor);
protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    QColor m_memoryColor;
    QColor m_swapColor;
    QColor m_bgColor;

    QList<float> m_memoryDataList;
    QList<float> m_swapDataList;

    int m_pointsCount;
#if 0
    QPainterPath m_memoryPath;
    QPainterPath m_swapPath;
#endif

    qreal m_curMaxMemSpace = 1;

    QChart *m_chart = nullptr;
    QChartView *m_chartView = nullptr;
    QLineSeries *m_upLineSeriesMem = nullptr;
    QLineSeries *m_lowLineSeriesMem = nullptr;
    QAreaSeries *m_areaSeriesMem = nullptr;
    QLineSeries *m_upLineSeriesSwap = nullptr;
    QLineSeries *m_lowLineSeriesSwap = nullptr;
    QAreaSeries *m_areaSeriesSwap = nullptr;
    QValueAxis *m_valueAxisX = nullptr;
    QValueAxis *m_valueAxisY = nullptr;

public slots:
    void onUpdateMemoryAndSwapData(float,double,float,double);
signals:
    void spaceToDynamicMax(qreal lfMaxSpace);
};

#endif //#define _SWAPANDMEMORYCHART_H
