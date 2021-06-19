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

#ifndef _NETWORKCHART_H_
#define _NETWORKCHSRT_H_

#include <QWidget>
#include <QStyleOption>
#include <QPainterPath>
#include <QtCharts>

QT_CHARTS_USE_NAMESPACE

class NetWorkChart : public QWidget
{
    Q_OBJECT
public:
    NetWorkChart(QWidget *parent = 0);
    ~NetWorkChart();

    void setBgColor(QColor bgColor);

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    QColor m_downLoadColor;
    QColor m_upLoadColor;
#if 0
    QPainterPath m_downLoadPath;
    QPainterPath m_upLoadPath;
#endif
    int m_pointsCount;
    QColor m_bgColor;
    quint64 m_curMaxLoadSpeed;

    QList<quint64> m_downLoadList;
    QList<quint64> m_upLoadList;

    QChart *m_chart = nullptr;
    QChartView *m_chartView = nullptr;
    QLineSeries *m_upLineSeriesUpload = nullptr;
    QLineSeries *m_lowLineSeriesUpload = nullptr;
    QAreaSeries *m_areaSeriesUpload = nullptr;
    QLineSeries *m_upLineSeriesDnload = nullptr;
    QLineSeries *m_lowLineSeriesDnload = nullptr;
    QAreaSeries *m_areaSeriesDnload = nullptr;
    QValueAxis *m_valueAxisX = nullptr;
    QValueAxis *m_valueAxisY = nullptr;

public slots:
    void onUpdateDownloadAndUploadData(quint64,quint64,quint64,quint64);
signals:
//    void speedToMib();
    void speedToLowKib();
//    void speedToMiddleKib();
    void speedToHighKib();
    void speedToDynamicMax(quint64 lluMaxSpeed);
};

#endif //_NETWORKCHSRT_H_
