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

class SwapAndMemoryChart : public QWidget
{
    Q_OBJECT
public:
    SwapAndMemoryChart(QWidget *parent = 0);
    ~SwapAndMemoryChart();
    void refreshMemoryAndSwapData(float,double,float,double);
protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    QColor m_outsideBorderColor;
    QColor m_memoryColor;
    QColor m_swapColor;
    QColor m_bgColor;

    float m_memoryData;
    float m_swapData;

    QList<float> *m_memoryDataList;
    QList<float> *m_swapDataList;

    int m_pointsCount;
    int m_pointSpace;

    int m_memoryMaxHeight;
    int m_swapMaxHeight;
    QPainterPath m_memoryPath;
    QPainterPath m_swapPath;
    int countWidth;

    QList<qreal> xList; //x数据链
    QList<qreal> yDrawList; //y数据链
    int showValue;

public slots:
    void onUpdateMemoryAndSwapData(float,double,float,double);
};

#endif //#define _SWAPANDMEMORYCHART_H
