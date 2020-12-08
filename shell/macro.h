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
#ifndef QWER_H
#define QWER_H

/**
  *DBus
  */
#define KYLIN_USER_GUIDE_PATH "/"
#define KYLIN_USER_GUIDE_SERVICE "com.kylinUserGuide.hotel"
#define KYLIN_USER_GUIDE_INTERFACE "com.guide.hotel"

/**
 * QT主题
 */
#define THEME_QT_SCHEMA "org.ukui.style"
#define MODE_QT_KEY "style-name"
#define FONT_SIZE "system-font-size"
#define THEME_QT_TRANS "org.ukui.control-center.personalise"
/* QT图标主题 */
#define ICON_QT_KEY "icon-theme-name"

/*
 * about the size of every single control
 */
#define NORMALHEIGHT 30
#define SPECIALWIDTH 200
#define NORMALWIDTH 100
#define MAINWINDOWHEIGHT 590
#define MAINWINDOWWIDTH 760
#define SEARCHBUTTON 16
#define DEFAULT_FONT_SIZE 11

#define SERVICE_NAME_SIZE 64
#define UKUI_SYSTEM_MONITOR_PATH "/"
#define UKUI_SYSTEM_MONITOR_SERVICE "com.ukuisystemmonitor.hotel"
#define UKUI_SYSTEM_MONITOR_INTERFACE "com.systemmonitor.hotel"

#define PADDING 1.5
#define POINTSPACE 10

enum SIGTYPE
{
    REDTYPE = 0,PURPLETYPE,GREENTYPE,BLUETYPE,YELLOWTYPE
};

#endif
