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

#ifndef REFRESHTHREAD_H
#define REFRESHTHREAD_H

#include <QThread>
#include <QHostAddress>

#include "conninode.h"
#include "process.h"

class RefreshThread : public QThread
{
    Q_OBJECT

private:
    void gettotalb(Process * curproc, quint64 * recvd, quint64 * sent);
    void gettotal(Process * curproc, u_int32_t * recvd, u_int32_t * sent);
    void refresh_process_list();
    void refresh_cnx_list();
    int pid_cnx;

public:
    explicit RefreshThread(QObject *parent = 0);
    void run();

signals:
    void procDetected(const QString& procname, quint64 rcv, quint64 sent, int pid, unsigned int uid, const QString& devname);
    void procDeleted(int pid);
    void refresh_finished();
    void cnxDetected(const QHostAddress& sourceip, quint16 sport, const QHostAddress& destip, quint16 dport);

public slots:
    void update_cnx_list(int pid);

};

#endif // REFRESHTHREAD_H
