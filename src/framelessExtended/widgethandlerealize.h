/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
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
