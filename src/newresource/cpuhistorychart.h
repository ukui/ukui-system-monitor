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

#ifndef _CPUHISTORYCHART_H_
#define _CPUHISTORYCHART_H_

#include <QWidget>
#include <QStyleOption>
#include <QPainterPath>
#include <QMap>
#include <QtCharts>

QT_CHARTS_USE_NAMESPACE

class CpuHistoryChart : public QWidget
{
    Q_OBJECT
public:
    CpuHistoryChart(QWidget *parent = 0);
    ~CpuHistoryChart();
    void refreshCpuHistoryData(double cpu);
    void redrawCpuHistoryPath();
    void setBgColor(QColor bgColor);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QColor m_cpuHistoryColor;

    QList<double> m_CpuHistoryList;

    int m_pointsCount;
    QColor m_bgColor;
    double m_Cpu;
    double m_cpuMaxHeight;

    QChart *m_chart = nullptr;
    QChartView *m_chartView = nullptr;
    QLineSeries *m_upLineSeriesCpu = nullptr;
    QLineSeries *m_lowLineSeriesCpu = nullptr;
    QAreaSeries *m_areaSeriesCpu = nullptr;
    QValueAxis *m_valueAxisX = nullptr;
    QValueAxis *m_valueAxisY = nullptr;

public slots:
    void onUpdateCpuPercent(double value);
//    void onUpdateCpuHistoryStatus(double value);
};

#endif //_CPUHISTORYCHART_H_
