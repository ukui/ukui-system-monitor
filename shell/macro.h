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

// process table view backup setting key
#define SETTINGSOPTION_PROCESSTABLEHEADERSTATE "process_table_header_state"

// filesystem table view backup setting key
#define SETTINGSOPTION_FILESYSTEMTABLEHEADERSTATE "filesystem_table_header_state"

/* QT图标主题 */
#define ICON_QT_KEY "icon-theme-name"

/*
 * about the size of every single control
 */
#define NORMALHEIGHT 34
#define SPECIALWIDTH 200
#define NORMALWIDTH 115
#define MAINWINDOWHEIGHT 650
#define MAINWINDOWWIDTH 760
#define SEARCHBUTTON 16
#define DEFAULT_FONT_SIZE 11

#define SERVICE_NAME_SIZE 64
//#define UKUI_SYSTEM_MONITOR_PATH "/"
//#define UKUI_SYSTEM_MONITOR_SERVICE "com.ukuisystemmonitor.hotel"
//#define UKUI_SYSTEM_MONITOR_INTERFACE "com.systemmonitor.hotel"

#define PADDING 1.5
#define POINTSPACE 10


//process setting
#define namepadding 140
#define userpadding 90
#define diskpadding 80
#define cpupadding 80
#define idpadding 80
#define networkpadding 80
#define memorypadding 100
#define prioritypadding 100
#define direciconposition 100

//file system setting
#define devicepadding 120
#define mounturiadding 150
#define typepadding 80
#define totalcapacitypadding 100
#define idlepadding 100
#define avaliablepadding 100
#define usedpadding 100

enum SIGTYPE
{
    REDTYPE = 0,
    PURPLETYPE,
    GREENTYPE,
    BLUETYPE,
    YELLOWTYPE
};

// custom proc map config
#define UKUI_SYSTEM_MONITOR_CONF  "/usr/share/ukui/ukui-system-monitor.conf"

#endif
