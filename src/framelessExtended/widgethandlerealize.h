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
#ifndef WIDGETHANDLEREALIZE_H
#define WIDGETHANDLEREALIZE_H

#include <QWidget>

#include <QEvent>
#include <QMouseEvent>
#include <QHoverEvent>

#include "framelesshandleprivate.h"
#include "cursorposcalculator.h"

class WidgetHandleRealize : public QWidget
{

public:
    explicit WidgetHandleRealize(FramelessHandlePrivate * _fpri, QWidget *pTopLevelWidget);
    ~WidgetHandleRealize();

public:
    QWidget * currentWidget();

    void handleWidgetEvent(QEvent * event);

private:
    void updateCursorShape(const QPoint &gMousePos);
    void resizeWidget(const QPoint &gMousePos);
    void moveWidget(const QPoint &gMousePos);
    void handleMousePressEvent(QMouseEvent * event);
    void handleMouseReleaseEvent(QMouseEvent * event);
    void handleMouseMoveEvent(QMouseEvent * event);
    void handleLeaveEvent(QEvent * event);
    void handleHoverMoveEvent(QHoverEvent * event);

    void moveMainWindow(void);

private:
    FramelessHandlePrivate * fpri;
    QWidget * widgetInAction;
    QPoint dragPos;

    CursorPosCalculator pressedMousePos;
    CursorPosCalculator moveMousePos;

    bool leftBtnPressed;
    bool cursorShapeChanged;

    Qt::WindowFlags currentWindowFlags;

};

#endif // WIDGETHANDLEREALIZE_H
