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

#include <QApplication>
#include <QTranslator>
#include <QObject>
#include <QDesktopWidget>
#include <QtSingleApplication>

#include "framelessExtended/framelesshandle.h"
#include "systemmonitor.h"
#include "shell/customstyle.h"
//#include "maincontroller.h"

#include <X11/Xlib.h>   // should be put in the last

int main(int argc, char *argv[])
{
    Display *disp = XOpenDisplay(NULL);
    Screen *scrn = DefaultScreenOfDisplay(disp);
    if (NULL == scrn) {
        return 0;
    }
    int width = scrn->width;

    if (width > 2560) {
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
                QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
                QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
        #endif
    }
    if (NULL != disp) {
        XCloseDisplay(disp);
    }


    QString id = QString("ukui-system-monitor"+QLatin1String(getenv("DISPLAY")));
    QtSingleApplication app(id,argc,argv);
    if(app.isRunning())
    {
        app.sendMessage(QApplication::arguments().length() > 1 ? QApplication::arguments().at(1) : app.applicationFilePath());
        qDebug() << QObject::tr("ukui-system-monitor is already running!");
        return EXIT_SUCCESS;
    }
    else
    {
        QApplication::setApplicationName("Kylin System Monitor");
        QApplication::setApplicationVersion("0.0.0.0001");

        QString locale = QLocale::system().name();
        QTranslator translator;
        if(locale == "zh_CN" || locale == "es" || locale == "fr" || locale == "de" || locale == "ru") {//中文 西班牙语 法语 德语 俄语
            if(!translator.load("ukui-system-monitor_" + locale + ".qm",
                                ":/translation/"))
                qDebug() << "Load translation file："<< "ukui-system-monitor_" + locale + ".qm" << " failed!";
            else
                app.installTranslator(&translator);
        }

        SystemMonitor *monitor=new SystemMonitor();
//        monitor->setAttribute(Qt::WA_TranslucentBackground);
//        monitor->setProperty("useSystemStyleBlur",true);

        monitor->setAttribute(Qt::WA_DeleteOnClose);

        app.setActivationWindow(monitor);
        QObject::connect(&app, SIGNAL(messageReceived(const QString&)),monitor, SLOT(sltMessageReceived(const QString&)));

        monitor->show();
//        qDebug()<<qAppName()<<"app name";
        FramelessHandle * pHandle = new FramelessHandle(monitor);
        pHandle->activateOn(monitor);
        app.exec();
        return 0;
    }
}
