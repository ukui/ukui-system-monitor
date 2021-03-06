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

#include "processdialog.h"
#include "propertiesdialog.h"
#include "processdata.h"
#include "processcategory.h"
//#include "renicedialog.h"
#include "util.h"

#include <QStringList>
#include <QCloseEvent>
#include <QBitmap>
#include <QPainter>
#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <QHeaderView>
#include <QDesktopServices>
#include <QApplication>
#include <QHBoxLayout>
#include <QList>
#include <QProcess>
#include <QStyleFactory>
#include <QToolTip>
#include <QProcess>

#include <unistd.h>
#include <systemd/sd-login.h>
#include <set>
#include <list>
#include <glibtop.h>
#include <glibtop/procio.h>
#include <glibtop/proclist.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <glibtop/procstate.h>
#include <glibtop/procmem.h>
#include <glibtop/procmap.h>
#include <glibtop/proctime.h>
#include <glibtop/procuid.h>
#include <glibtop/procargs.h>
#include <glibtop/prockernel.h>
#include <glibtop/sysinfo.h>
#include <pwd.h>
#include <glibtop/mem.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/resource.h> //for setpriority


using std::string;

QDataStream &operator<<(QDataStream &dataStream, const ProcDataPtr &object)
{
    auto ptr = object.data();
    auto ptrval = reinterpret_cast<qulonglong>(ptr);
    auto var = QVariant::fromValue(ptrval);
    dataStream << var;
    return  dataStream;
}

QDataStream &operator>>(QDataStream &dataStream, ProcDataPtr &object)
{
    QVariant var;
    dataStream >> var;
    qulonglong ptrval = var.toULongLong();
    auto ptr = reinterpret_cast<ProcData *>(ptrval);
    object = ProcDataPtr(ptr);
    return dataStream;
}

ProcessDialog::ProcessDialog(QList<bool> toBeDisplayedColumns, int currentSortIndex, bool isSort, QSettings *settings, QWidget *parent)
    :QWidget(parent)
    ,num_cpus(0)
    ,frequency(0U)
    ,proSettings(settings)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setAttribute(Qt::WA_NoMousePropagation);

    qRegisterMetaType<ProcDataPtr>();    //注册信号槽，实现线程间通信
    qRegisterMetaTypeStreamOperators<ProcDataPtr>();
    qRegisterMetaType<ProcDataPtrList>();
    qRegisterMetaType<QList<ProcData>>();

    actionPids = new QList<pid_t>();

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_processListWidget = new ProcessListWidget(toBeDisplayedColumns);
    connect(m_processListWidget, SIGNAL(changeColumnVisible(int,bool,QList<bool>)), this, SIGNAL(changeColumnVisible(int,bool,QList<bool>)));
    connect(m_processListWidget, SIGNAL(changeSortStatus(int,bool)), this, SIGNAL(changeSortStatus(int,bool)));
    connect(m_processListWidget, &ProcessListWidget::rightMouseClickedItems, this, &ProcessDialog::popupMenu, Qt::QueuedConnection);
    m_layout->addWidget(m_processListWidget);

//记录要显示进程的内容类别
    whose_processes = "user";
    proSettings->beginGroup("PROCESS");
    whose_processes = proSettings->value("WhoseProcesses", whose_processes).toString();
    proSettings->endGroup();

    QList<SortFunction> *sortFuncList = new QList<SortFunction>();
    sortFuncList->append(&ProcessListItem::sortByName);
    sortFuncList->append(&ProcessListItem::sortByUser);
    sortFuncList->append(&ProcessListItem::sortByDiskIo);
    sortFuncList->append(&ProcessListItem::sortByCPU);
    sortFuncList->append(&ProcessListItem::sortByPid);
    sortFuncList->append(&ProcessListItem::sortByFlowNet);
    sortFuncList->append(&ProcessListItem::sortByMemory);
    sortFuncList->append(&ProcessListItem::sortByPriority);

    m_processListWidget->setProcessSortFunctions(sortFuncList, currentSortIndex, isSort);
    m_processListWidget->setSearchFunction(&ProcessListItem::doSearch);
    m_menu = new QMenu();
    m_menu->setObjectName("MonitorMenu");
    m_stopAction = new QAction(tr("Stop process"), this);
    connect(m_stopAction, &QAction::triggered, this, &ProcessDialog::stopProcesses);
    m_continueAction = new QAction(tr("Continue process"), this);
    connect(m_continueAction, &QAction::triggered, this, &ProcessDialog::continueProcesses);
    m_endAction = new QAction(tr("End process"), this);
    connect(m_endAction, &QAction::triggered, this, &ProcessDialog::showEndProcessDialog);
    m_killAction = new QAction(tr("Kill process"), this);
    connect(m_killAction, &QAction::triggered, this, &ProcessDialog::showKillProcessDialog);

    priorityGroup = new MyActionGroup(this);
    veryHighAction = new MyActionGroupItem(this, priorityGroup, "very_high_action", -20);
    highAction = new MyActionGroupItem(this, priorityGroup, "high_action", -5);
    normalAction = new MyActionGroupItem(this, priorityGroup, "normal_action", 0);
    lowAction = new MyActionGroupItem(this, priorityGroup, "low_action", 5);
    veryLowAction = new MyActionGroupItem(this, priorityGroup, "very_low_action", 19);
    customAction = new MyActionGroupItem(this, priorityGroup, "custom_action", 32);
    {
        QAction *sep = new QAction(priorityGroup);
        sep->setSeparator(true);
    }
    veryHighAction->change(tr("Very High"));
    highAction->change(tr("High"));
    normalAction->change(tr("Normal"));
    lowAction->change(tr("Low"));
    veryLowAction->change(tr("Very Low"));
    customAction->change(tr("Custom"));
    connect(priorityGroup, SIGNAL(activated(int)), this, SLOT(changeProcPriority(int)));
    m_priorityMenu = new QMenu();
    m_priorityMenu->setObjectName("MonitorMenu");
    m_priorityMenu->addActions(priorityGroup->actions());
    m_priorityMenu->menuAction()->setText(tr("Change Priority"));

    m_propertiyAction = new QAction(tr("Properties"), this);
    connect(m_propertiyAction, &QAction::triggered, this, &ProcessDialog::showPropertiesDialog);

    m_menu->addAction(m_stopAction);//停止
    m_menu->addAction(m_continueAction);//继续进程
    m_menu->addAction(m_endAction);//结束
    m_menu->addAction(m_killAction);//杀死
    m_menu->addSeparator();
    m_menu->addMenu(m_priorityMenu);
    m_menu->addSeparator();
    m_menu->addAction(m_propertiyAction);

    glibtop_init();
    this->num_cpus = glibtop_get_sysinfo()->ncpu;

    scanThread = new ScanThread(this);
    scanThread->start(QThread::TimeCriticalPriority);

    refreshThread = new RefreshThread(this);
    refreshThread->start(QThread::HighPriority);

    connect(refreshThread, SIGNAL(procDetected(const QString &, quint64 , quint64 , int , unsigned int , const QString&)),
            this, SLOT(refreshLine(const QString &, quint64 , quint64 , int, unsigned int , const QString&)));
    connect(this,SIGNAL(changeProcessNetRefresh()),this,SLOT(onSearchFocusIn()));
    connect(this,SIGNAL(recoverProcessNetRefresh()),this,SLOT(onSearchFocusOut()));

    this->refreshProcessList();
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(refreshProcessList()));
    timer->start(1500);
}

ProcessDialog::~ProcessDialog()
{
    glibtop_close();
    this->clearOriginProcList();

    if (timer != NULL) {
        disconnect(timer,SIGNAL(timeout()),this,SLOT(refreshProcessList()));
        if(timer->isActive()) {
            timer->stop();
        }
        delete timer;
        timer = NULL;
    }
    delete processCategory;
    delete endProcessDialog;
    delete killProcessDialog;
    delete m_processListWidget;
    delete m_stopAction;
    delete m_continueAction;
    delete m_endAction;
    delete m_killAction;

    delete veryHighAction;
    delete highAction;
    delete normalAction;
    delete lowAction;
    delete veryLowAction;
    delete customAction;
    delete m_priorityMenu;

    delete m_propertiyAction;
    delete m_menu;
    delete actionPids;
    delete item;

    if(scanThread)
    {
        delete scanThread;
    }

    if(refreshThread)
    {
        delete refreshThread;
    }

    QLayoutItem *child;
    while ((child = m_categoryLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
    delete m_layout;
}

void ProcessDialog::displayAllProcess()
{
    timer->stop();
    this->clearOriginProcList();
    whose_processes = "all";
    this->refreshProcessList();
    timer->start(3000);
}

void ProcessDialog::displayActiveProcess()
{
    timer->stop();
    this->clearOriginProcList();
    whose_processes = "active";
    this->refreshProcessList();
    timer->start(3000);
}

void ProcessDialog::displayCurrentUserProcess()
{
    timer->stop();
    this->clearOriginProcList();
    whose_processes = "user";
    this->refreshProcessList();
    timer->start(3000);
}

void ProcessDialog::onActiveWhoseProcess(int index)
{
    if (index == 0) {
        if (this->whose_processes != "active")
            this->displayActiveProcess();
    } else if (index == 1) {
        if (this->whose_processes != "user")
            this->displayCurrentUserProcess();
    } else {
        if (this->whose_processes != "all")
            this->displayAllProcess();
    }

    proSettings->beginGroup("PROCESS");
    proSettings->setValue("WhoseProcesses", whose_processes);
    proSettings->endGroup();
    proSettings->sync();
}

void ProcessDialog::clearOriginProcList()
{
    for (ProcessWorker::Iterator it(ProcessWorker::begin()); it != ProcessWorker::end(); ++it)
        delete it->second;
    ProcessWorker::all.clear();
}

void ProcessDialog::changeProcPriority(int nice)
{
    qDebug()<<"nice value"<<nice;
    if (nice == 32) {
        //show renice dialog
        pid_t cur_pid = -1;
        for (pid_t pid : *actionPids) {
            cur_pid = pid;
            break;
        }
        if (cur_pid > -1) {
            ProcessWorker *info = ProcessWorker::find(cur_pid);
            if (!info) {
                actionPids->clear();
                return;
            }
            QString name = QString::fromStdString(info->name);
            /*ReniceDialog **/w = new ReniceDialog(tr("Change Priority of Process %1 (PID: %2)").arg(name).arg(QString::number(cur_pid)));
            w->loadData(info->nice);
            connect(w, &ReniceDialog::resetReniceValue, [=] (int value) {
                this->changeProcPriority(value);
            });
            w->exec();
        }
    }
    else {
        qDebug()<<"has entered";
        pid_t cur_pid = -1;
        for (pid_t pid : *actionPids) {
            cur_pid = pid;
            break;
        }
        if (cur_pid > -1) {
            ProcessWorker *info = ProcessWorker::find(cur_pid);
            if (!info) {
                actionPids->clear();
                return;
            }
            if (info->nice == nice) {
                actionPids->clear();
                return;
            }
            qDebug()<<"pkexec begin";
//            int saved_errno;
//            int error = setpriority(PRIO_PROCESS, cur_pid, nice);
//            qDebug()<<"error num"<<error;
//            //success
//            if(error != -1)  {
//                actionPids->clear();
//                return;
//            }
//            saved_errno = errno;

//            //need to be root
//            if(errno == EPERM || errno == EACCES) {
                qDebug() << "Change priority need to be root!!!";
                /*
                 * renice: sudo apt install bsdutils
                 * Maybe: QProcess::startDetached(command)
                 * QProcess::start()与QProcess::execute()都能完成启动外部程序的任务，区别在于start()是非阻塞的，而execute()是阻塞的: execute()=start()+waitforFinished()
                */

                if (QFileInfo("/usr/bin/pkexec").exists()) {//sudo apt install policykit-1
                    QProcess process;
                    process.execute(QString("pkexec --disable-internal-agent %1 %2 %3").arg("renice").arg(nice).arg(cur_pid));
                    /*process.start(QString("pkexec --disable-internal-agent %1 %2 %3").arg("renice").arg(nice).arg(cur_pid));
                    process.waitForStarted(1000);
                    process.waitForFinished(20*1000);*/
                }
                else if (QFileInfo("/usr/bin/gksudo").exists()) {//sudo apt install gksu
                    QProcess process;
                    process.execute(QString("gksudo \"%1 %2 %3\"").arg("renice").arg(nice).arg(cur_pid));
                    /*process.start(QString("gksudo \"%1 %2 %3\"").arg("renice").arg(nice).arg(cur_pid));
                    process.waitForStarted(1000);
                    process.waitForFinished(20*1000);*/
                }
                else if (QFileInfo("/usr/bin/gksu").exists()) {//sudo apt install gksu
                    QProcess process;
                    process.execute(QString("gksu \"%1 %2 %3\"").arg("renice").arg(nice).arg(cur_pid));
//                    process.start(QString("gksu \"%1 %2 %3\"").arg("renice").arg(nice).arg(cur_pid));
//                    process.waitForStarted(1000);
//                    process.waitForFinished(20*1000);
                }
                else {
                    //
                }
            }
        }
}

//void ProcessDialog::onCloseButtonClicked()
//{
//    this->close();
//}

//void ProcessDialog::closeEvent(QCloseEvent *event)
//{
//    event->accept();
//}

void ProcessDialog::refreshProcessList()
{
    pid_t* pid_list;
    glibtop_proclist proclist;
    int which = 0;
    int arg = 0;

    if (whose_processes == "all")
    {
        which = GLIBTOP_KERN_PROC_ALL;
        arg = 0;
    }
    else if (whose_processes == "active")
    {
        which = GLIBTOP_KERN_PROC_ALL | GLIBTOP_EXCLUDE_IDLE;
        arg = 0;
    }
    else if (whose_processes == "user")
    {
        which = GLIBTOP_KERN_PROC_UID;
        arg = getuid();
    }

    pid_list = glibtop_get_proclist(&proclist, which, arg);

    // FIXME: not sure if glibtop always returns a sorted list of pid
    // but it is important otherwise refresh_list won't find the parent
    std::sort(pid_list, pid_list + proclist.number);

    typedef std::list<ProcessWorker*> ProcList;
    ProcList addition;
    guint i;
    for(i = 0; i < proclist.number; ++i)
    {
        ProcessWorker *info = ProcessWorker::find(pid_list[i]);

//        qDebug()<<"Map size: "<< pidMap.size() << info;
        if (info)
        {
            if(pidMap.contains(pid_list[i]))
            {
                this->addFlowNetPerSec = pidMap[pid_list[i]];
                info = new ProcessWorker(pid_list[i], this->num_cpus, this->cpu_total_time,this->addFlowNetPerSec);
                ProcessWorker::all[info->pid] = info;
                info->mNumFlownet = numAddFlowNetPerSec;
            }
        }
        //不存在时创建该进程的对象
        //create the process object when the pid's object doesn't exist
        else
        {
            info = new ProcessWorker(pid_list[i], this->num_cpus, this->cpu_total_time, "0 KB/S");
            ProcessWorker::all[info->pid] = info;
            info->mNumFlownet = 0;
        }
        //当进程对象存在时，更新该进程对象的相关数据信息
        glibtop_proc_state procstate;
        glibtop_proc_uid procuid;
        glibtop_proc_time proctime;
        glibtop_proc_io procio;

//        info->mNumFlownet = numAddFlowNetPerSec;

        glibtop_get_proc_io(&procio,info->pid);
        glibtop_get_proc_state (&procstate, info->pid);
        info->status = procstate.state;

        glibtop_get_proc_uid (&procuid, info->pid);
        glibtop_get_proc_time (&proctime, info->pid);

        glibtop_proc_mem procmem;
        glibtop_get_proc_mem(&procmem, info->pid);
        info->mem = procmem.resident - procmem.share;

        glibtop_get_proc_state(&procstate, info->pid);
        info->status = procstate.state;

        info->set_user(procstate.uid);

        /* FIXME: total cpu time elapsed should be calculated on an individual basis here
        ** should probably have a total_time_last gint in the ProcInfo structure */
        glibtop_cpu cpu;
        glibtop_get_cpu(&cpu);

        this->cpu_total_time = cpu.total;
        this->process_total_time = proctime.rtime;

//        if(prevCpuTime.size() == 0)
//        {
//            prevCpuTime.insert(0,0);;
//        }

//        if(prevCpuTime.size() >= 2)
//        {
//            prevCpuTime.pop_back();
//        }

//        if(prevProcessTime.size() == 0)
//        {
//            prevProcessTime.insert(0,0);
//        }

//        if(prevProcessTime.size() >=2 )
//        {
//            prevProcessTime.pop_back();
//        }


//        qDebug()<<"length"<<prevCpuTime.size()<<"--"<<prevProcessTime.size();
//        info->pcpu = (process_total_time - prevProcessTime.first())/MAX(cpu_total_time - prevCpuTime.first(),1);

//        prevCpuTime.insert(0,cpu_total_time);
//        prevProcessTime.insert(0,process_total_time);
        this->cpu_total_time = MAX(cpu.total - this->cpu_total_time_last, 1);
        this->cpu_total_time_last = cpu.total;
        guint64 difference = proctime.utime + proctime.stime + proctime.cutime + proctime.cstime - info->cpu_time ;
        if (difference > 0)
            info->status = GLIBTOP_PROCESS_RUNNING;
        info->pcpu = difference / this->cpu_total_time;
        info->pcpu = MIN(info->pcpu/2, 100);
        info->pcpu *= /*info->pcpu **/ this->num_cpus /10.0;
//        CPU 百分比使用 Solaris 模式，工作在“Solaris 模式”，其中任务的 CPU 使用量将被除以总的 CPU 数目。否则它将工作在“Irix 模式”。
        this->frequency = cpu.frequency;
        info->frequency = this->frequency;

        ProcessWorker::cpu_times[info->pid] = info->cpu_time = proctime.utime + proctime.stime + proctime.cutime + proctime.cstime;
        info->nice = procuid.nice;
        if(!calDiskIoMap.contains(info->pid))
        {
            calDiskIoMap[info->pid] = 0;
        }

        speedLineBandDiskIo = new lineBandwith(info->pid,this);

        disk_io_bytes_total = procio.disk_wbytes+procio.disk_rbytes;

        addDiskIoPerSec = speedLineBandDiskIo->new_count(disk_io_bytes_total-calDiskIoMap[info->pid],info->pid);

        info->diskio_persec = addDiskIoPerSec;

        info->mNumDiskIo = disk_io_bytes_total-calDiskIoMap[info->pid];

        calDiskIoMap[info->pid] = disk_io_bytes_total;

        delete speedLineBandDiskIo;
    }

    // Remove dead processes from the process list and from the
    // tree. children are queued to be readded at the right place
    // in the tree.
    const std::set<pid_t> pids(pid_list, pid_list + proclist.number);
    ProcessWorker::Iterator it(ProcessWorker::begin());

    while (it != ProcessWorker::end()) {
        ProcessWorker * const info = it->second;
        ProcessWorker::Iterator next(it);
        ++next;

        if (pids.find(info->pid) == pids.end() || info->status == GLIBTOP_PROCESS_ZOMBIE) {
            addition.remove(info);
            ProcessWorker::all.erase(it);
            delete info;
        }
        it = next;
    }

    QList<ProcessListItem*> items;
    for (ProcessWorker::Iterator it(ProcessWorker::begin()); it != ProcessWorker::end(); ++it) {
        QString username = QString::fromStdString(it->second->user);
        long nice = it->second->nice;
        QString name = QString::fromStdString(it->second->name);
        QString session;
        if (it->second->session) {
            session = QString(it->second->session);
        }
        QString status = formatProcessState(it->second->status);

        double cpu = it->second->pcpu;
        long memory = it->second->mem;
        pid_t pid = it->second->pid;
        QString flownetpersec = it->second->flownet_persec;
        QString diskiopersec = it->second->diskio_persec;
        long long int numFlowNetPersec = it->second->mNumFlownet;
        long long int numDiskioPersec = it->second->mNumDiskIo;

        /*---------------------kobe test string---------------------
        //QString to std:string
        QString test_QString = "lixiang";
        std::string test_string = test_QString.toStdString();
        //std::string to QString
        QString result = QString::fromStdString(test_string)
        QString::fromStdString(test_QString.toStdString());
        ----------------------------------------------------------*/

        std::string desktopFile;
        desktopFile = getDesktopFileAccordProcNameApp(name, "");
//        QString q_str = QString::fromStdString(desktopFile);   // this is the way that convert from std::string to QString
        if(desktopFile.empty())  //this is the way to detect that if the std::string is null or not.
        {
            desktopFile = getDesktopFileAccordProcName(name, "");
        }
        QPixmap icon_pixmap;
        int iconSize = 24 * qApp->devicePixelRatio();

        QIcon defaultExecutableIcon = QIcon::fromTheme("application-x-executable");//gnome-mine-application-x-executable
        if (defaultExecutableIcon.isNull()) {
            defaultExecutableIcon = QIcon("/usr/share/icons/kylin-icon-theme/48x48/mimetypes/application-x-executable.png");
            if (defaultExecutableIcon.isNull())
                defaultExecutableIcon = QIcon(":/res/autostart-default.png");
        }
        QPixmap defaultPixmap = defaultExecutableIcon.pixmap(iconSize, iconSize);
//        QPixmap defaultPixmap = QIcon::fromTheme("application-x-executable").pixmap(iconSize, iconSize);
        if (desktopFile.size() == 0) {
            icon_pixmap = defaultPixmap;
            icon_pixmap.setDevicePixelRatio(qApp->devicePixelRatio());
        } else {
            icon_pixmap = getAppIconFromDesktopFile(desktopFile, 24);
            if (icon_pixmap.isNull()) {
                icon_pixmap = defaultPixmap;
                icon_pixmap.setDevicePixelRatio(qApp->devicePixelRatio());
            }
            //QPixmap pixmap = QPixmap::fromImage(img).scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        QString title = getDisplayNameAccordProcName(name, desktopFile);
        QString displayName;
        if (whose_processes == "all") {
//            displayName = QString("[%1] %2").arg(username).arg(title);
            displayName = QString("%2").arg(title);
        } else {
            displayName = title;
        }

        ProcData info;
//        info.pidt = it->second->pid;
        info.user = username;       // 用户名
        info.iconPixmap = icon_pixmap;  //   进程加载的图片
        info.displayName = displayName;
        info.cpu = cpu;
        info.m_memory = memory;         //内存
        info.pid = pid;
        info.m_status = status;         //状态
        info.m_nice = nice;             //优先级
        info.m_session = session;
        info.m_flownet = flownetpersec;  //单个进程流量
        info.cpu_duration_time = formatDurationForDisplay(100 * it->second->cpu_time / this->frequency);    //CPU占用你时间
//        info.processName = QString::fromStdString(it->second->name);     //进程名称
        QLocale locale;
        if( locale.language() == QLocale::English )  //获取系统语言环境
        {
            info.processName = QString::fromStdString(it->second->name);     //进程名称
        }
        else if( locale.language() == QLocale::Chinese )
        {
            info.processName = displayName;
        }
        info.m_diskio = diskiopersec;    //单个进程磁盘io速率
        info.m_numFlowNet = numFlowNetPersec;   //网络流量整数值
        info.m_numDiskIo = numDiskioPersec;      //磁盘读写整数值
//        info.commandLine = QString::fromStdString(it->second->arguments); //命令行

        item = new ProcessListItem(info);
        items << item;
    }

    this->updateStatus(items);

    g_free (pid_list);
}

void ProcessDialog::onSearchFocusIn()
{
    qDebug()<<"focusin---";
    timer->stop();
    disconnect(refreshThread, SIGNAL(procDetected(const QString &, quint64 , quint64 , int , unsigned int , const QString&)),
            this, SLOT(refreshLine(const QString &, quint64 , quint64 , int, unsigned int , const QString&)));
}

void ProcessDialog::onSearchFocusOut()
{
    qDebug()<<"focusout---";
    timer->start(1500);
    connect(refreshThread, SIGNAL(procDetected(const QString &, quint64 , quint64 , int , unsigned int , const QString&)),
            this, SLOT(refreshLine(const QString &, quint64 , quint64 , int, unsigned int , const QString&)));
}

void ProcessDialog::refreshLine(const QString &procname, quint64 rcv, quint64 sent, int pid, unsigned int uid, const QString &devname)
{
//    pidList<<pid;
//    if(!speedLineBand)
//    {
        speedLineBandFlowNet = new lineBandwith(pid,this);
//    }
    haveNetPid = pid;
    qint64 tmptotalFlowNetPerSec = rcv + sent;
    if(!flowNetPrevMap.contains(pid))
    {
        flowNetPrevMap[pid] = 0;//save prev data
    }

    numAddFlowNetPerSec = tmptotalFlowNetPerSec - flowNetPrevMap[pid];
//    qDebug() << "numAddFlowNetPerSec" << numAddFlowNetPerSec;
    addFlowNetPerSec = speedLineBandFlowNet->new_count(tmptotalFlowNetPerSec - flowNetPrevMap[pid],pid);
//    qDebug() << "ProcessDialog::refreshLine:deltaFlowNetPerSec" << tmptotalFlowNetPerSec - flowNetPrevMap[pid];
    flowNetPrevMap[pid] = tmptotalFlowNetPerSec;
//     qDebug() << "ProcessDialog::refreshLine:after flowNetPrevMap" << flowNetPrevMap[pid];
    if(!pidMap.contains(pid))
    {
        pidMap[pid] = addFlowNetPerSec;
    }
    QMap<int,QString>::const_iterator it = pidMap.find(pid);
//    qDebug() << "ProcessDialog::pidMap" << it.value();
    if(it.value() != addFlowNetPerSec)
    {
        pidMap[pid] = addFlowNetPerSec;
    }
    delete  speedLineBandFlowNet;
}

ProcessListWidget* ProcessDialog::getProcessView()
{
    return m_processListWidget;
}

void ProcessDialog::endDialogButtonClicked(int index, QString)
{
    emit closeDialog();
    if (index == 1) {//cancel:0   ok:1
        endProcesses();
    }
}

void ProcessDialog::killDialogButtonClicked(int index, QString)
{
    emit closeDialog();
    if (index == 1) {//cancel:0   ok:1
        killProcesses();
    }
}

void ProcessDialog::focusProcessView()
{
    QTimer::singleShot(100, m_processListWidget, SLOT(setFocus()));
}

void ProcessDialog::onSearch(QString text)
{
    m_processListWidget->doSearch(text);
}

//杀死   SIGSTOP,SIGCONT,SIGTERM,SIGKILL
void ProcessDialog::killProcesses()
{
//    int error;

//    for (pid_t pid : *actionPids) {
//        // Resume process first, otherwise kill process too slow.
//        kill(pid, SIGCONT);

//        error = kill(pid, SIGKILL);
//        if(error != -1) {
//            qDebug() << "success.....";
//        } else {
//            //need to be root
//            if(errno == EPERM) {//(kill -s %d %d", sig, pid)
//                qWarning() << QString("Kill process %1 failed, permission denied.").arg(pid);
//                if (QFileInfo("/usr/bin/pkexec").exists()) {//sudo apt install policykit-1
//                    QProcess process;
//                    process.execute(QString("pkexec --disable-internal-agent %1 %2 %3").arg("kill").arg(SIGKILL).arg(pid));
//                } else if (QFileInfo("/usr/bin/gksudo").exists()) {//sudo apt install gksu
//                    QProcess process;
//                    process.execute(QString("gksudo \"%1 %2 %3\"").arg("kill").arg(SIGKILL).arg(pid));
//                } else if (QFileInfo("/usr/bin/gksu").exists()) {//sudo apt install gksu
//                    QProcess process;
//                    process.execute(QString("gksu \"%1 %2 %3\"").arg("kill").arg(SIGKILL).arg(pid));
//                } else {
//                    qWarning() << "Failed to choose a tool to kill " << pid;
//                }
//            }
//        }
//    }

//    actionPids->clear();
    int error;
    for (pid_t pid : *actionPids) {
        error = kill(pid, SIGTERM);
        if(error != -1)  {
            qDebug() << "success.....";
        }
        else {
            //need to be root
            if(errno == EPERM) {
                qDebug() << QString("End process %1 failed, permission denied.").arg(pid);

                if (QFileInfo("/usr/bin/pkexec").exists()) {//sudo apt install policykit-1
                    QProcess process;
                    process.execute(QString("pkexec --disable-internal-agent %1 %2 %3").arg("kill").arg(SIGTERM).arg(pid));
                } else if (QFileInfo("/usr/bin/gksudo").exists()) {//sudo apt install gksu
                    QProcess process;
                    process.execute(QString("gksudo \"%1 %2 %3\"").arg("kill").arg(SIGTERM).arg(pid));
                } else if (QFileInfo("/usr/bin/gksu").exists()) {//sudo apt install gksu
                    QProcess process;
                    process.execute(QString("gksu \"%1 %2 %3\"").arg("kill").arg(SIGTERM).arg(pid));
                } else {
                    qWarning() << "Failed to choose a tool to end process " << pid;
                }
            }
        }
    }

    actionPids->clear();
}

//结束
void ProcessDialog::endProcesses()
{
    int error;
    for (pid_t pid : *actionPids) {
        error = kill(pid, SIGTERM);
        if(error != -1)  {
            qDebug() << "success.....";
        }
        else {
            //need to be root
            if(errno == EPERM) {
                qDebug() << QString("End process %1 failed, permission denied.").arg(pid);

                if (QFileInfo("/usr/bin/pkexec").exists()) {//sudo apt install policykit-1
                    QProcess process;
                    process.execute(QString("pkexec --disable-internal-agent %1 %2 %3").arg("kill").arg(SIGTERM).arg(pid));
                } else if (QFileInfo("/usr/bin/gksudo").exists()) {//sudo apt install gksu
                    QProcess process;
                    process.execute(QString("gksudo \"%1 %2 %3\"").arg("kill").arg(SIGTERM).arg(pid));
                } else if (QFileInfo("/usr/bin/gksu").exists()) {//sudo apt install gksu
                    QProcess process;
                    process.execute(QString("gksu \"%1 %2 %3\"").arg("kill").arg(SIGTERM).arg(pid));
                } else {
                    qWarning() << "Failed to choose a tool to end process " << pid;
                }
            }
        }
    }

    actionPids->clear();
}

void ProcessDialog::popupMenu(QPoint pos, QList<ProcessListItem*> items)
{
    actionPids->clear();

    int count = 0;
    pid_t cur_pid = -1;
    for (ProcessListItem *item : items) {
        count ++;
        ProcessListItem *procItem = static_cast<ProcessListItem*>(item);
        cur_pid = procItem->getPid();
        actionPids->append(cur_pid);
    }
    if (count == 1) {
        ProcessWorker *info = ProcessWorker::find(cur_pid);
        if (!info) {
            priorityGroup->setActionsEnabled(false);
        }
        else {
            priorityGroup->setActionsEnabled(true);
            gint nice = info->nice;
            int priority;
            if (nice < -7)
                priority = -20;
            else if (nice < -2)
                priority = -5;
            else if (nice < 3)
                priority = 0;
            else if (nice < 7)
                priority = 5;
            else
                priority = 19;
            priorityGroup->setChecked(priority);
        }
    }
    else {
        priorityGroup->setActionsEnabled(false);
    }
    m_menu->exec(pos);
}

void ProcessDialog::continueProcesses()
{
    for (pid_t pid : *actionPids) {
        if (kill(pid, SIGCONT) != 0) {
            qDebug() << QString("Resume process %1 failed, permission denied.").arg(pid);
            QProcess process;
            process.execute(QString("pkexec %1 %2 %3 ").arg("kill").arg("-CONT").arg(pid));
        }
    }

    actionPids->clear();
}

void ProcessDialog::showPropertiesDialog()
{
    for (pid_t pid : *actionPids) {
        foreach (QWidget *widget, QApplication::topLevelWidgets()) {
            // Show attribute dialog if it has create, avoid create attribute dialog duplicate.
            if (qobject_cast<const PropertiesDialog*>(widget) != 0) {
                PropertiesDialog *dialog = qobject_cast<PropertiesDialog*>(widget);
                if (dialog->getPid() == pid) {
                    dialog->show();
                    actionPids->clear();
                    return;
                }
            }
        }

        PropertiesDialog *dialog = new PropertiesDialog(this, pid);
        dialog->show();
    }

    actionPids->clear();
}

void ProcessDialog::showKillProcessDialog()
{
//    killProcessDialog->exec();
//    killProcessDialog->show();
    killProcessDialog = new MyDialog(QString(tr("Kill process")), QString(tr("Killing a process may destroy data, break the session or introduce a security risk. Only unresponsive processes should be killed.\nAre you sure to continue?")));
//    killProcessDialog->setWindowFlags(killProcessDialog->windowFlags() | Qt::WindowStaysOnTopHint);
    killProcessDialog->addButton(QString(tr("Cancel")), false);
    killProcessDialog->addButton(QString(tr("Kill process")), true);
    connect(killProcessDialog, &MyDialog::buttonClicked, this, &ProcessDialog::killDialogButtonClicked);
    killProcessDialog->show();
    connect(this,&ProcessDialog::closeDialog,killProcessDialog,&MyDialog::onButtonClicked);
}   

void ProcessDialog::showEndProcessDialog()
{
//    endProcessDialog->exec();
//    endProcessDialog->show();
    endProcessDialog = new MyDialog(QString(tr("End process")), QString(tr("Ending a process may destroy data, break the session or introduce a security risk. Only unresponsive processes should be ended.\nAre you sure to continue?")));
//    endProcessDialog->setWindowFlags(endProcessDialog->windowFlags() | Qt::WindowStaysOnTopHint);
    endProcessDialog->addButton(QString(tr("Cancel")), false);
    endProcessDialog->addButton(QString(tr("End process")), true);
    connect(endProcessDialog, &MyDialog::buttonClicked, this, &ProcessDialog::endDialogButtonClicked);
    endProcessDialog->show();
    connect(this,&ProcessDialog::closeDialog,endProcessDialog,&MyDialog::onButtonClicked);
}

//停止
void ProcessDialog::stopProcesses()
{
    pid_t currentPid = getpid();

    for (pid_t pid : *actionPids) {
        if (pid != currentPid) {
            if (kill(pid, SIGSTOP) != 0) {
                qDebug() << QString("Stop process %1 failed, permission denied.").arg(pid);
                QProcess process;
                process.execute(QString("pkexec %1 %2 %3 ").arg("kill").arg("-STOP").arg(pid));
            }
        }
    }
    actionPids->clear();
}

void ProcessDialog::updateStatus(QList<ProcessListItem*> items)
{
    m_processListWidget->refreshItems(items);
}

/*void ProcessDialog::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    //绘制背景色
//    QPainterPath path;
//    path.addRect(QRectF(rect()));
//    painter.setOpacity(1);
//    painter.fillPath(path, QColor("#FFFFFF"));

    //绘制圆角矩形
//    painter.setPen(QPen(QColor("#0d87ca"), 0));//边框颜色   QColor(255, 255, 255, 153)
//    painter.setBrush(QColor("#e9eef0"));//背景色   #0d87ca
    painter.setPen(QPen(QColor("#0000FF"), 0));//边框颜色   QColor(255, 255, 255, 153)
    painter.setBrush(QColor("#B22222"));//背景色   #0d87ca
    painter.setOpacity(1);
//    QRectF r(0 / 2.0, 0 / 2.0, width() - 0, height() - 0);//左边 上边 右边 下边
    QRectF r(2, 2, width() - 0, height() - 0);//左边 上边 右边 下边
    painter.drawRoundedRect(r, 10, 10);

    QWidget::paintEvent(event);
}*/
