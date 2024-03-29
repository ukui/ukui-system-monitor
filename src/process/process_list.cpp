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
#include "../desktopfileinfo.h"

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

unsigned Process::getProcThreads() const
{
    return d->m_uThreads;
}

void Process::setProcThreads(unsigned uThreads)
{
    if (d) {
        uThreads = uThreads == 0 ? 1 : uThreads;
        d->m_uThreads = uThreads;
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
        speedPerSec = "0 KB/s"; }
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

QString Process::calcFlownetPerSec(double lfValue, bool isSpeed)
{
    if (!isSpeed) {
        qint64 bandwith = lfValue - getPreFlownetCount();
        QDateTime now = QDateTime::currentDateTime();
        quint64 ms_lapse = d->m_preFlownetTime.msecsTo(now);
        QString speedPerSec = "";

        if (ms_lapse < 1000) ms_lapse = 1000; // prevent division by 0 ;-)
        qint64 speed = bandwith * 1000 / ms_lapse;

        if (speed == 0 || speed < 0) {
            speedPerSec = "0 KB/s"; 
        } else if (speed < 1900) {
            speedPerSec.setNum(speed);
            speedPerSec.append(" B/s");
        } else if (speed < 1900000) {
            speedPerSec.setNum(speed/1024);
            speedPerSec.append(" KB/s");
        } else if (speed < 1900000000) {
            speedPerSec.setNum(speed/(1024*1024));
            speedPerSec.append(" MB/s"); 
        } else {
            speedPerSec.setNum(speed/(1024*1024*1024));
            speedPerSec.append(" GB/s");
        }

        d->m_preFlownetTime.swap(now);
        setPreFlownetCount(lfValue);
        return speedPerSec;
    } else {
        QString speedPerSec = "";
        if (lfValue < 0)
            lfValue = 0;
        if (lfValue == 0 || lfValue < 0) {
            speedPerSec = "0 KB/s"; 
        } else if (lfValue < 1024) {
            speedPerSec.setNum(lfValue, 'f', 1);
            speedPerSec.append(" KB/s"); 
        } else if (lfValue < 1024 * 1024) {
            speedPerSec.setNum(lfValue/1024, 'f', 1);
            speedPerSec.append(" MB/s"); 
        } else {
            speedPerSec.setNum(lfValue/(1024*1024), 'f', 1);
            speedPerSec.append(" GB/s");
        }
        return speedPerSec;
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

bool Process::getProcStat()
{
    bool b = false;
    char path[256];
    QScopedArrayPointer<char> buf(new char[1025] {});
    int fd, rc;
    ssize_t sz;
    char *pos, *begin;

    errno = 0;
    sprintf(path, "/proc/%d/stat", pid());
    // open /proc/[pid]/stat
    if ((fd = open(path, O_RDONLY)) < 0) {
        return b;
    }

    // read data
    sz = read(fd, buf.data(), 1024);
    if (sz < 0) {
        close(fd);
        return b;
    }
    buf.data()[sz] = '\0';
    close(fd);

    // get process name between (...)
    begin = strchr(buf.data(), '(');
    begin += 1;

    pos = strrchr(buf.data(), ')');
    if (!pos) {
        return b;
    }

    *pos = '\0';
    // process name (may be truncated by kernel if it's too long)
    // setProcName(QString(begin));

    pos += 2;
    char state;                   // process state
    pid_t ppid;                   // parent process
    pid_t pgid;                   // process group id
    unsigned long long utime;     // user time
    unsigned long long stime;     // kernel time
    long long cutime;             // user time on waiting children
    long long cstime;             // kernel time on waiting children
    unsigned long long start_time;  // start time since system boot in clock ticks
    unsigned int processor;         // cpu number
    unsigned int rt_prio;           // real time priority
    unsigned int policy;            // scheduling policy
    unsigned int nthreads;          // number of threads
    int nice;                   // process nice
    unsigned long long guest_time;  // guest time (virtual cpu time for guest os)
    long long cguest_time;          // children guest time in clock ticks

    //****************3**4**5*******************************************14***15**
    rc = sscanf(pos, "%c %d %d %*d %*d %*d %*u %*u %*u %*u %*u %llu %llu"
                //*16***17******19*20******22************************************
                " %lld %lld %*d %d %u %*u %llu %*u %*u %*u %*u %*u %*u %*u %*u"
                //********************************39*40*41******43***44**********
                " %*u %*u %*u %*u %*u %*u %*u %*u %u %u %u %*u %llu %lld\n",
                &state,           // 3
                &ppid,            // 4
                &pgid,            // 5
                &utime,           // 14
                &stime,           // 15
                &cutime,          // 16
                &cstime,          // 17
                &nice,            // 19
                &nthreads,        // 20
                &start_time,      // 22
                &processor,       // 39
                &rt_prio,         // 40
                &policy,          // 41
                &guest_time,      // 43
                &cguest_time);    // 44
    if (rc < 15) {
        return b;
    }
    // have guest & cguest time
    if (rc < 17) {
        guest_time = cguest_time = 0;
    }
    setProcThreads(nthreads);
    return true;
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
    getProcStat();
}

ProcessList::ProcessList(QObject* parent)
    : QObject(parent), m_set {}
{
    glibtop_init();
    this->num_cpus = getCpuCount();//glibtop_get_sysinfo()->ncpu;
    procNetThread = new ProcessNetwork(this);
    procNetThread->start(QThread::LowPriority);

    connect(procNetThread, SIGNAL(procDetected(const QString &, double , double , int , unsigned int , const QString&)),
             this, SLOT(refreshLine(const QString &, double , double , int, unsigned int , const QString&)));

    // fill shell list
    [ = ] {
        FILE *fp;
        fp = fopen("/etc/shells", "r");
        if (fp)
        {
            char buf[128] {};
            char *s;
            while ((s = fgets(buf, 128, fp))) {
                if (s[0] == '/') {
                    auto sh = QLatin1String(basename(s));
                    if (sh.endsWith('\n'))
                        sh.chop(1);
                    if (!m_shellList.contains(sh)) {
                        m_shellList << sh;
                    }
                }
            }
        }
    }();
}

ProcessList::~ProcessList()
{
    if(procNetThread)
    {
        procNetThread->stop();
        procNetThread->wait();
    }
    
    glibtop_close();
}

void ProcessList::connectNetStateRefresh()
{
    connect(procNetThread, SIGNAL(procDetected(const QString &, double , double , int , unsigned int , const QString&)),
             this, SLOT(refreshLine(const QString &, double , double , int, unsigned int , const QString&)));
}

void ProcessList::disconnectNetStateRefresh()
{
    disconnect(procNetThread, SIGNAL(procDetected(const QString &, double , double , int , unsigned int , const QString&)),
             this, SLOT(refreshLine(const QString &, double , double , int, unsigned int , const QString&)));
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
    bool bFilterActive = false;

    m_lockReadWrite.lockForRead();
    if (m_strFilter == "all")
    {
        which = GLIBTOP_KERN_PROC_ALL;
        arg = 0;
    }
    else if (m_strFilter == "active")
    {
        which = GLIBTOP_KERN_PROC_ALL /*| GLIBTOP_EXCLUDE_IDLE*/;
        bFilterActive = true;
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
        if (!oldProcInfo.isValid()) {   // 新进程
            proc.setPreFlownetTime(QDateTime::currentDateTime());
            proc.setPreFlownetCount(0);
            proc.setFlowNet(0);
            QString addFlownetPerSec = proc.calcFlownetPerSec(0);
            proc.setFlowNetDesc(addFlownetPerSec);
        } else {
            proc.setPreFlownetTime(oldProcInfo.getPreFlownetTime());
            proc.setPreFlownetCount(oldProcInfo.getPreFlownetCount());
            proc.setFlowNet(oldProcInfo.getFlowNet());
            proc.setFlowNetDesc(oldProcInfo.getFlowNetDesc());
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
            if (oldProcInfo.isValid()) {
                proc.setProcCpuTime(oldProcInfo.getProcCpuTime());
            } else {
                if (m_mapCpuTimes.contains(pidCur) && proctime.rtime >= m_mapCpuTimes[pidCur]) {
                    proc.setProcCpuTime(m_mapCpuTimes[pidCur]);
                } else {
                    proc.setProcCpuTime(proctime.rtime);
                }
            }
        }

        proc.setMemeryUsed(procmem.resident - procmem.share);

        proc.updateProcUser(procstate.uid);

        guint64 difference = proctime.rtime - proc.getProcCpuTime();
        if (difference > 0)
            proc.setStatus(GLIBTOP_PROCESS_RUNNING);
        guint cpu_scale = 100 * this->num_cpus;
        double sPcpu = (double)difference * cpu_scale / this->cpu_total_time;
        sPcpu = MIN(sPcpu, cpu_scale);
        if (proc.getProcThreads() > 0) {
            sPcpu = MIN(sPcpu, proc.getProcThreads()*100);
        }
        proc.setCpuPercent(sPcpu);

//        CPU 百分比使用 Solaris 模式，工作在“Solaris 模式”，其中任务的 CPU 使用量将被除以总的 CPU 数目。否则它将工作在“Irix 模式”。
        proc.setFrequency(cpu.frequency);
        proc.setProcCpuTime(proctime.rtime);
        m_mapCpuTimes[pidCur] = proctime.rtime;
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
        
        if (!oldProcInfo.isValid()) {
            QString strIconPath = DesktopFileInfo::instance()->getIconByExec(proc.getProcName());
            if (strIconPath.isEmpty())
                strIconPath = DesktopFileInfo::instance()->getAndroidAppIconByCmd(proc.getProcArgments());
            proc.setIconPath(strIconPath);
        } else {
            if (proc.getProcName() != oldProcInfo.getProcName()) {
                QString strIconPath = DesktopFileInfo::instance()->getIconByExec(proc.getProcName());
                if (strIconPath.isEmpty())
                    strIconPath = DesktopFileInfo::instance()->getAndroidAppIconByCmd(proc.getProcArgments());
                proc.setIconPath(strIconPath);
            } else {
                proc.setIconPath(oldProcInfo.getIconPath());
            }
        }

        if (m_isScanStoped) {
            m_lockReadWrite.unlock();
            break;
        }

        QString title = DesktopFileInfo::instance()->getNameByExec(proc.getProcName());
        if (title.isEmpty()) {
            title = DesktopFileInfo::instance()->getAndroidAppNameByCmd(proc.getProcArgments());
            if (title.isEmpty()) {
                proc.setDisplayName(proc.getProcName()); //进程名称
            } else {
                proc.setDisplayName(title);
            }
        } else {
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
        } else if (bFilterActive && proc.getStatus() != GLIBTOP_PROCESS_RUNNING) {
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

unsigned ProcessList::getCpuCount()
{
    FILE* file = fopen("/proc/stat", "r");
    if (file == NULL) {
        qCritical("Cannot open /proc/stat!");
        return 1;
    }
    unsigned cpus = 0;
    do {
        char buffer[4096+1] = {0};
        if (fgets(buffer, 4096+1, file) == NULL) {
            break;
        } else if (g_str_has_prefix(buffer, "cpu")) {
            cpus++;
        }
    } while(true);

    fclose(file);
    cpus = MAX(cpus - 1, 1);
    return cpus;
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

void ProcessList::refreshLine(const QString &procname, double rcv, double sent, int pid, unsigned int uid, const QString &devname)
{
    // record process flownet
    #if 0
    qint64 tmptotalFlowNetPerSec = rcv + sent;
    m_lockReadWrite.lockForWrite();
    if (m_set.contains(pid) && m_set[pid].isValid()) {
        if (tmptotalFlowNetPerSec < m_set[pid].getPreFlownetCount()) {
            m_set[pid].setPreFlownetCount(tmptotalFlowNetPerSec);
            m_set[pid].setFlowNet(tmptotalFlowNetPerSec);
        } else {
            m_set[pid].setFlowNet(tmptotalFlowNetPerSec-m_set[pid].getPreFlownetCount());
        }
        QString addFlownetPerSec = m_set[pid].calcFlownetPerSec(tmptotalFlowNetPerSec);
        m_set[pid].setFlowNetDesc(addFlownetPerSec);
    }
    m_lockReadWrite.unlock();
    #else
    double tmptotalFlowNetPerSec = rcv + sent;
    m_lockReadWrite.lockForWrite();
    if (m_set.contains(pid) && m_set[pid].isValid()) {
        m_set[pid].setPreFlownetCount(tmptotalFlowNetPerSec);
        m_set[pid].setFlowNet(tmptotalFlowNetPerSec);
        QString addFlownetPerSec = m_set[pid].calcFlownetPerSec(tmptotalFlowNetPerSec, true);
        m_set[pid].setFlowNetDesc(addFlownetPerSec);
    }
    m_lockReadWrite.unlock();
    #endif
}

bool ProcessList::isShellCmd(QString strCmd)
{
    if (strCmd.isEmpty())
        return false;
    return m_shellList.contains(strCmd);
}

} // namespace process
} // namespace sysmonitor
