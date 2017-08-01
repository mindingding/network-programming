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

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t rcv_thread;

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

	sprintf(snd_msg,"%d",my_select());
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

		//이 밑으로는 자유롭게 코딩하면 됩니다.

		if( !strcmp(buffer,"START") )
		{
			send_msg(sock);
		}else if( !strcmp(buffer,"WIN"))
		{
			send_msg(sock);
		}else if( !strcmp(buffer,"DRAW"))
		{
			send_msg(sock);
		}else if( !strcmp(buffer,"LOSE"))
		{
			send_msg(sock);
		}else if( !strcmp(buffer,"END"))
		{
			exit(0);
		}
	}
}

int my_select()
{
	int myselect;
	srand((unsigned)time(NULL));

	myselect = (rand()%3)+1;

	return myselect;
}


void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputs("\n", stderr);
	exit(1);
}
