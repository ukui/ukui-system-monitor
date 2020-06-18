#include <QApplication>
#include <QTranslator>
#include <QObject>

#include "framelessExtended/framelesshandle.h"
#include "systemmonitor.h"
#include "shell/customstyle.h"

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




    SystemMonitor *monitor=new SystemMonitor();
    monitor->setAttribute(Qt::WA_DeleteOnClose);
    monitor->show();

    FramelessHandle * pHandle = new FramelessHandle(monitor);
    pHandle->activateOn(monitor);

    auto style = new InternalStyle(nullptr);
    app.setStyle(style);

    app.exec();
//    delete monitor;
    return 0;
}
