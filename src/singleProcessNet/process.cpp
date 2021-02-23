/* 
 * process.cpp
 *
 * Copyright (c) 2004,2005,2008,2011 Arnout Engelen
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

#include <iostream>
#include <strings.h>
#include <string>
#include <asm/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <pwd.h>
#include <map>

#include "process.h"
#include "kylinsystemnethogs.h"
#include "inode2prog.h" 
#include "conninode.h"

extern local_addr * local_addrs;

/* 
 * connection-inode table. takes information from /proc/net/tcp.
 * key contains source ip, source port, destination ip, destination 
 * port in format: '1.2.3.4:5-1.2.3.4:5'
 */
extern std::map <std::string, unsigned long> conninode;


/* this file includes:
 * - calls to inodeproc to get the pid that belongs to that inode
 */

/*
 * Initialise the global process-list with some special processes:
 * * unknown TCP traffic
 * * UDP traffic
 * * unknown IP traffic
 * We must take care these never get removed from the list.
 */
Process * unknowntcp; 
Process * unknownudp; 
Process * unknownip; 
ProcList * processes;

/* We're migrating to having several `unknown' processes that are added as 
 * normal processes, instead of hard-wired unknown processes.
 * This mapping maps from unknown processes descriptions to processes */
std::map <std::string, Process*> unknownprocs;


void process_init () 
{
	unknowntcp = new Process (0, "", "unknown TCP");
	processes = new ProcList (unknowntcp, NULL);
}

void process_delete()
{
	prg_cache_clear();
}

int Process::getLastPacket()
{
	int lastpacket=0;
	ConnList * curconn=connections;
	while (curconn != NULL)
	{
		assert (curconn != NULL);
		assert (curconn->getVal() != NULL);
		if (curconn->getVal()->getLastPacket() > lastpacket)
			lastpacket = curconn->getVal()->getLastPacket();
		curconn = curconn->getNext();
	}
	return lastpacket;
}

Process * findProcess (struct prg_node * node)  //final
{
	ProcList * current = processes;
	while (current != NULL)
	{
		Process * currentproc = current->getVal();
		assert (currentproc != NULL);
		
		if (node->pid == currentproc->pid)
			return current->getVal();
		current = current->next;
	}
	return NULL;
}

/* finds process based on inode, if any */
/* should be done quickly after arrival of the packet, 
 * otherwise findPID will be outdated */
Process * findProcess (unsigned long inode)    // init
{
	struct prg_node * node = findPID(inode);

	if (node == NULL)
		return NULL;

	return findProcess (node);
}

int ProcList::size ()
{
	int i=1;

	if (next != NULL)
		i += next->size();

	return i;
}

void check_all_procs ()
{
	ProcList * curproc = processes;
	while (curproc != NULL)
	{
		curproc->getVal()->check();
		curproc = curproc->getNext();
	}
}

/* 
 * returns the process from proclist with matching pid
 * if the inode is not associated with any PID, return NULL
 * if the process is not yet in the proclist, add it
 */
Process * getProcess (unsigned long inode, const char * devicename)
{
	struct prg_node * node = findPID(inode);
	
	if (node == NULL)
	{
        if (DEBUG)
			std::cout << "No PID information for inode " << inode << std::endl;
		return NULL;
	}

	Process * proc = findProcess (node);

	if (proc != NULL)
		return proc;

	Process * newproc = new Process (inode, devicename, node->name.c_str());

	newproc->pid = node->pid;

	char procdir [100];
	sprintf(procdir , "/proc/%d", node->pid);
	struct stat stats;
	int retval = stat(procdir, &stats);

	if (retval != 0)
		newproc->setUid(0);
	else
		newproc->setUid(stats.st_uid);

	processes = new ProcList (newproc, processes);
	return newproc;
}

/* 
 * Used when a new connection is encountered. Finds corresponding
 * process and adds the connection. If the connection  doesn't belong
 * to any known process, the process list is updated and a new process
 * is made. If no process can be found even then, it's added to the 
 * 'unknown' process.
 */
Process * getProcess (Connection * connection, const char * devicename)
{
	unsigned long inode = conninode[connection->refpacket->gethashstring()];

	if (inode == 0)
	{
		// no? refresh and check conn/inode table
		// refresh the inode->pid table first. Presumably processing the renewed connection->inode table 
		// is slow, making this worthwhile.
		// We take the fact for granted that we might already know the inode->pid (unlikely anyway if we 
		// haven't seen the connection->inode yet though).
		reread_mapping();
		refreshconninode();
		inode = conninode[connection->refpacket->gethashstring()];

#if REVERSEHACK
		if (inode == 0)
		{
			/* HACK: the following is a hack for cases where the 
			 * 'local' addresses aren't properly recognised, as is 
			 * currently the case for IPv6 */

		 	/* we reverse the direction of the stream if 
			 * successful. */
			Packet * reversepacket = connection->refpacket->newInverted();
			inode = conninode[reversepacket->gethashstring()];

			if (inode == 0)
			{
				delete reversepacket;
				if (bughuntmode || DEBUG)
					std::cout << "LOC: " << connection->refpacket->gethashstring() << " STILL not in connection-to-inode table - adding to the unknown process\n";
				unknowntcp->connections = new ConnList (connection, unknowntcp->connections);
				return unknowntcp;
			}

			delete connection->refpacket;
			connection->refpacket = reversepacket;
		}
#endif
	}

	Process * proc = NULL;
	if (inode != 0)
		proc = getProcess(inode, devicename);

	if (proc == NULL) {
		proc = new Process (inode, "", connection->refpacket->gethashstring());
		processes = new ProcList (proc, processes);
	}

	proc->connections = new ConnList (connection, proc->connections);
	return proc;
}
