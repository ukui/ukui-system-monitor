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

#include "sigcheck.h"

sigCheck::sigCheck(QWidget *parent,int whichSig)
    : QWidget(parent),sig(whichSig)
{
    this->setFixedSize(14,14);
}

sigCheck::~sigCheck()
{
    
}

void sigCheck::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);

    if(sig == REDTYPE)
    {
        p.setBrush(QBrush(QColor(246,79,61)));
    }

    if(sig == PURPLETYPE)
    {
        p.setBrush(QBrush(QColor(204,72,255)));
    }

    if(sig == GREENTYPE)
    {
        p.setBrush(QBrush(QColor(26,195,161)));
    }

    if(sig == BLUETYPE)
    {
        p.setBrush(QBrush(QColor(42,177,232)));
    }

    if(sig == YELLOWTYPE)
    {
        p.setBrush(QBrush(QColor(241,191,48)));
    }

    p.setPen(Qt::NoPen);
    QPainterPath path;
    opt.rect.adjust(0,0,0,0);

    path.addRoundedRect(opt.rect,2,2);
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    p.drawRoundedRect(opt.rect,2,2);
    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
