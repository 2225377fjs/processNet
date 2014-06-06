
#include "app.h"

extern "C" {



void* wakeUp(void *arg){
	App *app = (App*)arg;
	sleep(app->time);
	pcap_breakloop(app->handle);
	return ((void*)0);
}


void processCallBack(u_char *userData, const  pcap_pkthdr *header, const u_char *packet){
	App *app = (App*)userData;

	switch (app->linkType) {
		case (DLT_EN10MB):
			app->dp_parse_ethernet (header, packet);
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

double doProcess(int pid, int time){


	timeval begin, end;
	gettimeofday(&begin, NULL);

	App app(pid, time);
	app.dispatch();
	int len = app.len;

	gettimeofday(&end, NULL);

	double sec = end.tv_sec - begin.tv_sec;
	sec += ((double)(end.tv_usec - begin.tv_usec)) / 1000000;
	double all = (double)len;

	double su = all / sec / 1000;


	printf("%.2f KB/S\n", su);
	return su;
}


/*
int main(){
	timeval begin, end;
	gettimeofday(&begin, NULL);

	App app(7140, 10);
	app.dispatch();
	int len = app.len;

	gettimeofday(&end, NULL);

	double sec = end.tv_sec - begin.tv_sec;
	sec += ((double)(end.tv_usec - begin.tv_usec)) / 1000000;
	double all = (double)len;

	double su = all / sec / 1000;

	printf("%.2f KB/S\n", su);

	return 0;
	
	Process p(7140);
	if (p.hasInode(23045)) {
		printf("you de \n");
	} else {
		printf("meiyou \n");
	}
	
	cons = new Connection();

	char errbuff[PCAP_ERRBUF_SIZE];
	char *dev = pcap_lookupdev(errbuff);
	handle = pcap_open_live(dev, BUFSIZ, 0, 1000, errbuff);

	linkType = pcap_datalink(handle);
	int err = pthread_create(&ptid, NULL, doIt, NULL);
	pcap_loop(handle, -1, pcap_callBack, NULL);


	delete cons;
	printf("main\n");


	std::map <std::string, unsigned long> *conninode = refreshconninode();
	for (std::map <std::string, unsigned long>::iterator it = conninode->begin(); it != conninode->end(); it++) {
		std::cout<<it->first<<" :  "<<it->second<<"\n";
	}


}*/


}