/*
 * refreshthread.cpp
 *
 * Copyright (c) 2016 Stephane List
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "refreshthread.h"

#include <QtDebug>

#include <arpa/inet.h>


extern ProcList * processes;
extern timeval curtime;
extern Process * unknowntcp;
extern Process * unknownudp;
extern Process * unknownip;


RefreshThread::RefreshThread(QObject *parent) : QThread(parent)
{
    pid_cnx = -1;
}

/** get total values for this process */
void RefreshThread::gettotal(Process * curproc, u_int32_t * recvd, u_int32_t * sent)
{
    u_int32_t sum_sent = 0, sum_recv = 0;
    ConnList * curconn = curproc->connections;
    while (curconn != NULL)
    {
        Connection * conn = curconn->getVal();
        sum_sent += conn->sumSent;
        sum_recv += conn->sumRecv;
        curconn = curconn->getNext();
    }
    *recvd = sum_recv;
    *sent = sum_sent;
}

void RefreshThread::gettotalb(Process * curproc, quint64 * recvd, quint64 * sent)
{
    u_int32_t sum_sent = 0, sum_recv = 0;
    gettotal(curproc, &sum_recv, &sum_sent);
    *sent = sum_sent;
    *recvd = sum_recv;
}

void RefreshThread::refresh_process_list()
{
    ProcList * curproc = processes;
    ProcList * previousproc = NULL;
    int nproc = processes->size();

    while (curproc != NULL)
    {
        // walk though its connections, summing up their data, and
        // throwing away connections that haven't received a package
        // in the last PROCESSTIMEOUT seconds.
        assert (curproc != NULL);
        assert (curproc->getVal() != NULL);
        assert (nproc == processes->size());

        /* remove timed-out processes (unless it's one of the the unknown process) */
        if ((curproc->getVal()->getLastPacket() + PROCESSTIMEOUT <= curtime.tv_sec)
                && (curproc->getVal() != unknowntcp)
                && (curproc->getVal() != unknownudp)
                && (curproc->getVal() != unknownip))
        {
            //qDebug() << "PROC: Deleting process" << curproc->getVal()->name;
            ProcList * todelete = curproc;
            Process * p_todelete = curproc->getVal();

            emit procDeleted(curproc->getVal()->pid);

            if (previousproc)
            {
                previousproc->next = curproc->next;
                curproc = curproc->next;
            } else {
                processes = curproc->getNext();
                curproc = processes;
            }
            delete todelete;
            delete p_todelete;
            nproc--;
        }
        else
        {
            // add a non-timed-out process to the list of stuff to show
            quint64 value_sent = 0, value_recv = 0;

            gettotalb(curproc->getVal(), &value_recv, &value_sent);
            uid_t uid = curproc->getVal()->getUid();

            emit procDetected(curproc->getVal()->name, value_recv, value_sent, curproc->getVal()->pid, uid , curproc->getVal()->devicename);;

            previousproc = curproc;
            curproc = curproc->next;
        }
    }
}

void RefreshThread::update_cnx_list(int pid)
{
    pid_cnx = pid;
}




void RefreshThread::refresh_cnx_list()
{
//    qDebug() << "refresh cnx for pid :" << pid_cnx;

    ProcList * curproc = processes;

    while (curproc != NULL)
    {
        if (curproc->getVal()->pid == pid_cnx) {
            //qDebug() << "TODO IPv6";

            ConnList * curconn = curproc->getVal()->connections;
            while (curconn != NULL)
            {
                Connection * conn = curconn->getVal();
                /*
                qDebug() << "sip" << QHostAddress(ntohl(conn->refpacket->sip.s_addr));
                qDebug() << "dip" << QHostAddress(ntohl(conn->refpacket->dip.s_addr));
                qDebug() << "sport" << conn->refpacket->sport;
                qDebug() << "dport" << conn->refpacket->dport;
*/
                emit cnxDetected(QHostAddress(ntohl(conn->refpacket->sip.s_addr)),
                                 conn->refpacket->sport,
                                 QHostAddress(ntohl(conn->refpacket->dip.s_addr)),
                                 conn->refpacket->dport);
                curconn = curconn->getNext();
            }
        }
        curproc = curproc->next;
    }
}

void RefreshThread::run()
{
    while (1) {
        sleep(1.5);
        refreshconninode();
        refresh_process_list();
        refresh_cnx_list();
        emit refresh_finished();
    }
}
