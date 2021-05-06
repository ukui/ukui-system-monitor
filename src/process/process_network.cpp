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

#include "process_network.h"

#include <QDateTime>
#include <QDebug>
#include <unistd.h>
#include <net/if.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

static char * stripspaces (char * input);

ProcessNetwork::ProcessNetwork(QObject *parent)
    : QThread(parent)
{
    m_mapNetDeviceHandle.clear();
    m_listLocalAddr.clear();
}

ProcessNetwork::~ProcessNetwork()
{
    deinitNetInfo();
}

bool ProcessNetwork::initNetInfo()
{
    findAllLocalAddr();
    initNetDevPcapHandle();
    return true;
}

void ProcessNetwork::deinitNetInfo()
{
    deinitNetDevPcapHandle();
    m_listLocalAddr.clear();
}

void ProcessNetwork::findAllLocalAddr()
{
    struct ifaddrs *ifaddr, *ifa;

	if (getifaddrs(&ifaddr) == -1) 
	{
		qDebug() << "Fail to get interface addresses";
		return;
	}
    // get all net device name
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
	{
		if (ifa->ifa_addr == NULL)  
			continue;  

		// The interface is up, not a loopback and running ?
		if ( !(ifa->ifa_flags & IFF_LOOPBACK) && 
			 (ifa->ifa_flags & IFF_UP) &&
			 (ifa->ifa_flags & IFF_RUNNING) )
		{
			// Check if the interface is already known by going through all the devices
            string strDeviceName = ifa->ifa_name;
            if (m_mapNetDeviceHandle.find(strDeviceName) == m_mapNetDeviceHandle.end()) {
                m_mapNetDeviceHandle[strDeviceName] = NULL;
            }
		}
	}
	freeifaddrs(ifaddr);
    // get addrs by devicename
    map<string, dp_handle*>::iterator itNetDevHandle = m_mapNetDeviceHandle.begin();
    for (; itNetDevHandle != m_mapNetDeviceHandle.end(); itNetDevHandle++) {
        if (!itNetDevHandle->first.empty()) {
            int sock;
            struct ifreq iFreq;
            struct sockaddr_in *saddr;

            if((sock=socket(AF_INET, SOCK_DGRAM, 0))<0)
            {
        		qDebug()<<"creating socket failed while establishing local IP - are you root?";
                return ;
            }
            strncpy(iFreq.ifr_name, itNetDevHandle->first.c_str(), IFNAMSIZ-1);

            if(ioctl(sock, SIOCGIFADDR, &iFreq)<0) {
                qDebug() << QString("ioctl failed while establishing local IP for selected device %1. You may specify the device on the command line.").arg(itNetDevHandle->first.c_str());
                close(sock);
                return;
            }

            saddr=(struct sockaddr_in*)&iFreq.ifr_addr;
            addLocalAddr(saddr->sin_addr.s_addr);
            /* also get local IPv6 addresses */
            FILE *ifinfo = fopen ("/proc/net/if_inet6", "r");
            char buffer[500] = {0};
            if (ifinfo)
            {
                do
                {
                    if (fgets(buffer, sizeof(buffer), ifinfo))
                    {
                        char address[33] = {0};
                        char ifname[9] = {0};
                        int n_results = sscanf (buffer, "%32[0-9a-f] %*d %*d %*d %*d %8[0-9a-zA-Z]", address, ifname);
                        if (n_results == 2 && strcmp(stripspaces(ifname), itNetDevHandle->first.c_str()) == 0) {
                            addLocalAddr(address);
                        }
                    }
                } while (!feof(ifinfo));
                fclose(ifinfo);
            }
            close(sock);
        }
    }    
}

static int process_tcp (u_char * userdata, const dp_header * header, const u_char * m_packet) {
    ProcessNetwork * args = (ProcessNetwork *) userdata;
    struct tcphdr * tcp = (struct tcphdr *) m_packet;

    /* get info from userdata, then call attachPacketToProcess */
    ProcessNetPacket sProcNetPacket = args->m_tempNetPacket;
    sProcNetPacket.uIpProtocol = IPPROTO_TCP;
    switch (sProcNetPacket.sa_family)
    {
    case (AF_INET):
        sProcNetPacket.sport = ntohs(tcp->source);
        sProcNetPacket.dport = ntohs(tcp->dest);
        sProcNetPacket.len = header->len;
        sProcNetPacket.time = header->ts;
        break;
    case (AF_INET6):
        sProcNetPacket.sport = ntohs(tcp->source);
        sProcNetPacket.dport = ntohs(tcp->dest);
        sProcNetPacket.len = header->len;
        sProcNetPacket.time = header->ts;
        break;
    }
    args->attachPacketToProcess(sProcNetPacket);
    return 1;
}

static int process_udp (u_char * userdata, const dp_header * header, const u_char * m_packet) {
    ProcessNetwork * args = (ProcessNetwork *) userdata;
    struct udphdr * udp = (struct udphdr *) m_packet;

    ProcessNetPacket sProcNetPacket = args->m_tempNetPacket;
    sProcNetPacket.uIpProtocol = IPPROTO_UDP;
    switch (sProcNetPacket.sa_family)
    {
    case (AF_INET):
        sProcNetPacket.sport = ntohs(udp->source);
        sProcNetPacket.dport = ntohs(udp->dest);
        sProcNetPacket.len = header->len;
        sProcNetPacket.time = header->ts;
        break;
    case (AF_INET6):
        sProcNetPacket.sport = ntohs(udp->source);
        sProcNetPacket.dport = ntohs(udp->dest);
        sProcNetPacket.len = header->len;
        sProcNetPacket.time = header->ts;
        break;
    }
    args->attachPacketToProcess(sProcNetPacket);
    return 1;
}

static int process_ip (u_char * userdata, const dp_header * /* header */, const u_char * m_packet) {
    ProcessNetwork * args = (ProcessNetwork *) userdata;
    struct ip * ip = (struct ip *) m_packet;
    args->m_tempNetPacket.sa_family = AF_INET;
    args->m_tempNetPacket.sip = ip->ip_src;
    args->m_tempNetPacket.dip = ip->ip_dst;

    return 0;
}

static int process_ip6 (u_char * userdata, const dp_header * /* header */, const u_char * m_packet) {
    ProcessNetwork * args = (ProcessNetwork *) userdata;
    const struct ip6_hdr * ip6 = (struct ip6_hdr *) m_packet;

    args->m_tempNetPacket.sa_family = AF_INET6;
    args->m_tempNetPacket.sip6 = ip6->ip6_src;
    args->m_tempNetPacket.dip6 = ip6->ip6_dst;

    return 0;
}

bool ProcessNetwork::initNetDevPcapHandle()
{
    int promisc = 0;
    char errbuf[PCAP_ERRBUF_SIZE];
    map<string, dp_handle*>::iterator itNetDevHandle = m_mapNetDeviceHandle.begin();
    for (; itNetDevHandle != m_mapNetDeviceHandle.end(); ) {
        if (!itNetDevHandle->second) {
            itNetDevHandle->second = dp_open_live(itNetDevHandle->first.c_str(), BUFSIZ, promisc, 100, errbuf);
            if (itNetDevHandle->second)
            {
                dp_addcb (itNetDevHandle->second, dp_packet_ip, process_ip);
                dp_addcb (itNetDevHandle->second, dp_packet_ip6, process_ip6);
                dp_addcb (itNetDevHandle->second, dp_packet_tcp, process_tcp);
                dp_addcb (itNetDevHandle->second, dp_packet_udp, process_udp);

                if (dp_setnonblock (itNetDevHandle->second, 1, errbuf) == -1)
                {
                    qCritical() <<  "Error putting libpcap in nonblocking mode";
                }
                itNetDevHandle++;
            }
            else
            {
                qCritical() << "Error opening handler for device " << itNetDevHandle->first.c_str();
                m_mapNetDeviceHandle.erase(itNetDevHandle++);
            }
        }
    }
    return true;
}

bool ProcessNetwork::deinitNetDevPcapHandle()
{
    map<string, dp_handle*>::iterator itNetDevHandle = m_mapNetDeviceHandle.begin();
    for (; itNetDevHandle != m_mapNetDeviceHandle.end(); itNetDevHandle++) {
        if (itNetDevHandle->second) {
            if (itNetDevHandle->second->pcap_handle) {
                pcap_close(itNetDevHandle->second->pcap_handle);
                itNetDevHandle->second->pcap_handle = NULL;
            }
            free(itNetDevHandle->second);
            itNetDevHandle->second = NULL;
        }
    }
    m_mapNetDeviceHandle.clear();
    return true;
}

bool ProcessNetwork::attachPacketToProcess(unsigned long luInode, ProcessNetPacket& procNetPacket)
{
    map<pid_t, ProcessNetInfo>::iterator itProcNetInfo = m_mapProcNetInfo.begin();
    bool bMatchPro = false;
    for (; itProcNetInfo != m_mapProcNetInfo.end(); itProcNetInfo++) {
        if (find(itProcNetInfo->second.procINodes.begin(), itProcNetInfo->second.procINodes.end(),
            luInode) != itProcNetInfo->second.procINodes.end()) {
            if (isPacketOutgoing(procNetPacket)) {
                itProcNetInfo->second.lluSendCount += procNetPacket.len;
            } else {
                itProcNetInfo->second.lluRecvCount += procNetPacket.len;
            }
            itProcNetInfo->second.lastTickCount = QDateTime::currentDateTime().toMSecsSinceEpoch();
            // qDebug()<<"attachPacketToProcess:"<<QString::fromStdString(packetHashKey)<<"|pid:"<<itProcNetInfo->first<<"|"
            //     <<itProcNetInfo->second.lluSendCount<<"|"<<itProcNetInfo->second.lluRecvCount<<"|ts:"<<itProcNetInfo->second.lastTickCount;
            bMatchPro = true;
        }
    }
    return bMatchPro;
}

bool ProcessNetwork::attachPacketToProcess(ProcessNetPacket& procNetPacket)
{
    string packetHashKey = getPacketHashkey(procNetPacket);
    map<string, unsigned long>::iterator itConnINode = m_mapConnectionINode.find(packetHashKey);
    if (itConnINode != m_mapConnectionINode.end()) {
        if (!attachPacketToProcess(itConnINode->second, procNetPacket)) {
            refreshProcNetInfo();
            if (!attachPacketToProcess(itConnINode->second, procNetPacket)) {
                return false;
            }
        }
    } else {
        refreshConnINodes();
        map<string, unsigned long>::iterator itConnINode = m_mapConnectionINode.find(packetHashKey);
        if (itConnINode != m_mapConnectionINode.end()) {
            if (!attachPacketToProcess(itConnINode->second, procNetPacket)) {
                refreshProcNetInfo();
                if (!attachPacketToProcess(itConnINode->second, procNetPacket)) {
                    return false;                
                }
            }
        } else {
            return false; 
        }
    }
    return true;
}

void ProcessNetwork::addLocalAddr(in_addr_t addr)
{
    LocalAddr localAddr;
    localAddr.addr = addr;
    localAddr.sa_family = AF_INET;
    char strAddr[16] = {0};
    inet_ntop (AF_INET, &addr, strAddr, 15);
    localAddr.str_ipinfo = strAddr;
    m_listLocalAddr.push_back(localAddr);
}

void ProcessNetwork::addLocalAddr(string strIpOrig)
{
    if (strIpOrig.size() < 32) 
        return;
    char address [40] = {0};
    address[0] = strIpOrig[0]; address[1] = strIpOrig[1];
    address[2] = strIpOrig[2]; address[3] = strIpOrig[3];
    address[4] = ':';
    address[5] = strIpOrig[4]; address[6] = strIpOrig[5];
    address[7] = strIpOrig[6]; address[8] = strIpOrig[7];
    address[9] = ':';
    address[10] = strIpOrig[8]; address[11] = strIpOrig[9];
    address[12] = strIpOrig[10]; address[13] = strIpOrig[11];
    address[14] = ':';
    address[15] = strIpOrig[12]; address[16] = strIpOrig[13];
    address[17] = strIpOrig[14]; address[18] = strIpOrig[15];
    address[19] = ':';
    address[20] = strIpOrig[16]; address[21] = strIpOrig[17];
    address[22] = strIpOrig[18]; address[23] = strIpOrig[19];
    address[24] = ':';
    address[25] = strIpOrig[20]; address[26] = strIpOrig[21];
    address[27] = strIpOrig[22]; address[28] = strIpOrig[23];
    address[29] = ':';
    address[30] = strIpOrig[24]; address[31] = strIpOrig[25];
    address[32] = strIpOrig[26]; address[33] = strIpOrig[27];
    address[34] = ':';
    address[35] = strIpOrig[28]; address[36] = strIpOrig[29];
    address[37] = strIpOrig[30]; address[38] = strIpOrig[31];
    address[39] = 0;
    struct in6_addr addr6;
    int result = inet_pton (AF_INET6, address, &addr6);
    if (result <= 0) {
        return;
    }
    LocalAddr localAddr;
    localAddr.addr6 = addr6;
    localAddr.sa_family = AF_INET6;
    localAddr.str_ipinfo = address;
    m_listLocalAddr.push_back(localAddr);
}

bool ProcessNetwork::localAddrContains(const in_addr_t & n_addr)
{
    vector<LocalAddr>::iterator itLocalAddr = m_listLocalAddr.begin();
    for (; itLocalAddr != m_listLocalAddr.end(); itLocalAddr++) {
        if ((itLocalAddr->sa_family == AF_INET) && (itLocalAddr->addr == n_addr))
            return true;
    }
    return false;
}

bool ProcessNetwork::localAddrContains(const struct in6_addr & n_addr)
{
    vector<LocalAddr>::iterator itLocalAddr = m_listLocalAddr.begin();
    for (; itLocalAddr != m_listLocalAddr.end(); itLocalAddr++) {
        if ((itLocalAddr->sa_family == AF_INET6) && (memcmp (&itLocalAddr->addr6, &n_addr, sizeof(struct in6_addr)) == 0))
            return true;
    }
    return false;
}

void ProcessNetwork::refreshConnINodes()
{
    m_mapConnectionINode.clear();
    #if 0
    searchConnINodes("/proc/net/tcp");
    searchConnINodes("/proc/net/tcp6");
    searchConnINodes("/proc/net/udp");
    searchConnINodes("/proc/net/udp6");
    #else 
    searchConnINodes(AF_INET, IPPROTO_TCP, "/proc/net/tcp");
    searchConnINodes(AF_INET, IPPROTO_UDP, "/proc/net/udp");
    searchConnINodes(AF_INET6, IPPROTO_TCP, "/proc/net/tcp6");
    searchConnINodes(AF_INET6, IPPROTO_UDP, "/proc/net/udp6");
    #endif
}

int ProcessNetwork::searchConnINodes(int family, int proto, string strConnFile)
{
    FILE *fp {};
    const size_t BLEN = 4096;
    QByteArray buffer {BLEN, 0};
    int nr {};
    ino_t ino {};
    char s_addr[128] {}, d_addr[128] {};
    QByteArray fmtbuf {};
    QString patternA {}, patternB {};

    if (!(fp = fopen(strConnFile.c_str(), "r")))
    {
        return -2;
    }

    while (fgets(buffer.data(), BLEN, fp))
    {
        in6_addr in6_src {};
        in6_addr in6_des {};
        in_addr in4_src {};
        in_addr in4_des {};
        short int sa_family {};
        unsigned src_port = 0;
        unsigned des_port = 0;
        uid_t uid = 0;
        char saddr_str[INET_ADDRSTRLEN + 1] {}, daddr_str[INET_ADDRSTRLEN + 1] {};
        
        //*****************************************************************
        nr = sscanf(buffer.data(), "%*s %64[0-9A-Fa-f]:%x %64[0-9A-Fa-f]:%x %*x %*s %*s %*s %u %*u %ld",
                    s_addr,
                    &src_port,
                    d_addr,
                    &des_port,
                    &uid,
                    &ino);

        // ignore first line
        if (nr == 0)
            continue;

        // socket still in waiting state
        if (ino == 0) {
            continue;
        }

        sa_family = family;

        // saddr & daddr
        if (family == AF_INET6) {
            sscanf(s_addr, "%08x%08x%08x%08x",
                    &in6_src.s6_addr32[0],
                    &in6_src.s6_addr32[1],
                    &in6_src.s6_addr32[2],
                    &in6_src.s6_addr32[3]);
            sscanf(d_addr, "%08x%08x%08x%08x",
                    &in6_des.s6_addr32[0],
                    &in6_des.s6_addr32[1],
                    &in6_des.s6_addr32[2],
                    &in6_des.s6_addr32[3]);
            // convert ipv4 mapped ipv6 address to ipv4
            if (in6_src.s6_addr32[0] == 0x0 &&
                    in6_src.s6_addr32[1] == 0x0 &&
                    in6_src.s6_addr32[2] == 0xffff0000) {
                sa_family = AF_INET;
                in4_src.s_addr = in6_src.s6_addr32[3];
                in4_des.s_addr = in6_des.s6_addr32[3];
            }
        } else {
            sscanf(s_addr, "%x", &in4_src.s_addr);
            sscanf(d_addr, "%x", &in4_des.s_addr);
        }

        if (sa_family == AF_INET) {
            inet_ntop(AF_INET, &in4_src, saddr_str, INET_ADDRSTRLEN);
            inet_ntop(AF_INET, &in4_des, daddr_str, INET_ADDRSTRLEN);
            patternA = QString("%1:%2-%3:%4").arg(saddr_str).arg(src_port).arg(daddr_str).arg(des_port);
            if (proto == IPPROTO_TCP) {
                patternB = QString("%1:%2-%3:%4").arg(daddr_str).arg(des_port).arg(saddr_str).arg(src_port);
            }
        } else if (sa_family == AF_INET6) {
            inet_ntop(AF_INET6, &in6_src, saddr_str, INET6_ADDRSTRLEN);
            inet_ntop(AF_INET6, &in6_des, daddr_str, INET6_ADDRSTRLEN);

            patternA = QString("%1:%2-%3:%4").arg(saddr_str).arg(src_port).arg(daddr_str).arg(des_port);
            if (proto == IPPROTO_TCP) {
                patternB = QString("%1:%2-%3:%4").arg(daddr_str).arg(des_port).arg(saddr_str).arg(src_port);
            }
        } else {
            // unexpected here
        }

        fmtbuf = patternA.toLocal8Bit();
        m_mapConnectionINode[fmtbuf.toStdString()] = ino;

        /* workaround: sometimes, when a connection is actually from 172.16.3.1 to
        * 172.16.3.3, packages arrive from 195.169.216.157 to 172.16.3.3, where
        * 172.16.3.1 and 195.169.216.157 are the local addresses of different 
        * interfaces */
        vector<LocalAddr>::iterator itLocalAddr = m_listLocalAddr.begin();
        for (; itLocalAddr != m_listLocalAddr.end(); itLocalAddr++) {
            /* TODO maybe only add the ones with the same sa_family */
            QString patternC = QString("%1:%2-%3:%4").arg(itLocalAddr->str_ipinfo.c_str()).arg(src_port).arg(daddr_str).arg(des_port);
            fmtbuf = patternA.toLocal8Bit();
            m_mapConnectionINode[fmtbuf.toStdString()] = ino;
        }

        // if it's TCP, we need add reverse mapping due to its bidirectional piping feature,
        // otherwise we wont be able to get the inode
        if (proto == IPPROTO_TCP) {
            fmtbuf = patternB.toLocal8Bit();
            m_mapConnectionINode[fmtbuf.toStdString()] = ino;
            /* workaround: sometimes, when a connection is actually from 172.16.3.1 to
            * 172.16.3.3, packages arrive from 195.169.216.157 to 172.16.3.3, where
            * 172.16.3.1 and 195.169.216.157 are the local addresses of different 
            * interfaces */
            vector<LocalAddr>::iterator itLocalAddr = m_listLocalAddr.begin();
            for (; itLocalAddr != m_listLocalAddr.end(); itLocalAddr++) {
                /* TODO maybe only add the ones with the same sa_family */
                QString patternD = QString("%1:%2-%3:%4").arg(daddr_str).arg(des_port).arg(itLocalAddr->str_ipinfo.c_str()).arg(src_port);
                fmtbuf = patternA.toLocal8Bit();
                m_mapConnectionINode[fmtbuf.toStdString()] = ino;
            }
        }
    }
    fclose(fp);

    return 0;
}

int ProcessNetwork::searchConnINodes(string strConnFile)
{
    if (strConnFile.empty()) {
        return -1;
    }
    FILE * procinfo = fopen (strConnFile.c_str(), "r");

	char buffer[8192] = {0};
	if (procinfo == NULL)
		return -2;
	
	if (fgets(buffer, sizeof(buffer), procinfo)) {
		// do nothing
	}

	do
	{
		if (fgets(buffer, sizeof(buffer), procinfo)) {
            short int sa_family;
            struct in6_addr result_addr_local;
            struct in6_addr result_addr_remote;

            char rem_addr[128] = {0}, local_addr[128] = {0};
            int local_port, rem_port;
            struct in6_addr in6_local;
            struct in6_addr in6_remote;

            unsigned long inode;

            int matches = sscanf(buffer, "%*d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %*X %*X:%*X %*X:%*X %*X %*d %*d %ld %*512s\n",
                local_addr, &local_port, rem_addr, &rem_port, &inode);
            if (matches != 5) {
                continue;
            }
            
            if (inode == 0) {
                /* connection is in TIME_WAIT state. We rely on 
                * the old data still in the table. */
                continue;
            }

            if (strlen(local_addr) > 8)
            {
                /* this is an IPv6-style row */

                /* Demangle what the kernel gives us */
                sscanf(local_addr, "%08X%08X%08X%08X", 
                    &in6_local.s6_addr32[0], &in6_local.s6_addr32[1],
                    &in6_local.s6_addr32[2], &in6_local.s6_addr32[3]);
                sscanf(rem_addr, "%08X%08X%08X%08X",
                    &in6_remote.s6_addr32[0], &in6_remote.s6_addr32[1],
                        &in6_remote.s6_addr32[2], &in6_remote.s6_addr32[3]);

                if ((in6_local.s6_addr32[0] == 0x0) && (in6_local.s6_addr32[1] == 0x0)
                    && (in6_local.s6_addr32[2] == 0xFFFF0000))
                {
                    /* IPv4-compatible address */
                    result_addr_local.s6_addr32[0]  = in6_local.s6_addr32[3];
                    result_addr_remote.s6_addr32[0] = in6_remote.s6_addr32[3];
                    sa_family = AF_INET;
                } else {
                    result_addr_local  = in6_local;
                    result_addr_remote = in6_remote;
                    sa_family = AF_INET6;
                }
            }
            else
            {
                /* this is an IPv4-style row */
                sscanf(local_addr, "%X", (unsigned int *) &result_addr_local);
                sscanf(rem_addr, "%X",   (unsigned int *) &result_addr_remote);
                sa_family = AF_INET;
            }

            char hashkey[HASHKEYSIZE] = {0};
            char local_string[50] = {0};
            char remote_string[50] = {0};
            inet_ntop(sa_family, &result_addr_local,  local_string,  49);
            inet_ntop(sa_family, &result_addr_remote, remote_string, 49);

            snprintf(hashkey, HASHKEYSIZE * sizeof(char), "%s:%d-%s:%d", local_string, local_port, remote_string, rem_port);
            m_mapConnectionINode[hashkey] = inode;

            /* workaround: sometimes, when a connection is actually from 172.16.3.1 to
            * 172.16.3.3, packages arrive from 195.169.216.157 to 172.16.3.3, where
            * 172.16.3.1 and 195.169.216.157 are the local addresses of different 
            * interfaces */
            vector<LocalAddr>::iterator itLocalAddr = m_listLocalAddr.begin();
            for (; itLocalAddr != m_listLocalAddr.end(); itLocalAddr++) {
                /* TODO maybe only add the ones with the same sa_family */
                char hashkey[HASHKEYSIZE] = {0};
                snprintf(hashkey, HASHKEYSIZE * sizeof(char), "%s:%d-%s:%d", itLocalAddr->str_ipinfo.c_str(), local_port, remote_string, rem_port);
                m_mapConnectionINode[hashkey] = inode;
            }
        }
	} while (!feof(procinfo));

	fclose(procinfo);
	return 0;
}

/* moves the pointer right until a non-space is seen */
static char * stripspaces (char * input)
{
	char * retval = input;
	while (*retval == ' ')
	  retval++;
	return retval;
}

static bool is_number (const char * string) {
	while (*string) {
		if (!isdigit (*string))
			return false;
		string++;
	}
	return true;
}

static unsigned long str2ulong (const char * ptr) {
	unsigned long retval = 0;

	while ((*ptr >= '0') && (*ptr <= '9')) {
		retval *= 10;
		retval += *ptr - '0';
		ptr++;
	}
	return retval;
}

static int str2int (const char * ptr) {
	int retval = 0;

	while ((*ptr >= '0') && (*ptr <= '9')) {
		retval *= 10;
		retval += *ptr - '0';
		ptr++;
	}
	return retval;
}

// read file from filehandle and return all content
static std::string read_file (int fd) {
	char buf[256] = {0};
	std::string content;

	for (int length; (length = read(fd, buf, sizeof(buf))) > 0;) {
		if (length < 0) {
			break;
		}
		content.append(buf, length);
	}

	return content;
}

// read file from filepath and return all content
static std::string read_file (const char* filepath) {
	int fd = open(filepath, O_RDONLY);

	if (fd < 0) {
		return "";
	}

	std::string contents = read_file(fd);

	if (close(fd)) {
		return "";
	}

	return contents;
}

static std::string getprogname (pid_t pid) {
	const int maxfilenamelen = 14 + MAX_PID_LENGTH + 1;
	char filename[maxfilenamelen];

	std::snprintf(filename, maxfilenamelen, "/proc/%d/cmdline", pid);
	return read_file(filename);
}

void ProcessNetwork::refreshProcNetInfo()
{
    DIR * proc = opendir ("/proc");
	if (proc == 0) {
		return ;
	}

	dirent * entry;
	while ((entry = readdir(proc))) {
		if (entry->d_type != DT_DIR) continue;

		if (!is_number(entry->d_name)) continue;

		char dirname[10 + MAX_PID_LENGTH] = {0};
        size_t dirlen = 10 + MAX_PID_LENGTH;
        string strPid = entry->d_name;
        snprintf(dirname, dirlen, "/proc/%s/fd", strPid.c_str());
        DIR * dir = opendir(dirname);
        if (!dir)
        {
            continue;
        }

        /* walk through /proc/%s/fd/... */
        dirent * entryFd;
        while ((entryFd = readdir(dir))) {
            if (entryFd->d_type != DT_LNK)
                continue;
            size_t fromlen = 256;
            char fromname[256] = {0};
            string strFd = entryFd->d_name;
            snprintf (fromname, fromlen, "%s/%s", dirname, strFd.c_str());

            int linklen = 80;
            char linkname[linklen] = {0};
            int usedlen = readlink(fromname, linkname, linklen-1);
            if (usedlen == -1)
            {
                continue;
            }
            if (usedlen < linklen) {
                linkname[usedlen] = '\0';
                if (strncmp(linkname, "socket:[", 8) == 0) {
                    unsigned long luINode = str2ulong(linkname + 8);
                    pid_t pid = str2int(strPid.c_str());

                    map<pid_t, ProcessNetInfo>::iterator itProcNetInfo = m_mapProcNetInfo.find(pid);
                    if (itProcNetInfo != m_mapProcNetInfo.end()) {
                        m_mapProcNetInfo[pid].procINodes.push_back(luINode);
                    } else {
                        // read process name and userid
                        ProcessNetInfo processNetInfo;
                        char procdir [100] = {0};
                        snprintf(procdir, 100, "/proc/%d", pid);
                        struct stat stats;
                        int retval = stat(procdir, &stats);
                        if (retval != 0)
                            processNetInfo.procUid = 0;
                        else
                            processNetInfo.procUid = stats.st_uid;
                        processNetInfo.strName = getprogname(pid);
                        processNetInfo.procINodes.push_back(luINode);
                        processNetInfo.lluRecvCount = 0;
                        processNetInfo.lluSendCount = 0;
                        processNetInfo.lastTickCount = QDateTime::currentDateTime().toMSecsSinceEpoch();
                        m_mapProcNetInfo[pid] = processNetInfo;
                    }
                }
            }
        }
        closedir(dir);
	}
	closedir(proc);
}

void ProcessNetwork::checkProcessInfo()
{
    map<pid_t, ProcessNetInfo>::iterator itProcNetInfo = m_mapProcNetInfo.begin();
    quint64 curTickCount = QDateTime::currentDateTime().toMSecsSinceEpoch();
    for (; itProcNetInfo != m_mapProcNetInfo.end(); ) {
        if (curTickCount - itProcNetInfo->second.lastTickCount > PROCESSTIMEOUT*1000) { // remove timeout procnetinfo
            itProcNetInfo->second.procINodes.clear();
            emit procDeleted(itProcNetInfo->first);
            m_mapProcNetInfo.erase(itProcNetInfo++);
        } else {
            emit procDetected(QString::fromStdString(itProcNetInfo->second.strName), 
                itProcNetInfo->second.lluRecvCount, itProcNetInfo->second.lluSendCount, 
                itProcNetInfo->first, itProcNetInfo->second.procUid, QString::fromStdString(itProcNetInfo->second.strDevName));
            itProcNetInfo++;
        }
    }
}

bool ProcessNetwork::isPacketOutgoing(ProcessNetPacket& procNetPacket)
{
    if (procNetPacket.dir != dir_unknown) {
        return procNetPacket.dir == dir_outgoing;
    }
    bool islocal = false;
    if (procNetPacket.sa_family == AF_INET)
        islocal = localAddrContains(procNetPacket.sip.s_addr);
    else
        islocal = localAddrContains(procNetPacket.sip6);
    if (islocal) {
        procNetPacket.dir = dir_outgoing;
        return true;
    } else {
        procNetPacket.dir = dir_incoming;
        return false;
    }
}

string ProcessNetwork::getPacketHashkey(ProcessNetPacket& procNetPacket)
{
    if (!procNetPacket.strHashKey.empty())
        return procNetPacket.strHashKey;
    char strHashKey[256] = {0};
    char local_string[50] = {0};
	char remote_string[50] = {0};
	if (procNetPacket.sa_family == AF_INET) {
		inet_ntop(procNetPacket.sa_family, &procNetPacket.sip, local_string,  49);
		inet_ntop(procNetPacket.sa_family, &procNetPacket.dip, remote_string, 49);
	} else {
		inet_ntop(procNetPacket.sa_family, &procNetPacket.sip6, local_string,  49);
        inet_ntop(procNetPacket.sa_family, &procNetPacket.dip6, remote_string, 49);
	}
	if (isPacketOutgoing(procNetPacket)) {
		snprintf(strHashKey, 256 * sizeof(char), "%s:%d-%s:%d", local_string, procNetPacket.sport, remote_string, procNetPacket.dport);
	} else {
		snprintf(strHashKey, 256 * sizeof(char), "%s:%d-%s:%d", remote_string, procNetPacket.dport, local_string, procNetPacket.sport);
	}
    procNetPacket.strHashKey = strHashKey;
    return procNetPacket.strHashKey;
}

void ProcessNetwork::run()
{
    initNetInfo();
    m_updateProcNetInfoTick = QDateTime::currentDateTime().toMSecsSinceEpoch();
    while (!m_isStoped && !m_mapNetDeviceHandle.empty()) { // main loop
        bool packets_read = false;
        quint64 curTickCount = QDateTime::currentDateTime().toMSecsSinceEpoch();
        if (curTickCount-m_refreshProcNetInfoTick > 1000) {
            m_refreshProcNetInfoTick = curTickCount;
            // fresh connection inodes
            refreshConnINodes();
            // refreshProcNetInfo();
        }

        map<string, dp_handle*>::iterator itNetDevHandle = m_mapNetDeviceHandle.begin();
        while (itNetDevHandle != m_mapNetDeviceHandle.end()) {
            m_tempNetPacket.strDevName = itNetDevHandle->first;
            m_tempNetPacket.sa_family = AF_UNSPEC;
            if (itNetDevHandle->second) {
                int retval = dp_dispatch(itNetDevHandle->second, -1, (u_char *)this, sizeof(ProcessNetwork));
                if (retval < 0)
                {
                    qCritical() << "Error dispatching: " << retval;
                }
                else if (retval != 0)
                {
                    packets_read = true;
                }
            }
            itNetDevHandle++;
        }

        // If no packets were read at all this iteration, pause to prevent 100% CPU utilisation
        if (!packets_read) {
            usleep(1000);
        }
        if (curTickCount-m_updateProcNetInfoTick > 2000) {
            m_updateProcNetInfoTick = curTickCount;
            // check procnetinfo to update
            checkProcessInfo();
        }
    }
    deinitNetInfo();
}

void ProcessNetwork::stop()
{
    m_isStoped = true;
}
