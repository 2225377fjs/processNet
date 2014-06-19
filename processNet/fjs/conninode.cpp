#include "conninode.h"




/*
 * parses a /proc/net/tcp-line of the form:
 *     sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode
 *     10: 020310AC:1770 9DD8A9C3:A525 01 00000000:00000000 00:00000000 00000000     0        0 2119 1 c0f4f0c0 206 40 10 3 -1                            
 *     11: 020310AC:0404 936B2ECF:0747 01 00000000:00000000 00:00000000 00000000  1000        0 2109 1 c0f4fc00 368 40 20 2 -1                            
 *
 * and of the form:
 *      2: 0000000000000000FFFF0000020310AC:0016 0000000000000000FFFF00009DD8A9C3:A526 01 00000000:00000000 02:000A7214 00000000     0        0 2525 2 c732eca0 201 40 1 2 -1
 *
 */
void addtoconninode (char * buffer, std::map <std::string, unsigned long>* conninode)
{
	short int sa_family;
    	struct in6_addr result_addr_local;
    	struct in6_addr result_addr_remote;

	char rem_addr[128], local_addr[128];
	int local_port, rem_port;
    	struct in6_addr in6_local;
    	struct in6_addr in6_remote;

	// this leaked memory
	//unsigned long * inode = (unsigned long *) malloc (sizeof(unsigned long));
	unsigned long inode;

	int matches = sscanf(buffer, "%*d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %*X %*X:%*X %*X:%*X %*X %*d %*d %ld %*512s\n",
		local_addr, &local_port, rem_addr, &rem_port, &inode);

	if (matches != 5) {
		fprintf(stderr,"Unexpected buffer: '%s'\n",buffer);
		exit(0);
	}
	
	if (inode == 0) {
		/* connection is in TIME_WAIT state. We rely on 
		 * the old data still in the table. */
		return;
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
			result_addr_local  = *((struct in6_addr*) &(in6_local.s6_addr32[3]));
			result_addr_remote = *((struct in6_addr*) &(in6_remote.s6_addr32[3]));
			sa_family = AF_INET;
		} else {
			/* real IPv6 address */
			//inet_ntop(AF_INET6, &in6_local, addr6, sizeof(addr6));
			//INET6_getsock(addr6, (struct sockaddr *) &localaddr);
			//inet_ntop(AF_INET6, &in6_remote, addr6, sizeof(addr6));
			//INET6_getsock(addr6, (struct sockaddr *) &remaddr);
			//localaddr.sin6_family = AF_INET6;
			//remaddr.sin6_family = AF_INET6;
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

	char * hashkey = (char *) malloc (HASHKEYSIZE * sizeof(char));
	char * local_string = (char*) malloc (50);
	char * remote_string = (char*) malloc (50);
	inet_ntop(sa_family, &result_addr_local,  local_string,  49);
	inet_ntop(sa_family, &result_addr_remote, remote_string, 49);

	snprintf(hashkey, HASHKEYSIZE * sizeof(char), "%s:%d-%s:%d", local_string, local_port, remote_string, rem_port);
	free (local_string);

	//if (DEBUG)
	//	fprintf (stderr, "Hashkey: %s\n", hashkey);

	//std::cout << "Adding to conninode\n" << std::endl;

	(*conninode)[hashkey] = inode;

	/* workaround: sometimes, when a connection is actually from 172.16.3.1 to
	 * 172.16.3.3, packages arrive from 195.169.216.157 to 172.16.3.3, where
	 * 172.16.3.1 and 195.169.216.157 are the local addresses of different 
	 * interfaces */
	free (hashkey);
	free (remote_string);
}

/* opens /proc/net/tcp[6] and adds its contents line by line */
int addprocinfo (const char * filename, std::map <std::string, unsigned long>* conninode) {
	FILE * procinfo = fopen (filename, "r");

	char buffer[8192];

	if (procinfo == NULL)
		return 0;
	
	fgets(buffer, sizeof(buffer), procinfo);

	do
	{
		if (fgets(buffer, sizeof(buffer), procinfo))
			addtoconninode(buffer, conninode);
	} while (!feof(procinfo));

	fclose(procinfo);

	return 1;
}




std::map <std::string, unsigned long>* refreshconninode ()
{
	/* we don't forget old mappings, just overwrite */
	//delete conninode;
	//conninode = new HashTable (256);
	std::map <std::string, unsigned long> *conninode = new std::map <std::string, unsigned long>();
	if (! addprocinfo ("/proc/net/tcp", conninode))
	{
		std::cout << "Error: couldn't open /proc/net/tcp\n";
		exit(0);
	}
	return conninode;
	//addprocinfo ("/proc/net/tcp6");

	//if (DEBUG)
	//	reviewUnknown();
}
