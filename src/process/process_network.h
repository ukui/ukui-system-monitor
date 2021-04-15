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

#ifndef __PROCESS_NETWORK_H__
#define __PROCESS_NETWORK_H__

#include <QThread>
#include <map>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <cassert>
#include <string>
#include <malloc.h>
#include <iostream>
#include <vector>
using namespace std;

extern "C" {
#include "../singleProcessNet/decpcap.h"
}

#define _BSD_SOURCE 1

/* take the average speed over the last 5 seconds */
#define PERIOD 5

/* the amount of time after the last packet was recieved
 * after which a process is removed */
#define PROCESSTIMEOUT 150

/* the amount of time after the last packet was recieved
 * after which a connection is removed */
#define CONNTIMEOUT 50

#define DEBUG 0

#define REVERSEHACK 0

// 2 times: 32 characters, 7 ':''s, a ':12345'.
// 1 '-'
// -> 2*45+1=91. we make it 92, for the null.
#define HASHKEYSIZE 256

#define PROGNAME_WIDTH 512

// viewMode: how to represent numbers
#define VIEWMODE_KBPS     0
#define VIEWMODE_TOTAL_KB 1
#define VIEWMODE_TOTAL_B  2
#define VIEWMODE_TOTAL_MB 3
#define VIEWMODE_COUNT    4

#define MAX_PID_LENGTH    20

typedef struct _LocalAddr_s
{
    in_addr_t addr;
	struct in6_addr addr6;
	short int sa_family;
    string str_ipinfo;
}LocalAddr;

typedef struct _ProcessNetInfo_s
{
    string strName;
    uid_t procUid;
    string strDevName;
    unsigned long long lluRecvCount;
    unsigned long long lluSendCount;
    quint64 lastTickCount;
    vector<unsigned long> procINodes;
}ProcessNetInfo;

typedef struct _ProcessNetPacket_s
{
    in6_addr sip6;
	in6_addr dip6;
	in_addr sip;
	in_addr dip;
	unsigned short sport;
	unsigned short dport;
	u_int32_t len;
	timeval time;
    short int sa_family;
    string strHashKey;
    string strDevName;
    uint8_t uIpProtocol;
}ProcessNetPacket;

class ProcessNetwork : public QThread
{
    Q_OBJECT
public:
    explicit ProcessNetwork(QObject *parent);
    virtual ~ProcessNetwork();
    bool initNetInfo();
    void deinitNetInfo();

    bool attachPacketToProcess(ProcessNetPacket& procNetPacket);
    bool attachPacketToProcess(unsigned long luInode, ProcessNetPacket& procNetPacket);

    void run();
    void stop();

signals:
    void procDetected(const QString& procname, quint64 rcv, quint64 sent, int pid, unsigned int uid, const QString& devname);
    void procDeleted(int pid);

private:
    bool initNetDevPcapHandle();
    bool deinitNetDevPcapHandle();
    void findAllLocalAddr();
    void addLocalAddr(in_addr_t addr);
    void addLocalAddr(string strIpOrig);
    bool localAddrContains(const in_addr_t & n_addr);
    bool localAddrContains(const struct in6_addr & n_addr);
    
    void refreshConnINodes();
    int searchConnINodes(string strConnFile);
    int searchConnINodes(int family, int proto, string strConnFile);

    void refreshProcNetInfo();
    void checkProcessInfo();

    bool isPacketOutgoing(ProcessNetPacket& procNetPacket);
    string getPacketHashkey(ProcessNetPacket& procNetPacket);

public:
    ProcessNetPacket m_tempNetPacket;
    quint64 m_refreshProcNetInfoTick = 0;
    quint64 m_updateProcNetInfoTick = 0;

private:
    vector<LocalAddr> m_listLocalAddr;
    map<string, unsigned long> m_mapConnectionINode;
    map<pid_t, ProcessNetInfo> m_mapProcNetInfo;
    map<string, dp_handle*> m_mapNetDeviceHandle;
    
    bool m_isStoped = false;
};

#endif // __PROCESS_NETWORK_H__
