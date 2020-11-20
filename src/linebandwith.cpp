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

#include "linebandwith.h"

#include <QtDebug>

//int lineBandwith::countMap[0] = 0;


lineBandwith::lineBandwith(int pid,QWidget *parent) :QLineEdit(parent)

{
//    countMap = new QMap;
    time_prev = QDateTime::currentDateTime();
    count_prev = 0;
//    countMap[pid] = 0;
}

QString lineBandwith::new_count(qint64 count ,int pid)
{
    qint64 bandwith = count - countMap[pid];
//    qDebug() <<"bandwidth: "<< bandwith;
    QDateTime now = QDateTime::currentDateTime();
    quint64 ms_lapse = time_prev.msecsTo(now);

    if (ms_lapse < 1000) ms_lapse = 1000; // prevent division by 0 ;-)
    quint64 speed = bandwith * 1000 / ms_lapse;

    if (speed == 0 || speed < 0) {
        speedPerSec = "0 KB/S"; }
    else if (speed < 1900) {
        speedPerSec.setNum(speed);
        speedPerSec.append(" B/s"); }
    else if (speed < 1900000) {
        speedPerSec.setNum(speed/1024);
        speedPerSec.append(" kB/s"); }
    else if (speed < 1900000000) {
        speedPerSec.setNum(speed/(1024*1024));
        speedPerSec.append(" MB/s"); }
    else {
        speedPerSec.setNum(speed/(1024*1024*1024));
        speedPerSec.append(" GB/s");
    }
//    qDebug() << "sprrdPerSec: " << speedPerSec;
    time_prev.swap(now);
    countMap[pid] = count;
    return speedPerSec;
}
