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

#include <QApplication>
#include <QTranslator>
#include <QObject>

#include "framelessExtended/framelesshandle.h"
#include "systemmonitor.h"
#include "shell/customstyle.h"
#include "maincontroller.h"

int main(int argc, char *argv[])
{
    QApplication app(argc,argv);

    QApplication::setApplicationName("Kylin System Monitor");
    QApplication::setApplicationVersion("0.0.0.0001");

    QString locale = QLocale::system().name();
    QTranslator translator;
    if(locale == "zh_CN" || locale == "es" || locale == "fr" || locale == "de" || locale == "ru") {//中文 西班牙语 法语 德语 俄语
        if(!translator.load("kylin-assistant_" + locale + ".qm",
                            ":/translation/"))
            qDebug() << "Load translation file："<< "kylin-assistant_" + locale + ".qm" << " failed!";
        else
            app.installTranslator(&translator);
    }




//    SystemMonitor *monitor=new SystemMonitor();
//    monitor->setAttribute(Qt::WA_DeleteOnClose);
//    monitor->show();

//    FramelessHandle * pHandle = new FramelessHandle(monitor);
//    pHandle->activateOn(monitor);

    auto style = new InternalStyle(nullptr);
    app.setStyle(style);

    MainController *ctrl = MainController::self();

    app.exec();
//    delete monitor;
    return 0;
}
