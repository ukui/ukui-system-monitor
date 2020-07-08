#ifndef _MAINCONTROL_H_
#define _MAINCONTROL_H_

#include <QObject>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusConnectionInterface>
#include <unistd.h>

#include "systemmonitor.h"
#include "../shell/macro.h"
#include "framelessExtended/framelesshandle.h"

class MainController : public QObject
{
    Q_OBJECT
public:
    static MainController* self();
    virtual ~MainController();
private:
    explicit MainController();
    void init();
    void creatDBusService();
    int IsNotRunning();
private:
    static MainController *mSelf;
    SystemMonitor * monitor;
};

#endif //_MAINCONTROL_H_
