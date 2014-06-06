#include <iostream>
#include <map>
#include <set>
extern "C" {
#ifndef __APP_H
#define __APP_H

#include <Python.h>

#include <pthread.h>

#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <iostream>

#include <string.h>
#include <string>

#include <asm/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <pwd.h>
#include <map>

#include <netinet/in.h>
#include <map>
#include <stdio.h>
#include <stdlib.h>


#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <ctype.h>
#include <cstdlib>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/time.h>
#include <time.h>

#include <pcap.h>

#include "conninode.h"
#include "process.h"



#define HASHKEYSIZE 92



void* wakeUp(void *arg);

void processCallBack(u_char *userData, const  pcap_pkthdr *header, const u_char *packet);

double doProcess(int pid, int time);




class App{
public:
	App(long pid, int time) {
		this->process = new Process(pid);
		this->con = new Connection();
		this->time = time;
		this->pid = pid;
		this->len = 0;
	}

	~App() {
		delete this->process;
		delete this->con;
	}

	void dispatch() {
		char errbuff[PCAP_ERRBUF_SIZE];
		char *dev = pcap_lookupdev(errbuff);
		
		this->handle = pcap_open_live(dev, BUFSIZ, 0, 1000, errbuff);

		this->linkType = pcap_datalink(this->handle);
		int err = pthread_create(&this->ptid, NULL, wakeUp, this);
		pcap_loop(this->handle, -1, processCallBack, (u_char*)this);
	}

	void dp_parse_ethernet (const pcap_pkthdr * header, const u_char * packet){
		const struct ether_header * ethernet = (struct ether_header *)packet;
		u_char * payload = (u_char *) packet + sizeof (struct ether_header);


		/* parse payload */
		switch (ethernet->ether_type)
		{
			case (0x0008):
				this->dp_parse_ip (header, payload);
				break;
			case (0xDD86):
				//dp_parse_ip6 (handle, header, payload);
				break;
			default:
				// TODO: maybe support for other protocols apart from IPv4 and IPv6 
				break;
		}
	}

	void dp_parse_ip (const pcap_pkthdr * header, const u_char * packet){
		const struct ip * ip = (struct ip *) packet;

		this->info.sa_family = AF_INET;
		this->info.ip_src = ip->ip_src;
		this->info.ip_dst = ip->ip_dst;

		u_char * payload = (u_char *) packet + sizeof (struct ip);
		switch (ip->ip_p)
		{
			case (6):
				this->dp_parse_tcp (header, payload);
				break;
			default:
				// TODO: maybe support for non-tcp IP packets
				break;
		}
	}

	void dp_parse_tcp (const pcap_pkthdr * header, const u_char * packet){
		struct tcphdr * tcp = (struct tcphdr *) packet;

		unsigned long inode = this->con->getConnectionInoe(this->info.ip_src, ntohs(tcp->source), this->info.ip_dst, ntohs(tcp->dest));
		if (this->process->hasInode(inode)) {
			this->len += header->len;
		}
	}

	pcap_t *handle;
	int time;
	int linkType;
	dpargs info;
	pthread_t ptid;
	long pid;
	int len;

private:
	Process *process;
	Connection *con;
	
	
};



#endif

}