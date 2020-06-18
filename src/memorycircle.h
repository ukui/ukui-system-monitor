/*
 * Copyright (C) 2013 ~ 2018 National University of Defense Technology(NUDT) & Tianjin Kylin Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntukylin.com/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MEMORYCIRCLE_H
#define MEMORYCIRCLE_H

#include <QWidget>
#include <QPainter>
#include <glibtop/mem.h>
#include <glibtop/swap.h>

typedef struct MemoryInfo
{
    float total;
    float free;
    float cached;
    float used;
    float user;
    float percent;

    float swaptotal;
    float swapfree;
    float swapcached;
    float swapused;
    float swappercent;

}Meminfo;

class MemoryCircle : public QWidget
{
    Q_OBJECT

public:
    MemoryCircle(QWidget *parent = 0);
    ~MemoryCircle();

    void drawCircle(QPainter &painter, bool isSwap = false);
    void drawColorPie(QPainter &painter, bool isSwap = false);
    void drawTextInfo(QPainter &painter);

public slots:
    void onUpdateMemoryStatus();

signals:
    void rebackMemoryInfo(const QString &info, double percent);

protected:
    void paintEvent(QPaintEvent * event);

private:
    qreal circleRadius;
    qreal rectWidth;
    qreal rectHeight;
    qreal colorPieRadius;
    QPointF center;
    QPointF swapcenter;
    QRectF pieRect;
    QRectF swappieRect;
    Meminfo mi;
};

#endif // MEMORYCIRCLE_H
