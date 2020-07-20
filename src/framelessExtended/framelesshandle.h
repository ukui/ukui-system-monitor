/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntukylin.com/kobe24_lixiang@126.com
 *  rxy     renxinyu@kylinos.cn
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
#ifndef FRAMELESSHANDLE_H
#define FRAMELESSHANDLE_H

#include <QObject>

class QWidget;
class FramelessHandlePrivate;

class FramelessHandle : public QObject
{
    Q_OBJECT

public:
    explicit FramelessHandle(QObject *parent = 0);
    ~FramelessHandle();

    void activateOn(QWidget * topLevelWidget);
    void setWidgetResizable(bool resizable);
    void setWidgetMovable(bool movable);
    void setBorderWidget(uint width);

    bool currentWidgetResizable();
    bool currentWidgetMovable();
    uint currentBorderWidth();

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event);

private:
    FramelessHandlePrivate * fpri;

};

#endif // FRAMELESSHANDLE_H
