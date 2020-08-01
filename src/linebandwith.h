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

#ifndef LINEBANDWITH_H
#define LINEBANDWITH_H

#include "QLineEdit"

#include "QDateTime"

class lineBandwith : public QLineEdit
{
    Q_OBJECT

public:
    explicit lineBandwith(int pid,QWidget *parent = 0);

    QString new_count(qint64 count ,int pid);

private:
    qint64 count_prev;
    QDateTime time_prev;
    QString speedPerSec;
    QMap <int,int> countMap;

signals:

public slots:

};

#endif // LINEBANDWITH_H
