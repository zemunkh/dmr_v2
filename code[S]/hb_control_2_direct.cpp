 #include <cstdlib>
#include <iostream>
#include <fstream>
#include <string.h>
#include <dirent.h>
#include <wiringPi.h>
#include <wiringSerial.h>
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
#include <errno.h> //signal function

#define BUFSIZE 100
#define HBSIZE 5
#define HBCHECK 10 //3
#define MSDIR "/home/pi/Desktop/heartbeat/basicmath/basicmath_small"//basicmath_small
#define MSNAME "basicmath_small"//basicmath_small
#define PRIIP "169.254.160.166"

using namespace std;

void* hb_rcv_p(void* arg);
void* failure_check(void* arg);
int check_process();
void error_handling(char* message);
void sig_alarm(int sig);

static const unsigned int TIMEOUT_SECS = 2;
static const unsigned int MAXTRIES = 5;

int CHECKPOINT = 0;
int LOGCOLLECT = 0;
unsigned int tries = 0;
int SEC_F_CHECK = 0;
int hb_state = 1;

int prev_value = 0;
int same_value = 0;
int again_value = 0;

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

	char snd_buff[HBSIZE];
	char rcv_buff[HBSIZE];

	int hb_check = 0;

	int test_cnt = 0;
	int watch_count = 0;
	int watch_delay = 0;
	int digit = 0;
	int temp_i = 0;

	socklen_t clnt_leng;
	int usingPort = 48221;
	bool bValid = 1;

	struct sockaddr_in servaddr;

	sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0)
		error_handling("UDP socket() error");

	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&bValid, sizeof(bValid));

	struct timeval optVal = {0, CHECKPOINT};
	int optLen = sizeof(optVal);
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &optVal, optLen);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(usingPort);
	if(inet_pton(AF_INET,PRIIP,&servaddr.sin_addr) <= 0)
		error_handling("inet_pton() error");

	int timeout_cnt = 0;

	while(1)
	{
		memset(&snd_buff, 0, sizeof(char)*HBSIZE);
		memset(&rcv_buff, 0, sizeof(char)*HBSIZE);

		digit = 0;
		temp_i = CHECKPOINT;
		do{ ++digit; temp_i/=10; }while(temp_i>0);

		sprintf(snd_buff,"%c%03d",(char)(digit+0x40),CHECKPOINT);
		if(sendto(sockfd,snd_buff,HBSIZE,0,(struct sockaddr*)&servaddr, sizeof(servaddr))<0)
			error_handling("sendto() error");

		int n = -1;

		struct sockaddr_in fromaddr;
		socklen_t fromaddrlen = sizeof(fromaddr);

		timeout_cnt = 0;
		while((n=recvfrom(sockfd, rcv_buff, HBSIZE, 0, (struct sockaddr*)&fromaddr, &fromaddrlen))<=0 && timeout_cnt < 10)
		{
			timeout_cnt++;
		}

		if(timeout_cnt >= 10)
		{
				hb_state = 0;
		}
		else
		{
			if(n > 0)
			{
				hb_state = 1;
		
				int in_digit = (int)(rcv_buff[0]-0x40); 
				int Data_value= (int)(rcv_buff[4]-50); //++

				if (same_value == 10){
					same_value++;
					again_value++;
				}
				else if(Data_value == prev_value) same_value++;
				else if(Data_value != prev_value){
					again_value = 0;
					same_value = 0;
					prev_value = Data_value;
				} //++
		
				if(in_digit > 0 && in_digit <= 3)
				{
					char temp_hb[HBSIZE];
					memset(&temp_hb,0,sizeof(char)*HBSIZE);
					sprintf(temp_hb,"0%c%c%c",rcv_buff[1],rcv_buff[2],rcv_buff[3]); //+
					int j=atoi(temp_hb);
					if(j >= 1 && j <= 1000)
					{
						CHECKPOINT = j;

						struct timeval optVal2 = {0, CHECKPOINT};
						optLen = sizeof(optVal2);
						setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &optVal2, optLen);
						FILE *f;//++
						f=fopen("/home/pi/Desktop/heartbeat/basicmath/Data_value.txt","w");
						fprintf(f,"%d",Data_value);
						fclose(f); //++
					}
				}
			}
		}

		delay(CHECKPOINT);
	}

	return 0;
}

void* failure_check(void* arg)
{
	//initializing...
	int hb_state2 = 1;
	int hb_cnt = 0;

	delay(CHECKPOINT*3);

	while(1)
	{
		if(hb_state == 0 && hb_state2 == 1 || same_value >= 10)
		{
			if(again_value == 1) //check_process() == 0
			{
				again_value = 0;
				if(access("/home/Desktop/heartbeat/basicmath/Check_value.txt",F_OK)!=0){//++
				char __buff[BUFSIZE];
				system("date +%H:%M:%S.%3N >> 0220_hb_log_detect.csv");
				sprintf(__buff, "sudo %s &", MSDIR);
				system(__buff);
				cout << __buff << endl;
				hb_state2 = 0;
				hb_cnt = 0;
				system("sudo touch /home/pi/Desktop/heartbeat/basicmath/Check_value.txt"); //++
				}
			}
		}
		else
		{
			if(++hb_cnt >= HBCHECK)
                                hb_state2 = 1;

			if(check_process() != 0 && hb_state == 1)
			{
				char __buff[BUFSIZE];
                                sprintf(__buff, "sudo kill -9 `ps -ef | grep %s | grep -v grep | awk '{print $2}'`", MSNAME);
                                system(__buff);
				//system("sudo rm /home/pi/Desktop/heartbeat/basicmath/Data_value.txt");
			}
		}

		delay(CHECKPOINT);
	}

	return 0;
}

void sig_alarm(int sig)
{
	tries += 1;
}
