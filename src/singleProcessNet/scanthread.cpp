/*
 * scanthread.cpp
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


#include "scanthread.h"
#include <QtDebug>
#include "process.h"
#include "devices.h"
#include "kylinsystemnethogs.h"

extern Process * unknownudp;
timeval curtime;

int process_tcp (u_char * userdata, const dp_header * header, const u_char * m_packet) {
    struct dpargs * args = (struct dpargs *) userdata;
    struct tcphdr * tcp = (struct tcphdr *) m_packet;

    curtime = header->ts;

    /* get info from userdata, then call getPacket */
    Packet * packet=NULL;
    switch (args->sa_family)
    {
    case (AF_INET):
        packet = new Packet (args->ip_src, ntohs(tcp->source), args->ip_dst, ntohs(tcp->dest), header->len, header->ts);
        break;
    case (AF_INET6):
        packet = new Packet (args->ip6_src, ntohs(tcp->source), args->ip6_dst, ntohs(tcp->dest), header->len, header->ts);
        break;
    }

    Connection * connection = findConnection(packet);

    if (connection != NULL)
    {
        connection->add(packet);
    } else {
        connection = new Connection (packet);
        getProcess(connection, args->device);
    }
    delete packet;

    return true;
}

int process_udp (u_char * userdata, const dp_header * header, const u_char * m_packet) {
    struct dpargs * args = (struct dpargs *) userdata;
    struct udphdr * udp = (struct udphdr *) m_packet;

    curtime = header->ts;

    Packet * packet=NULL;
    switch (args->sa_family)
    {
    case (AF_INET):
        packet = new Packet (args->ip_src, ntohs(udp->source), args->ip_dst, ntohs(udp->dest), header->len, header->ts);
        break;
    case (AF_INET6):
        packet = new Packet (args->ip6_src, ntohs(udp->source), args->ip6_dst, ntohs(udp->dest), header->len, header->ts);
        break;
    }

    Connection * connection = findConnection(packet);

    if (connection != NULL)
    {
        connection->add(packet);
    } else {
        connection = new Connection (packet);
        getProcess(connection, args->device);
    }
    delete packet;
    return true;
}

int process_ip (u_char * userdata, const dp_header * /* header */, const u_char * m_packet) {
    struct dpargs * args = (struct dpargs *) userdata;
    struct ip * ip = (struct ip *) m_packet;
    args->sa_family = AF_INET;
    args->ip_src = ip->ip_src;
    args->ip_dst = ip->ip_dst;

    return false;
}

int process_ip6 (u_char * userdata, const dp_header * /* header */, const u_char * m_packet) {
    struct dpargs * args = (struct dpargs *) userdata;
    const struct ip6_hdr * ip6 = (struct ip6_hdr *) m_packet;

    args->sa_family = AF_INET6;
    args->ip6_src = ip6->ip6_src;
    args->ip6_dst = ip6->ip6_dst;

    return false;
}


ScanThread::ScanThread(QObject *parent) : QThread(parent)
{

}

void ScanThread::run()
{
    process_init();
    device * devices = NULL;
    int promisc = 0;

    devices = get_default_devices();
    if ( devices == NULL )
    {
        qCritical("Not devices to monitor");
        return;
    }

    char errbuf[PCAP_ERRBUF_SIZE];

    handle * handles = NULL;
    device * current_dev = devices;
    while (current_dev != NULL) {
        getLocal(current_dev->name);

        dp_handle * newhandle = dp_open_live(current_dev->name, BUFSIZ, promisc, 100, errbuf);
        if (newhandle != NULL)
        {
            dp_addcb (newhandle, dp_packet_ip, process_ip);
            dp_addcb (newhandle, dp_packet_ip6, process_ip6);
            dp_addcb (newhandle, dp_packet_tcp, process_tcp);
            dp_addcb (newhandle, dp_packet_udp, process_udp);

            if (dp_setnonblock (newhandle, 1, errbuf) == -1)
            {
                qCritical() <<  "Error putting libpcap in nonblocking mode";
            }
            handles = new handle (newhandle, current_dev->name, handles);
        }
        else
        {
            qCritical() << "Error opening handler for device " << current_dev->name;
        }

        current_dev = current_dev->next;
    }

    struct dpargs * userdata = (dpargs *) malloc (sizeof (struct dpargs));

    while (1) // main loop
    {
        bool packets_read = false;

        handle * current_handle = handles;
        while (current_handle != NULL)
        {
            userdata->device = current_handle->devicename;
            userdata->sa_family = AF_UNSPEC;
            int retval = dp_dispatch (current_handle->content, -1, (u_char *)userdata, sizeof (struct dpargs));
            if (retval < 0)
            {
                qCritical() << "Error dispatching: " << retval;
            }
            else if (retval != 0)
            {
                packets_read = true;
            }
            current_handle = current_handle->next;
        }

        // If no packets were read at all this iteration, pause to prevent 100% CPU utilisation
        if (!packets_read) {
            usleep(100);
        }
    }
}
