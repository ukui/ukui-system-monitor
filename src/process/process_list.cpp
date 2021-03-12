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

qint64 Process::getFlowNet() const
{
    return d->m_numFlowNet;
}

void Process::setFlowNet(qint64 llFlowNet)
{
    if (d) {
        llFlowNet = llFlowNet < 0 ? 0 : llFlowNet;
        d->m_numFlowNet = llFlowNet;
    }
}

qint64 Process::getDiskIO() const
{
    return d->m_numDiskIo;
}

void Process::setDiskIO(qint64 llDiskIO)
{
    if (d) {
        llDiskIO = llDiskIO < 0 ? 0 : llDiskIO;
        d->m_numDiskIo = llDiskIO;
    }
}

QString Process::getIconPath() const
{
    return d->strIconPath;
}

void Process::setIconPath(QString iconPath)
{
    if (d) {
        d->strIconPath = iconPath;
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

QDateTime Process::getPreDiskIoTime() const
{
    return d->m_preDiskIoTime;
}

void Process::setPreDiskIoTime(QDateTime dt)
{
    if (d) {
        d->m_preDiskIoTime = dt;
    }
}

qint64 Process::getPreDiskIoCount() const
{
    return d->m_preDiskIoCount;
}

void Process::setPreDiskIoCount(qint64 nCount)
{
    if (d) {
        nCount = nCount < 0 ? 0 : nCount;
        d->m_preDiskIoCount = nCount;
    }
}

QDateTime Process::getPreFlownetTime() const
{
    return d->m_preFlownetTime;
}

void Process::setPreFlownetTime(QDateTime dt)
{
    if (d) {
        d->m_preFlownetTime = dt;
    }
}

qint64 Process::getPreFlownetCount() const
{
    return d->m_preFlownetCount;
}

void Process::setPreFlownetCount(qint64 nCount)
{
    if (d) {
        nCount = nCount < 0 ? 0 : nCount;
        d->m_preFlownetCount = nCount;
    }
}

QString Process::calcDiskIoPerSec(qint64 nNewCount)
{
    qint64 bandwith = nNewCount - getPreDiskIoCount();
    QDateTime now = QDateTime::currentDateTime();
    quint64 ms_lapse = d->m_preDiskIoTime.msecsTo(now);
    QString speedPerSec = "";

    if (ms_lapse < 1000) ms_lapse = 1000; // prevent division by 0 ;-)
    qint64 speed = bandwith * 1000 / ms_lapse;

    if (speed == 0 || speed < 0) {
        speedPerSec = "0 KB/S"; }
    else if (speed < 1900) {
        speedPerSec.setNum(speed);
        speedPerSec.append(" B/s"); }
    else if (speed < 1900000) {
        speedPerSec.setNum(speed/1024);
        speedPerSec.append(" KB/s"); }
    else if (speed < 1900000000) {
        speedPerSec.setNum(speed/(1024*1024));
        speedPerSec.append(" MB/s"); }
    else {
        speedPerSec.setNum(speed/(1024*1024*1024));
        speedPerSec.append(" GB/s");
    }

    d->m_preDiskIoTime.swap(now);
    setPreDiskIoCount(nNewCount);
    return speedPerSec;
}

QString Process::calcFlownetPerSec(qint64 nNewCount)
{
    qint64 bandwith = nNewCount - getPreFlownetCount();
    QDateTime now = QDateTime::currentDateTime();
    quint64 ms_lapse = d->m_preFlownetTime.msecsTo(now);
    QString speedPerSec = "";

    if (ms_lapse < 1000) ms_lapse = 1000; // prevent division by 0 ;-)
    qint64 speed = bandwith * 1000 / ms_lapse;

    if (speed == 0 || speed < 0) {
        speedPerSec = "0 KB/S"; }
    else if (speed < 1900) {
        speedPerSec.setNum(speed);
        speedPerSec.append(" B/s"); }
    else if (speed < 1900000) {
        speedPerSec.setNum(speed/1024);
        speedPerSec.append(" kB/s"); }
    else if (speed < 1900000000) {
        speedPerSec.setNum(speed/(1024*1024));
        speedPerSec.append(" MB/s"); }
    else {
        speedPerSec.setNum(speed/(1024*1024*1024));
        speedPerSec.append(" GB/s");
    }

    d->m_preFlownetTime.swap(now);
    setPreFlownetCount(nNewCount);
    return speedPerSec;
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

    setProcCpuTime(proctime.rtime);
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

void ProcessList::connectNetStateRefresh()
{
    connect(refreshThread, SIGNAL(procDetected(const QString &, quint64 , quint64 , int , unsigned int , const QString&)),
            this, SLOT(refreshLine(const QString &, quint64 , quint64 , int, unsigned int , const QString&)));
}

void ProcessList::disconnectNetStateRefresh()
{
    disconnect(refreshThread, SIGNAL(procDetected(const QString &, quint64 , quint64 , int , unsigned int , const QString&)),
            this, SLOT(refreshLine(const QString &, quint64 , quint64 , int, unsigned int , const QString&)));
}

bool ProcessList::containsById(pid_t pid)
{
    bool bContainsObj = false;
    m_lockReadWrite.lockForRead();
    bContainsObj = m_set.contains(pid);
    m_lockReadWrite.unlock();
    return bContainsObj;
}

void ProcessList::setScanFilter(QString strFilter)
{
    m_lockReadWrite.lockForWrite();
    if (!strFilter.isEmpty()) {
        m_strFilter = strFilter;
    }
    m_lockReadWrite.unlock();
}

void ProcessList::onClearAllProcess()
{
    m_lockReadWrite.lockForWrite();
    m_set.clear();
    m_lockReadWrite.unlock();
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

    m_lockReadWrite.lockForRead();
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
    
    if (m_isScanStoped) {
        m_lockReadWrite.unlock();
        return ;
    }
    m_lockReadWrite.unlock();

    pid_list = glibtop_get_proclist(&proclist, which, arg);
    
    /* FIXME: total cpu time elapsed should be calculated on an individual basis here
        ** should probably have a total_time_last gint in the ProcInfo structure */
    glibtop_cpu cpu;
    glibtop_get_cpu(&cpu);
    this->cpu_total_time = MAX(cpu.total - this->cpu_total_time_last, 1);
    this->cpu_total_time_last = cpu.total;

    // FIXME: not sure if glibtop always returns a sorted list of pid
    // but it is important otherwise refresh_list won't find the parent
    std::sort(pid_list, pid_list + proclist.number);

    // remove not exist
    m_lockReadWrite.lockForWrite();
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
    m_lockReadWrite.unlock();

    guint i;
    for(i = 0; i < proclist.number; ++i)
    {
        pid_t pidCur = pid_list[i];
        Process oldProcInfo = getProcessById(pidCur);
        m_lockReadWrite.lockForWrite();
        Process proc(pidCur);
        if (m_isScanStoped) {
            m_lockReadWrite.unlock();
            break;
        }
        proc.setValid(true);
        proc.UpdateProcInfo();

        // flownet
        qint64 curProcFlownet = 0;
        if (numAddFlowNetPerSec.contains(pidCur)) {
            curProcFlownet = numAddFlowNetPerSec[pidCur];
        }
        if (!oldProcInfo.isValid()) {   // 新进程
            proc.setPreFlownetTime(QDateTime::currentDateTime());
            proc.setPreFlownetCount(curProcFlownet);
            proc.setFlowNet(curProcFlownet-proc.getPreFlownetCount());
            QString addFlownetPerSec = proc.calcFlownetPerSec(curProcFlownet);
            proc.setFlowNetDesc(addFlownetPerSec);
        } else {
            proc.setPreFlownetTime(oldProcInfo.getPreFlownetTime());
            proc.setPreFlownetCount(oldProcInfo.getPreFlownetCount());
            proc.setFlowNet(curProcFlownet-proc.getPreFlownetCount());
            QString addFlownetPerSec = proc.calcFlownetPerSec(curProcFlownet);
            proc.setFlowNetDesc(addFlownetPerSec);
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
        if (!m_isScanStoped) {
            if (oldProcInfo.isValid() && proc.getProcCpuTime() >= oldProcInfo.getProcCpuTime()) {
                proc.setProcCpuTime(oldProcInfo.getProcCpuTime());
            }
        }

        proc.setMemeryUsed(procmem.resident - procmem.share);

        proc.updateProcUser(procstate.uid);

        guint64 difference = proctime.rtime - proc.getProcCpuTime();
        if (difference > 0)
            proc.setStatus(GLIBTOP_PROCESS_RUNNING);
        guint cpu_scale = 100 * this->num_cpus;
        double sPcpu = (gdouble)difference * cpu_scale / this->cpu_total_time;
        sPcpu = MIN(sPcpu, cpu_scale);
        proc.setCpuPercent(sPcpu);

//        CPU 百分比使用 Solaris 模式，工作在“Solaris 模式”，其中任务的 CPU 使用量将被除以总的 CPU 数目。否则它将工作在“Irix 模式”。
        proc.setFrequency(cpu.frequency);
        proc.setProcCpuTime(proctime.rtime);
        proc.setNice(procuid.nice);

        // disk io
        disk_io_bytes_total = procio.disk_wbytes+procio.disk_rbytes;
        if (!oldProcInfo.isValid()) {   // 新进程
            proc.setPreDiskIoTime(QDateTime::currentDateTime());
            proc.setPreDiskIoCount(disk_io_bytes_total);
            proc.setDiskIO(disk_io_bytes_total-proc.getPreDiskIoCount());
            QString addDiskIoPerSec = proc.calcDiskIoPerSec(disk_io_bytes_total);
            proc.setDiskIODesc(addDiskIoPerSec);
        } else {
            proc.setPreDiskIoTime(oldProcInfo.getPreDiskIoTime());
            proc.setPreDiskIoCount(oldProcInfo.getPreDiskIoCount());
            proc.setDiskIO(disk_io_bytes_total-proc.getPreDiskIoCount());
            QString addDiskIoPerSec = proc.calcDiskIoPerSec(disk_io_bytes_total);
            proc.setDiskIODesc(addDiskIoPerSec);
        }

        proc.setProcStatus(formatProcessState(proc.getStatus()));

        if (m_isScanStoped) {
            m_lockReadWrite.unlock();
            break;
        }

        std::string desktopFile;
        desktopFile = getDesktopFileAccordProcNameApp(proc.getProcName(), "");
//        QString q_str = QString::fromStdString(desktopFile);   // this is the way that convert from std::string to QString
        if(desktopFile.empty())  //this is the way to detect that if the std::string is null or not.
        {
            desktopFile = getDesktopFileAccordProcName(proc.getProcName(), "");
        }
        if (!oldProcInfo.isValid()) {
            QString strIconPath = getAppIconPathFromDesktopFile(desktopFile);
            proc.setIconPath(strIconPath);            
        } else {
            proc.setIconPath(oldProcInfo.getIconPath());
        }

        if (m_isScanStoped) {
            m_lockReadWrite.unlock();
            break;
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
                m_lockReadWrite.unlock();
                removeProcess(proc.pid());
                m_lockReadWrite.lockForWrite();
            }
        } else {
            m_set[proc.pid()] = proc;
        }
        m_lockReadWrite.unlock();
    }
    g_free (pid_list);
}

Process ProcessList::getProcessById(pid_t pid)
{
    Process procInfo(pid);
    m_lockReadWrite.lockForRead();
    if (m_set.contains(pid)) {
        procInfo = m_set[pid];
    }
    m_lockReadWrite.unlock();
    return procInfo;
}

QList<pid_t> ProcessList::getPIDList()
{
    QList<pid_t> pidList;
    m_lockReadWrite.lockForRead();
    pidList = m_set.keys();
    m_lockReadWrite.unlock();
    return pidList;
}

void ProcessList::removeProcess(pid_t pid)
{
    m_lockReadWrite.lockForWrite();
    m_set.remove(pid);
    m_lockReadWrite.unlock();
}

void ProcessList::updateProcessState(pid_t pid, QString& state)
{
    m_lockReadWrite.lockForWrite();
    if (m_set.contains(pid))
        m_set[pid].setProcStatus(state);
    m_lockReadWrite.unlock();
}

void ProcessList::updateProcessPriority(pid_t pid, qulonglong priority)
{
    m_lockReadWrite.lockForWrite();
    if (m_set.contains(pid))
        m_set[pid].setNice(priority);
    m_lockReadWrite.unlock();
}

void ProcessList::stopScanProcess()
{
    m_lockReadWrite.lockForWrite();
    m_isScanStoped = true;
    m_lockReadWrite.unlock();
}

void ProcessList::startScanProcess()
{
    m_lockReadWrite.lockForWrite();
    m_isScanStoped = false;
    m_lockReadWrite.unlock();
}

void ProcessList::refreshLine(const QString &procname, quint64 rcv, quint64 sent, int pid, unsigned int uid, const QString &devname)
{
    // record process flownet
    qint64 tmptotalFlowNetPerSec = rcv + sent;
    m_lockReadWrite.lockForWrite();
    if(!flowNetPrevMap.contains(pid))
    {
        flowNetPrevMap[pid] = 0;//save prev data
    }

    flowNetPrevMap[pid] = tmptotalFlowNetPerSec;
    m_lockReadWrite.unlock();
}

} // namespace process
} // namespace sysmonitor
