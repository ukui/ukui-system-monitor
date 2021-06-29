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
#include <ukui-log4qt.h>

#include "framelessExtended/framelesshandle.h"
#include "systemmonitor.h"
#include "shell/customstyle.h"
#include "../shell/xatom-helper.h"
//#include "maincontroller.h"

#include <X11/Xlib.h>   // should be put in the last

#include <execinfo.h>
#include <unistd.h>
#define BUFF_SIZE  1024

static void crashHandler(int sig)
{
    signal(sig, SIG_IGN);
    size_t size = 0;
    char **strings = NULL;
    size_t i = 0, j = 0;

    char path[BUFF_SIZE] = {0};
    static char *homePath = getenv("HOME");
    snprintf(path, BUFF_SIZE, "%s/.config/ukui", homePath);
    strcat(path,"/ukui_sysmon_crash.log");
    FILE *fp = fopen(path,"a+");
    
    if (fp) {
        void *array[20];
        size = backtrace (array, 20);
        strings = (char **)backtrace_symbols (array, size);

        char logStr[BUFF_SIZE] = "0";
        sprintf(logStr,"!!!--- [%s]pid:%d received signal: %d=%s ---!!!\n version = %s, Stack trace\r\n", 
            QDateTime::currentDateTime().toString().toStdString().c_str(),getpid(),sig,strsignal(sig), "2.0.8");
        fwrite(logStr,sizeof(char),sizeof(logStr),fp);
        for (i = 0; i < size; i++)
        {
            memset(logStr,0,BUFF_SIZE);
            snprintf(logStr, BUFF_SIZE, "#%d\t%s \n",i,strings[i]);
            fwrite(logStr,sizeof(char),sizeof(logStr),fp);
        }

        fflush(fp);
        fclose(fp);
        free (strings);
    }
    exit(128 + sig);
}

static void registerSignals()
{
    if(signal(SIGCHLD,SIG_IGN)==SIG_ERR)//忽略子进程已经停止或退出
    {
        //注册SIGCHLD信号失败
        perror("signal error");
    }
    if(signal(SIGSEGV,crashHandler)==SIG_ERR)//无效内存段访问
    {
        //注册SIGSEGV信号失败
        perror("signal error");
    }
    if(signal(SIGILL,crashHandler)==SIG_ERR)//非法指令
    {
        //注册SIGILL信号失败
        perror("signal error");
    }
    if(signal(SIGTERM,crashHandler)==SIG_ERR)//终止
    {
        //注册SIGTERM信号失败
        perror("signal error");
    }
    if(signal(SIGHUP,crashHandler)==SIG_ERR)//系统挂断
    {
        //注册SIGHUP信号失败
        perror("signal error");
    }
    if(signal(SIGABRT,crashHandler)==SIG_ERR)//进程停止运行
    {
        //注册SIGABRT信号失败
        perror("signal error");
    }
    if(signal(SIGKILL,crashHandler)==SIG_ERR)//终止
    {
        //注册SIGKILL信号失败
        perror("signal error");
    }
}

int main(int argc, char *argv[])
{
    registerSignals();
    initUkuiLog4qt("ukui-system-monitor");
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
      QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
      QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    #endif
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
      QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    #endif

    QString id = QString("ukui-system-monitor"+QLatin1String(getenv("DISPLAY")));
    QtSingleApplication app(id,argc,argv);
    if(app.isRunning())
    {
        app.sendMessage(QApplication::arguments().length() > 1 ? QApplication::arguments().at(1) : app.applicationFilePath());
        qInfo() << QObject::tr("ukui-system-monitor is already running!");
        return EXIT_SUCCESS;
    }
    else
    {
        QApplication::setApplicationName("Kylin System Monitor");
        QApplication::setApplicationVersion("0.0.0.0001");

        QString locale = QLocale::system().name();
        QTranslator translator;
        if(locale == "zh_CN" || locale == "es" || locale == "fr" || locale == "de" 
            || locale == "ru" || locale == "en_US") {//中文 西班牙语 法语 德语 俄语 英语
            if(!translator.load("ukui-system-monitor_" + locale + ".qm",
                                ":/translation/"))
                qDebug() << "Load translation file："<< "ukui-system-monitor_" + locale + ".qm" << " failed!";
            else
            {
                app.installTranslator(&translator);
                qDebug()<<"load success";
            }
        }

        SystemMonitor monitor;

        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(monitor.winId(), hints);

        app.setActivationWindow(&monitor);
        app.setWindowIcon(QIcon::fromTheme("ukui-system-monitor"));
        QObject::connect(&app, SIGNAL(messageReceived(const QString&)),&monitor, SLOT(sltMessageReceived(const QString&)));
        monitor.show();
        return app.exec();
    }
}
