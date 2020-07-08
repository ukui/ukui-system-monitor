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

#include "maincontroller.h"

MainController* MainController::mSelf = 0;  //static variable
MainController* MainController::self()      //static function    //complete the singleton object
{
    if (!mSelf)
    {
        mSelf = new MainController;
    }
    return mSelf;
}

MainController::MainController()
{
    init();
    monitor=new SystemMonitor();
    monitor->setAttribute(Qt::WA_DeleteOnClose);
    monitor->show();
    FramelessHandle * pHandle = new FramelessHandle(monitor);
    pHandle->activateOn(monitor);
}

MainController::~MainController()
{
}

void MainController::init()                   //init select
{
    if(IsNotRunning())
    {
        creatDBusService();                 //create connect
        qDebug()<<"--------------creatDBusService";
    }
    else
    {
        qDebug()<<"ukui-flash-disk is running";  //or finish the process
        exit(0);
    }
}

int MainController::IsNotRunning()
{
    //determine the session bus that if it has been connected
    char service_name[SERVICE_NAME_SIZE];
    memset(service_name, 0, SERVICE_NAME_SIZE);
    snprintf(service_name, SERVICE_NAME_SIZE, "%s_%d",UKUI_SYSTEM_MONITOR_SERVICE,getuid());
    QDBusConnection conn = QDBusConnection::sessionBus();
    if (!conn.isConnected())
        return 0;

    QDBusReply<QString> reply = conn.interface()->call("GetNameOwner", service_name);
    return reply.value() == "";
}

void MainController::creatDBusService()
{
    // 用于建立到session bus的连接
    //to be used for creating the session bus connection
    QDBusConnection bus = QDBusConnection::sessionBus();
    // 在session bus上注册名为"com.kylin_user_guide.hotel"的service
    // register the service in session bus that named by "com.kylin_user_guide.hotel"

    char service_name[SERVICE_NAME_SIZE];
    memset(service_name, 0, SERVICE_NAME_SIZE);
    snprintf(service_name, SERVICE_NAME_SIZE, "%s_%d",UKUI_SYSTEM_MONITOR_SERVICE,getuid());

    if (!bus.registerService(service_name))
    {  //注意命名规则-和_
       //Note the naming convention.
            qDebug() << bus.lastError().message();
            exit(1);
    }
    // "QDBusConnection::ExportAllSlots"表示把类Hotel的所有Slot都导出为这个Object的method
    // "QDBusConnection::ExportAllSlots" stands for that it makes all the slot in class hotel export to the method of this object
    bus.registerObject("/", this ,QDBusConnection::ExportAllSlots);
}
