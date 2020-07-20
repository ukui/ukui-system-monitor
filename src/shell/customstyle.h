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
#ifndef CUSTOMSTYLE_H
#define CUSTOMSTYLE_H

#include <QProxyStyle>

/*!
 * \brief The InternalStyle class
 * \note
 * InternalStyle is a special style naming rule of qt5-ukui-platformtheme.
 * It tells the platform do not take care of the style management of application
 * when an application set the style as default style.
 *
 * Do not modify this class name, otherwise the style will changed to default system
 * style when system style changed.
 *
 * InternalStyle是qt5-ukui-platformtheme的特殊命名规则，
 * 它用于告知平台插件不接管系统主题的管理（前提是将其设置为qApp的默认主题）。
 *
 * 不要修改这个类名，否则切换系统主题时，应用定制的应用主题会被系统主题覆盖。
 */
class InternalStyle : public QProxyStyle
{
    Q_OBJECT
public:
    explicit InternalStyle(const QString &styleName, QObject *parent = nullptr);
    ~InternalStyle();

    void drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    void drawControl(QStyle::ControlElement element,
                     const QStyleOption *option,
                     QPainter *painter,
                     const QWidget *widget = nullptr) const;

    void drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const;

    void polish(QPalette &pal);
    void polish(QWidget *widget);
    void unpolish(QWidget *widget);

signals:

};

#endif // CUSTOMSTYLE_H
