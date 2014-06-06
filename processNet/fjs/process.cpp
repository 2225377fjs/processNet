#include "process.h"
extern "C" {


bool is_number (char * string) {
	while (*string) {
		if (!isdigit (*string))
			return false;
		string++;
	}
	return true;
}

unsigned long str2ulong (char * ptr) {
	unsigned long retval = 0;

	while ((*ptr >= '0') && (*ptr <= '9')) {
		retval *= 10;
		retval += *ptr - '0';
		ptr++;
	}
	return retval;
}
int str2int (char * ptr) {
	int retval = 0;

	while ((*ptr >= '0') && (*ptr <= '9')) {
		retval *= 10;
		retval += *ptr - '0';
		ptr++;
	}
	return retval;
}




void reread_mapping () {
	DIR * proc = opendir ("/proc/7140/fd");

	if (proc == 0) {
		std::cerr << "Error reading /proc, neede to get inode-to-pid-maping\n";
		exit(1);
	}

	dirent * entry;

	while ((entry = readdir(proc))) {
		if (entry->d_type != DT_DIR) continue;

		if (! is_number (entry->d_name)) continue;

		//std::cout << "Getting info for " << entry->d_name << std::endl;
		//get_info_for_pid(entry->d_name);
	}
	//std::cout << "End...\n";
	closedir(proc);
}

char * getprogname (char * pid) {
	int filenamelen = 14 + strlen(pid) + 1; 
	int bufsize = 80;
	char buffer [bufsize];
	char * filename = (char *) malloc (filenamelen);
	snprintf (filename, filenamelen, "/proc/%s/cmdline", pid);
	int fd = open(filename, O_RDONLY);
	if (fd < 0) {
		fprintf (stderr, "Error opening %s: %s\n", filename, strerror(errno));
		free (filename);
		exit(3);
		return NULL;
	}
	int length = read (fd, buffer, bufsize);
	if (close (fd)) {
		std::cout << "Error closing file: " << strerror(errno) << std::endl;
		exit(34);
	}
	free (filename);
	if (length < bufsize - 1)
		buffer[length]='\0';

	char * retval = buffer;

	/* this removed directory names, but that malfunctions
	 * when the program name is like "sshd: arnouten@pts/8"
	if ((retval = strrchr(buffer, '/')))
		retval++;
	else 
		retval = buffer; 
	*/
	// truncating is now done where it should be, in cui.cpp

	return strdup(retval);
}



std::set<long>* getProcessSocketInode(long pid) {
	std::set<long>* v = new std::set<long>();
	char temp[50];
	memset(temp, 0, 50);
	sprintf(temp, "%ld", pid);
	size_t dirlen = 10 + strlen(temp);
	char * dirname = (char *) malloc (dirlen * sizeof(char));
	snprintf(dirname, dirlen, "/proc/%s/fd", temp);


	DIR * dir = opendir (dirname);

	dirent * entry;

	while ((entry = readdir(dir))) {
		if (entry->d_type != DT_LNK)
			continue;
		//std::cout << "Looking at: " << entry->d_name << std::endl;

		int fromlen = dirlen + strlen(entry->d_name) + 1;
		char * fromname = (char *) malloc (fromlen * sizeof(char));
		snprintf (fromname, fromlen, "%s/%s", dirname, entry->d_name);

		//std::cout << "Linking from: " << fromname << std::endl;

		int linklen = 80;
		char linkname [linklen];
		int usedlen = readlink(fromname, linkname, linklen-1);
		if (usedlen == -1)
		{
			free (fromname);
			continue;
		}
		
		linkname[usedlen] = '\0';
		//std::cout << "Linking to: " << linkname << std::endl;
		//get_info_by_linkname (pid, linkname);
		if (strncmp(linkname, "socket:[", 8) == 0) {
			
			char * ptr = linkname + 8;
			unsigned long inode = str2ulong(ptr);

			//char * progname = getprogname (temp);
			//free(progname);

			v->insert(inode);
		} else {
			//std::cout << "Linkname looked like: " << linkname << endl;
		}
		free (fromname);
	}
	closedir(dir);
	free(dirname);
	return v;

}



pcap_t *handle;
int linkType;
dpargs info;

Connection *cons;





void dp_parse_tcp (const pcap_pkthdr * header, const u_char * packet)
{
	struct tcphdr * tcp = (struct tcphdr *) packet;

	unsigned long inode = cons->getConnectionInoe(info.ip_src, ntohs(tcp->source), info.ip_dst, ntohs(tcp->dest));
	printf("%d -> len is :  %d\n", inode, header->len);
}

void dp_parse_ip (const pcap_pkthdr * header, const u_char * packet)
{
	const struct ip * ip = (struct ip *) packet;

	info.sa_family = AF_INET;
	info.ip_src = ip->ip_src;
	info.ip_dst = ip->ip_dst;

	u_char * payload = (u_char *) packet + sizeof (struct ip);
	switch (ip->ip_p)
	{
		case (6):
			dp_parse_tcp (header, payload);
			break;
		default:
			// TODO: maybe support for non-tcp IP packets
			break;
	}
}


void dp_parse_ethernet (const pcap_pkthdr * header, const u_char * packet)
{
	const struct ether_header * ethernet = (struct ether_header *)packet;
	u_char * payload = (u_char *) packet + sizeof (struct ether_header);


	/* parse payload */
	switch (ethernet->ether_type)
	{
		case (0x0008):
			dp_parse_ip (header, payload);
			break;
		case (0xDD86):
			//dp_parse_ip6 (handle, header, payload);
			break;
		default:
			// TODO: maybe support for other protocols apart from IPv4 and IPv6 
			break;
	}
}

void pcap_callBack(u_char *userData, const  pcap_pkthdr *header, const u_char *packet) {
	//int len = header->len;
	switch (linkType) {
		case (DLT_EN10MB):
			dp_parse_ethernet (header, packet);
			break;
			/*
		case (DLT_PPP):
			dp_parse_ppp (handle, header, packet);
			break;
		case (DLT_LINUX_SLL):
			dp_parse_linux_cooked (handle, header, packet);
			break;
		case (DLT_RAW):
		case (DLT_NULL):
			// hope for the best
			dp_parse_ip (handle, header, packet);
			break;)*/
		default:
			printf("unknown link type");
			break;
	}
}

void* doIt(void *arg){
	sleep(30);
	pcap_breakloop(handle);
	printf("breaked\n");
	return ((void*)0);
}

}