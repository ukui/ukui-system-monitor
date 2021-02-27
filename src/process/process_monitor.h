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

#ifndef __PROCESS_MONITOR_H__
#define __PROCESS_MONITOR_H__

#include <QReadWriteLock>
#include <QObject>
#include <QBasicTimer>
#include <QThread>

#include <memory>

namespace sysmonitor {
namespace process {

class ProcessList;
class ProcessMonitor;

class ProcessMonitorThread : public QThread
{
    Q_OBJECT
    Q_DISABLE_COPY(ProcessMonitorThread)

public:
    explicit ProcessMonitorThread(QObject *parent = nullptr);
    ~ProcessMonitorThread();

public:
    static ProcessMonitorThread *instance();
    
    ProcessMonitor *procMonitorInstance() const;

private:
    ProcessMonitor *m_monitor;
};

class ProcessMonitor : public QObject
{
    Q_OBJECT

signals:
    void procInfoUpdated();
    void clearProcessList();

public:
    explicit ProcessMonitor(QObject *parent = nullptr);
    virtual ~ProcessMonitor();

    static ProcessMonitor *instance();

    ProcessList *processList();

    void startMonitorJob();
    // atomic operation, can be called from main thread (gui) when exit requested
    void requestInterrupt();    

public slots:
    void updateProcMonitorInfo();
    void onChangeRefreshFilter(QString strFilter);

private:
    ProcessList *m_processList = nullptr;

    QBasicTimer m_basicTimer;
};

} //namespace process
} //namespace sysmonitor

#endif //__PROCESS_MONITOR_H__