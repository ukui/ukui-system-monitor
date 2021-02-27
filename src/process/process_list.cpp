/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
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

#include "process_list.h"
#include "../util.h"
#include "../linebandwith.h"

#include <QDebug>
#include <QApplication>

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
#include <sys/resource.h> 
#include <systemd/sd-login.h>

namespace sysmonitor {
namespace process {

static bool init = false;
static bool is_running = false;

QString getPriorityName(int prio)
{
    const static QMap<ProcessPriority, QString> priorityMap = {
        {VeryHighPriority, QApplication::translate("ProcessDialog", "Very High")},
        {HighPriority, QApplication::translate("ProcessDialog", "High")},
        {NormalPriority, QApplication::translate("ProcessDialog", "Normal")},
        {LowPriority, QApplication::translate("ProcessDialog", "Low")},
        {VeryLowPriority, QApplication::translate("ProcessDialog", "Very low")},
        {CustomPriority, QApplication::translate("ProcessDialog", "Custom")},
        {InvalidPriority, QApplication::translate("ProcessDialog", "Invalid")}
    };

    ProcessPriority p = InvalidPriority;
    if (prio == VeryHighPriority || prio == HighPriority || prio == NormalPriority || prio == LowPriority || prio == VeryLowPriority) {
        p = ProcessPriority(prio);
    } else if (prio >= VeryHighPriorityMax && prio <= VeryLowPriorityMin) {
        p = CustomPriority;
    }

    return priorityMap[p];
}

ProcessPriority getProcessPriorityStub(int prio)
{
    if (prio == 0) {
        return NormalPriority;
    } else if (prio == VeryHighPriority || prio == HighPriority || prio == LowPriority || prio == VeryLowPriority) {
        return ProcessPriority(prio);
    } else if (prio <= VeryLowPriorityMin && prio >= VeryHighPriorityMax) {
        return CustomPriority;
    } else {
        return InvalidPriority;
    }
}

Process::Process()
    : d(new ProcessData())
{

}
Process::Process(pid_t pid)
    : d(new ProcessData())
{
    d->pid = pid;
}
Process::Process(const Process &other)
    : d(other.d)
{
}
Process &Process::operator=(const Process &rhs)
{
    if (this == &rhs)
        return *this;

    d = rhs.d;
    return *this;
}

Process::~Process()
{
}

bool Process::isValid() const
{
    return d && d->isValid();
}

void Process::setValid(bool isValid)
{
    if (d) {
        d->valid = isValid;
    }
}

pid_t Process::pid() const
{
    return d->pid;
}

double Process::getCpuPercent() const
{
    return d->cpu;
}

void Process::setCpuPercent(double cpuPercent)
{
    if (d) {
        d->cpu = cpuPercent;
    }
}

qulonglong Process::getMemeryUsed() const
{
    return d->m_memory;
}

void Process::setMemeryUsed(qulonglong lMemUsed)
{
    if (d) {
        d->m_memory = lMemUsed;
    }
}

int Process::getNice() const
{
    return d->m_nice;
}

void Process::setNice(int lNice)
{
    if (d) {
        d->m_nice = lNice;
    }
}

QString Process::getFlowNetDesc() const
{
    return d->m_flownet;
}

void Process::setFlowNetDesc(QString strFlowNet)
{
    if (d) {
        d->m_flownet = strFlowNet;
    }
}

QString Process::getDiskIODesc() const
{
    return d->m_diskio;
}

void Process::setDiskIODesc(QString strDiskIO)
{
    if (d) {
        d->m_diskio = strDiskIO;
    }
}

qulonglong Process::getFlowNet() const
{
    return d->m_numFlowNet;
}

void Process::setFlowNet(qulonglong llFlowNet)
{
    if (d) {
        d->m_numFlowNet = llFlowNet;
    }
}

qulonglong Process::getDiskIO() const
{
    return d->m_numDiskIo;
}

void Process::setDiskIO(qulonglong llDiskIO)
{
    if (d) {
        d->m_numDiskIo = llDiskIO;
    }
}

QPixmap Process::getIconPixmap() const
{
    return d->iconPixmap;
}

void Process::setIconPixmap(QPixmap pixmapIcon)
{
    if (d) {
        d->iconPixmap = pixmapIcon;
    }
}

QString Process::getProcName() const
{
    return d->processName;
}

void Process::setProcName(QString strProcName)
{
    if (d) {
        d->processName = strProcName;
    }
}

QString Process::getDisplayName() const
{
    return d->displayName;
}

void Process::setDisplayName(QString strDisplayName)
{
    if (d) {
        d->displayName = strDisplayName;
    }
}

QString Process::getProcPath() const
{
    return d->path;
}

void Process::setProcPath(QString strProcPath)
{
    if (d) {
        d->path = strProcPath;
    }
}

QString Process::getProcUser() const
{
    return d->user;
}

void Process::setProcUser(QString strProcUser)
{
    if (d) {
        d->user = strProcUser;
    }
}

QString Process::getProcStatus() const
{
    return d->m_status;
}

void Process::setProcStatus(QString strProcStatus)
{
    if (d) {
        d->m_status = strProcStatus;
    }
}

QString Process::getProcSession() const
{
    return d->m_session;
}

void Process::setProcSession(QString strProcSession)
{
    if (d) {
        d->m_session = strProcSession;
    }
}

QString Process::getProcCpuDurationTime() const
{
    return d->cpu_duration_time;
}

void Process::setProcCpuDurationTime(QString strProcCpuDurationTime)
{
    if (d) {
        d->cpu_duration_time = strProcCpuDurationTime;
    }
}

unsigned int Process::getProcType() const
{
    return d->m_type;
}

void Process::addProcType(unsigned int uType)
{
    if (d) {
        d->m_type |= uType;
    }
}

void Process::clearProcType()
{
    if (d) {
        d->m_type = NoFilter;
    }
}

QString Process::getProcUnit() const
{
    return d->m_unit;
}

void Process::setProcUnit(QString strUnit)
{
    if (d) {
        d->m_unit = strUnit;
    }
}

QString Process::getProcSeat() const
{
    return d->m_seat;
}

void Process::setProcSeat(QString strSeat)
{
    if (d) {
        d->m_seat = strSeat;
    }
}

unsigned long Process::getProcStartTime() const
{
    return d->m_start_time;
}

void Process::setProcStartTime(unsigned long luStartTime)
{
    if (d) {
        d->m_start_time = luStartTime;
    }
}

qulonglong Process::getProcCpuTime() const
{
    return d->m_cpu_time;
}

void Process::setProcCpuTime(qulonglong lluCpuTime)
{
    if (d) {
        d->m_cpu_time = lluCpuTime;
    }
}

unsigned Process::getStatus() const
{
    return d->m_ustatus;
}

void Process::setStatus(unsigned uStatus)
{
    if (d) {
        d->m_ustatus = uStatus;
    }
}

double Process::getProcPCPU() const
{
    return d->m_pcpu;
}

void Process::setProcPCPU(double fPcpu)
{
    if (d) {
        d->m_pcpu = fPcpu;
    }
}

unsigned Process::getFrequency() const
{
    return d->m_frequency;
}

void Process::setFrequency(unsigned uFreq)
{
    if (d) {
        d->m_frequency = uFreq;
    }
}

QString Process::getProcTooltip() const
{
    return d->m_tooltip;
}

void Process::setProcTooltip(QString strTooltip)
{
    if (d) {
        d->m_tooltip = strTooltip;
    }
}
QString Process::getProcArgments() const
{
    return d->m_arguments;
}

void Process::setProcArgments(QString strArgments)
{
    if (d) {
        d->m_arguments = strArgments;
    }
}

void Process::updateProcUser(unsigned uUid)
{
    if (!d || d->m_uid == uUid)
        return;
    d->m_uid = uUid;
    // getuser
    struct passwd* pwd;
    pwd = getpwuid(uUid);

    if (pwd && pwd->pw_name)
        setProcUser(pwd->pw_name);
    else {
        char username[16];
        g_sprintf(username, "%u", uUid);
        setProcUser(username);
    }
}

void Process::get_process_name(const gchar *cmd, const GStrv args)
{
    if (args) {
        // look for /usr/bin/very_long_name
        // and also /usr/bin/interpreter /usr/.../very_long_name
        // which may have use prctl to alter 'cmd' name
        for (int i = 0; i != 2 && args[i]; ++i) {
            char* basename;
            basename = g_path_get_basename(args[i]);

            if (g_str_has_prefix(basename, cmd)) {
                setProcName(basename);
                g_free(basename);
                return;
            }
            g_free(basename);
        }
    }
    setProcName(cmd);
//    qDebug()<<"process name: "<<info->name;
}

void Process::get_process_systemd_info()
{
    if (!init) {
        if (access("/run/systemd/seats/", F_OK) >= 0) {
            is_running = true;
        }
        init = true;
    }

    if (is_running) {
        gchar* unit = NULL;
        gchar* session = NULL;
        gchar* seat = NULL;
        sd_pid_get_unit(pid(), &unit);
        if (unit) {
            setProcUnit(unit);
            g_free(unit);
            unit = NULL;
        }

        sd_pid_get_session(pid(), &session);

        if (session) {
            setProcSession(session);
            sd_session_get_seat(session, &seat);
            if (seat) {
                setProcSeat(seat);
                g_free(seat);
                seat = NULL;
            }
            g_free(session);
            session = NULL;
        }
    }
}

void Process::UpdateProcInfo()
{
    //    glibtop_proc_io procio;
    glibtop_proc_state procstate;
    glibtop_proc_time proctime;
    glibtop_proc_args procargs;
    gchar** args;

    glibtop_get_proc_state(&procstate, pid());
//    glibtop_get_proc_io(&procio,pid());
    glibtop_get_proc_time(&proctime, pid());
    args = glibtop_get_proc_argv(&procargs, pid(), 0);

    get_process_name(procstate.cmd, static_cast<const GStrv>(args));

    std::string tooltip = make_string(g_strjoinv(" ", args));
    if (tooltip.empty())
        tooltip = procstate.cmd;
    gchar* pToolTip = g_markup_escape_text(tooltip.c_str(), -1);
    if (pToolTip) {
        setProcTooltip(pToolTip);
        g_free(pToolTip);
        pToolTip = NULL;
    }
    gchar* pArgments = g_strescape(tooltip.c_str(), "\\\"");
    if (pArgments) {
        setProcArgments(pArgments);
        g_free(pArgments);
        pArgments = NULL;
    }
    g_strfreev(args);

    guint64 cpu_time = proctime.utime + proctime.stime + proctime.cutime + proctime.cstime;
    setProcCpuTime(cpu_time);
    setProcStartTime(proctime.start_time);

//    get_process_selinux_context (info);
//    info->cgroup_name = NULL;
//    get_process_cgroup_info(info);
    get_process_systemd_info();
}

ProcessList::ProcessList(QObject* parent)
    : QObject(parent), m_set {}
{
    glibtop_init();
    this->num_cpus = glibtop_get_sysinfo()->ncpu;
    scanThread = new ScanThread(this);
    scanThread->start(QThread::TimeCriticalPriority);

    refreshThread = new RefreshThread(this);
    refreshThread->start(QThread::HighPriority);

    connect(refreshThread, SIGNAL(procDetected(const QString &, quint64 , quint64 , int , unsigned int , const QString&)),
            this, SLOT(refreshLine(const QString &, quint64 , quint64 , int, unsigned int , const QString&)));
}

ProcessList::~ProcessList()
{
    if(scanThread)
    {
        scanThread->stop();
        scanThread->wait();
    }

    if(refreshThread)
    {
        refreshThread->stop();
        refreshThread->wait();
    }
    glibtop_close();
}

bool ProcessList::containsById(pid_t pid)
{
    return m_set.contains(pid);
}

void ProcessList::setScanFilter(QString strFilter)
{
    if (!strFilter.isEmpty()) {
        m_strFilter = strFilter;
    }
}

void ProcessList::onClearAllProcess()
{
    m_set.clear();
}

void ProcessList::refresh()
{
    scanProcess();
}

void ProcessList::scanProcess()
{
    pid_t* pid_list;
    glibtop_proclist proclist;
    int which = 0;
    int arg = 0;

    if (m_strFilter == "all")
    {
        which = GLIBTOP_KERN_PROC_ALL;
        arg = 0;
    }
    else if (m_strFilter == "active")
    {
        which = GLIBTOP_KERN_PROC_ALL | GLIBTOP_EXCLUDE_IDLE;
        arg = 0;
    }
    else if (m_strFilter == "user")
    {
        which = GLIBTOP_KERN_PROC_UID;
        arg = getuid();
    }

    pid_list = glibtop_get_proclist(&proclist, which, arg);

    // FIXME: not sure if glibtop always returns a sorted list of pid
    // but it is important otherwise refresh_list won't find the parent
    std::sort(pid_list, pid_list + proclist.number);

    // remove not exist
    QMutableMapIterator<pid_t, Process> itSet(m_set);
    while (itSet.hasNext()) {
        itSet.next();
        bool bFound = false;
        for(unsigned i = 0; i < proclist.number; ++i) {
            if (itSet.key() == pid_list[i]) {
                bFound = true;
                break;
            }
        }
        if (!bFound) {
            itSet.remove();
        }
    }

    guint i;
    for(i = 0; i < proclist.number; ++i)
    {
        pid_t pidCur = pid_list[i];
        Process proc(pidCur);
        proc.setValid(true);
        proc.UpdateProcInfo();

        if (pidMap.contains(pidCur)) {
            proc.setFlowNetDesc(pidMap[pidCur]);
        } else {
            proc.setFlowNetDesc("0 KB/S");
        }
        if (numAddFlowNetPerSec.contains(pidCur)) {
            proc.setFlowNet(numAddFlowNetPerSec[pidCur]);
        } else {
            proc.setFlowNet(0);
        }

        //当进程对象存在时，更新该进程对象的相关数据信息
        glibtop_proc_state procstate;
        glibtop_proc_uid procuid;
        glibtop_proc_time proctime;
        glibtop_proc_io procio;

        glibtop_get_proc_io(&procio,pidCur);
        glibtop_get_proc_state (&procstate, pidCur);
        proc.setStatus(procstate.state);

        glibtop_get_proc_uid(&procuid, pidCur);
        glibtop_get_proc_time(&proctime, pidCur);

        glibtop_proc_mem procmem;
        glibtop_get_proc_mem(&procmem, pidCur);

        // check cpu time
        if (containsById(pidCur)) {
            if (proctime.rtime >= getProcessById(pidCur).getProcCpuTime()) {
                proc.setProcCpuTime(getProcessById(pidCur).getProcCpuTime());
            }   
        }

        proc.setMemeryUsed(procmem.resident - procmem.share);

        proc.updateProcUser(procstate.uid);

        /* FIXME: total cpu time elapsed should be calculated on an individual basis here
        ** should probably have a total_time_last gint in the ProcInfo structure */
        glibtop_cpu cpu;
        glibtop_get_cpu(&cpu);

        this->cpu_total_time = cpu.total;
        this->process_total_time = proctime.rtime;

        this->cpu_total_time = MAX(cpu.total - this->cpu_total_time_last, 1);
        this->cpu_total_time_last = cpu.total;
        guint64 difference = proctime.utime + proctime.stime + proctime.cutime + proctime.cstime - proc.getProcCpuTime();
        if (difference > 0)
            proc.setStatus(GLIBTOP_PROCESS_RUNNING);
        double sPcpu = difference / this->cpu_total_time;
        sPcpu = MIN(sPcpu/2, 100);
        sPcpu *= this->num_cpus /10.0;
        proc.setCpuPercent(sPcpu);

//        CPU 百分比使用 Solaris 模式，工作在“Solaris 模式”，其中任务的 CPU 使用量将被除以总的 CPU 数目。否则它将工作在“Irix 模式”。
        proc.setFrequency(cpu.frequency);
        proc.setProcCpuTime(proctime.utime + proctime.stime + proctime.cutime + proctime.cstime);
        proc.setNice(procuid.nice);
        if(!calDiskIoMap.contains(pidCur))
        {
            calDiskIoMap[pidCur] = 0;
        }

        lineBandwith speedLineBandDiskIo(pidCur);

        disk_io_bytes_total = procio.disk_wbytes+procio.disk_rbytes;

        QString addDiskIoPerSec = speedLineBandDiskIo.new_count(disk_io_bytes_total-calDiskIoMap[pidCur],pidCur);

        proc.setDiskIODesc(addDiskIoPerSec);
        proc.setDiskIO(disk_io_bytes_total-calDiskIoMap[pidCur]);
        
        calDiskIoMap[pidCur] = disk_io_bytes_total;

        proc.setProcStatus(formatProcessState(proc.getStatus()));

        std::string desktopFile;
        desktopFile = getDesktopFileAccordProcNameApp(proc.getProcName(), "");
//        QString q_str = QString::fromStdString(desktopFile);   // this is the way that convert from std::string to QString
        if(desktopFile.empty())  //this is the way to detect that if the std::string is null or not.
        {
            desktopFile = getDesktopFileAccordProcName(proc.getProcName(), "");
        }

        if (!m_set.contains(proc.pid())) {
            QPixmap icon_pixmap;
            int iconSize = 20 * qApp->devicePixelRatio();

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
                //icon_pixmap = getAppIconFromDesktopFile(desktopFile, 20);
                if (icon_pixmap.isNull()) {
                    icon_pixmap = defaultPixmap;
                    icon_pixmap.setDevicePixelRatio(qApp->devicePixelRatio());
                }
                //QPixmap pixmap = QPixmap::fromImage(img).scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }
            proc.setIconPixmap(icon_pixmap);            
        } else {
            proc.setIconPixmap(getProcessById(proc.pid()).getIconPixmap());  
        }

        QString title = getDisplayNameAccordProcName(proc.getProcName(), desktopFile);
        QLocale locale;
        if( locale.language() == QLocale::English )  //获取系统语言环境
        {
            proc.setDisplayName(proc.getProcName()); //进程名称
        }
        else if( locale.language() == QLocale::Chinese )
        {
            proc.setDisplayName(title);
        }
        proc.setProcCpuDurationTime(formatDurationForDisplay(100 * proc.getProcCpuTime() / proc.getFrequency()));
        proc.setProcStatus(formatProcessState(proc.getStatus()));
        if (proc.getStatus() == GLIBTOP_PROCESS_ZOMBIE) {
            if (m_set.contains(proc.pid())) {
                removeProcess(proc.pid());
            }
        } else {
            m_set[proc.pid()] = proc;
        }
    }
    g_free (pid_list);
}

const Process ProcessList::getProcessById(pid_t pid) const
{
    return m_set[pid];
}

QList<pid_t> ProcessList::getPIDList() const
{
    return m_set.keys();
}

void ProcessList::removeProcess(pid_t pid)
{
    m_set.remove(pid);
}

void ProcessList::updateProcessState(pid_t pid, QString& state)
{
    if (m_set.contains(pid))
        m_set[pid].setProcStatus(state);
}

void ProcessList::updateProcessPriority(pid_t pid, qulonglong priority)
{
    if (m_set.contains(pid))
        m_set[pid].setNice(priority);
}

void ProcessList::endProcess(pid_t pid)
{
    //sendSignalToProcess(pid, SIGTERM);
}

void ProcessList::pauseProcess(pid_t pid)
{
    //sendSignalToProcess(pid, SIGSTOP);
}

void ProcessList::resumeProcess(pid_t pid)
{
    //sendSignalToProcess(pid, SIGCONT);
}

void ProcessList::killProcess(pid_t pid)
{
    //sendSignalToProcess(pid, SIGKILL);
}

int ProcessList::setProcessPriority(pid_t pid, int priority)
{
    return 0;
}

void ProcessList::refreshLine(const QString &procname, quint64 rcv, quint64 sent, int pid, unsigned int uid, const QString &devname)
{
    lineBandwith speedLineBandFlowNet(pid);

    qint64 tmptotalFlowNetPerSec = rcv + sent;
    if(!flowNetPrevMap.contains(pid))
    {
        flowNetPrevMap[pid] = 0;//save prev data
    }

    numAddFlowNetPerSec[pid] = tmptotalFlowNetPerSec - flowNetPrevMap[pid];

    QString addFlowNetPerSec = speedLineBandFlowNet.new_count(tmptotalFlowNetPerSec - flowNetPrevMap[pid],pid);

    flowNetPrevMap[pid] = tmptotalFlowNetPerSec;

    pidMap[pid] = addFlowNetPerSec;
}

} // namespace process
} // namespace sysmonitor