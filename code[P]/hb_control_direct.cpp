#include <cstdlib>
//#include <stdlib.h>
#include <iostream>
#include <fstream>
//#include <string>
#include <string.h>
#include <dirent.h>
#include <wiringPi.h>
#include <wiringSerial.h>
//#include <math.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <vector>
#include <time.h>
#include <signal.h> //signal function
#include <errno.h> //errno function

#define BUFSIZE 100
#define HBSIZE 5
#define HBCHECK 5 //3
#define MSDIR "/home/pi/Desktop/heartbeat/basicmath/basicmath_small"//basicmath_small
#define MSNAME "basicmath_small"//basicmath_small

using namespace std;

void* hb_rcv_p(void* arg);
void* failure_check(void* arg);
int check_process();
void error_handling(char* message);

int CHECKPOINT = 0;
int LOGCOLLECT = 0;
unsigned int tries = 0;
int SEC_F_CHECK = 0;

int main(int argc, char* argv[])
{
	if(argc == 1)
		error_handling("no heartbeat option error");

	if(argc == 2)
		error_handling("no log option error");

	CHECKPOINT = atoi(argv[1]);
	LOGCOLLECT = atoi(argv[2]);

	int fd;

	pthread_t hb_rcv_t, failure_check_t;
	void* thread_result;

	if(wiringPiSetup()==-1)
		error_handling("wiringPiSetup() error");

	pthread_create(&failure_check_t, NULL, failure_check, (void*)fd);

	while(1)
	{
		pthread_create(&hb_rcv_t, NULL, hb_rcv_p, (void*)fd);
		pthread_join(hb_rcv_t, &thread_result);

		sleep(1);
	}

	pthread_join(failure_check_t, &thread_result);

	return 0;
}

int check_process()
{
	DIR* pdir;
	struct dirent* pinfo;
	int is_live = 0;

	pdir = opendir("/proc");
	if(pdir == NULL)
	{
		return 0;
	}

	while(1)
	{
		pinfo = readdir(pdir);
		if(pinfo == NULL)
			break;

		if(pinfo->d_type != 4 || pinfo->d_name[0] == '.' || pinfo->d_name[0] > 57)
			continue;

		FILE* fp;
		char cp_buff[128];
		char cp_path[128];

		sprintf(cp_path, "/proc/%s/status", pinfo->d_name);
		fp = fopen(cp_path, "rt");
		if(fp)
		{
			fgets(cp_buff, 128, fp);
			fclose(fp);

			if(strstr(cp_buff, MSNAME))
			{
				is_live = 1;
				break;
			}
		}
		else
		{

		}
	}

	closedir(pdir);

	return is_live;
}

void error_handling(char* message)
{
	exit(1);
}

void* hb_rcv_p(void* arg)
{
	int fd = (int)arg;
	int sockfd;
 	int Data_value;

	char snd_buff[HBSIZE];
	char rcv_buff[HBSIZE];

	int hb_check = 0;

	int digit = 0;
	int temp_i = 0;

	socklen_t clnt_leng;
	int usingPort = 48221;
	bool bValid = 1;

	struct sockaddr_in servaddr, clntaddr;

	sockfd = socket(PF_INET, SOCK_DGRAM, 0);
        if(sockfd < 0)
                error_handling("UDP socket() error");

        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&bValid, sizeof(bValid));

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(usingPort);

        if(bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
                error_handling("UDP bind() error");


	while(1)
	{
		memset(&snd_buff, 0, sizeof(char)*HBSIZE);
		memset(&rcv_buff, 0, sizeof(char)*HBSIZE);

		int n;

		clnt_leng = sizeof(clntaddr);


		if((n=recvfrom(sockfd,rcv_buff,sizeof(rcv_buff),0,(struct sockaddr*)&clntaddr,&clnt_leng))<=0)
			error_handling("recvfrom() error");


		if(rcv_buff[0]){

                }

		digit = 0;
                temp_i = CHECKPOINT;
                do{ ++digit; temp_i/=10; }while(temp_i>0);

		
		if(access("/home/pi/Desktop/heartbeat/basicmath/Check_value.txt",F_OK)!=0){ //++
                                FILE *f;
                                f=fopen("/home/pi/Desktop/heartbeat/basicmath/Data_value.txt","r");
                                fscanf(f,"%d",&Data_value);
                                fclose(f);
                                system("sudo touch /home/pi/Desktop/heartbeat/basicmath/Check_value.txt"); 
             } //++

	
                sprintf(snd_buff, "%c%03d%d", (char)(digit+0x40), CHECKPOINT,Data_value); //+

		if(sendto(sockfd,snd_buff,HBSIZE,0,(struct sockaddr*)&clntaddr,clnt_leng)<0)
			error_handling("sendto() error");



                delay(CHECKPOINT);
	}

	close(sockfd);

	return 0;
}

void* failure_check(void* arg)
{
	int hb_state = 1;

	delay(CHECKPOINT*3);

	while(1)
	{
		if(++SEC_F_CHECK >= HBCHECK){
			SEC_F_CHECK = 0;
			hb_state = 0;
		}

		if(hb_state == 0)
		{
			if(check_process() == 0)
			{
				char __buff[BUFSIZE];
				system("date +%H:%M:%S.%3N >> 0220_hb_log_detect.csv");
				sprintf(__buff, "sudo %s &", MSDIR);
				system(__buff);
				hb_state = 1;
			}
			else{

			}
		}
		else
		{}

		delay(CHECKPOINT);
	}

	return 0;
}
