#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

void send_msg(int sock);
void *recv_msg(void *arg);
void error_handling(char *message);

#define BUFSIZE 100

int myselect = 1;

struct records
{
	int hand;
	int score;
};
struct records r;
int cnt = 1;
void send_msg(int sock);
void *recv_msg(void *arg);
int my_select();
void error_handling(char *msg);

int oh[3] = {0,0,0};

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t rcv_thread;

	srand(time(NULL));

	if(argc!=3)
	{
		printf("Usage : %s IP PORT\n", argv[0]);
		exit(1);
	}

	sock=socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));

	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error");

	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);

	pthread_join(rcv_thread, NULL);

	close(sock);
	return 0;
}

void send_msg(int sock)
{
	char snd_msg[2];
	int hand = 0;

	hand = my_select();

	r.hand = hand;
	sprintf(snd_msg,"%d",hand);
	write(sock, snd_msg, strlen(snd_msg));
}

void *recv_msg(void *arg)
{
	int sock = *((int*)arg);
	char buffer[BUFSIZE];
	
	while(1)
	{
		memset(buffer, 0, BUFSIZE);
		
		read(sock, buffer, BUFSIZE-1);

		fputs(buffer, stdout);
		fputs("\n",stdout);

		if( !strcmp(buffer,"START") )
		{
			r.score = 1;
			send_msg(sock);
			//r.score = 1;
		}
		else if( !strcmp(buffer,"WIN"))
		{
			r.score = 1;
			send_msg(sock);
			
		}
		else if( !strcmp(buffer,"DRAW"))
		{
			r.score = 2;
			send_msg(sock);

		}
		else if( !strcmp(buffer,"LOSE"))
		{
			r.score = 3;
			send_msg(sock);

		}
		else if( !strcmp(buffer,"END"))
		{
			exit(0);
		}
	}
}

int my_select() 
{

	if(cnt == 1) {
		r.hand = 1; //가위
		cnt++;
	}

	if(oh[2] < 100) {

		if(r.score == 1) { //이겼으면
			myselect = r.hand;
			oh[0]++;
		}
		else if(r.score == 2) {//비겼으면
			myselect = ((r.hand+1)%3);
			oh[1]++;
		}
		else if(r.score == 3) {//졌으면
			myselect = ((r.hand+2)%3);
			oh[2]++;
		}

		if(myselect == 0)
			myselect = myselect +3;
		}

		if(oh[2] >= 100)
			myselect = (rand()%2)+1;

		return myselect;

}

void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputs("\n", stderr);
	exit(1);
}
