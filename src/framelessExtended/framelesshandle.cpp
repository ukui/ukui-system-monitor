/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntukylin.com/kobe24_lixiang@126.com
 *  rxy         renxinyu@kylinos.cn
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
#include "framelesshandle.h"

#include <QEvent>

#include "framelesshandleprivate.h"
#include "widgethandlerealize.h"

FramelessHandle::FramelessHandle(QObject *parent) :
    QObject(parent),
    fpri(new FramelessHandlePrivate())
{
    fpri->widgetResizable = true;
    fpri->widgetMovable = true;
}

FramelessHandle::~FramelessHandle()
{
    QList<QWidget *> widgets = fpri->widgethandleHash.keys();
    for (int i = 0; i < widgets.size(); i++){
        delete fpri->widgethandleHash.take(widgets[i]);
    }
    delete fpri;
}

bool FramelessHandle::eventFilter(QObject *watched, QEvent *event){
    switch (event->type()){

    case QEvent::MouseMove:
    case QEvent::HoverMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::Leave:{
        WidgetHandleRealize * widgethandleobj = fpri->widgethandleHash.value(static_cast<QWidget *>(watched));
        if (widgethandleobj){
            widgethandleobj->handleWidgetEvent(event);
            return true;
        }
    }
    default:
        break;
    }
    return QObject::eventFilter(watched, event);
}

void FramelessHandle::activateOn(QWidget *topLevelWidget){
    if (!fpri->widgethandleHash.contains(topLevelWidget)){
        WidgetHandleRealize * widgethandleobj = new WidgetHandleRealize(fpri, topLevelWidget);
        fpri->widgethandleHash.insert(topLevelWidget, widgethandleobj);

        topLevelWidget->installEventFilter(this);
    }
}

void FramelessHandle::setWidgetResizable(bool resizable){
    fpri->widgetResizable = resizable;
}

void FramelessHandle::setWidgetMovable(bool movable){
    fpri->widgetMovable = movable;
}

void FramelessHandle::setBorderWidget(uint width){
    if (width > 0)
        CursorPosCalculator::borderWidth = width;
}

bool FramelessHandle::currentWidgetResizable(){
    return fpri->widgetResizable;
}

bool FramelessHandle::currentWidgetMovable(){
    return fpri->widgetMovable;
}

uint FramelessHandle::currentBorderWidth(){
    return CursorPosCalculator::borderWidth;
}
