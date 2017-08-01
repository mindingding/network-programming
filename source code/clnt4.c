#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

void send_msg(int sock);
void *recv_msg(void *arg);
void error_handling(char *message);

#define BUFSIZE 100
int myselect = 1;
int cnt=1;
int score=0;

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

		if( !strcmp(buffer,"START") )
		{
			send_msg(sock);
			score = 1;
		}else if( !strcmp(buffer,"WIN"))
		{
			send_msg(sock);
			score = 1;
		}else if( !strcmp(buffer,"DRAW"))
		{
			send_msg(sock);
			score = 2;
		}else if( !strcmp(buffer,"LOSE"))
		{
			send_msg(sock);
			score = 3;
		}else if( !strcmp(buffer,"END"))
		{
			exit(0);
		}
	}
}

int enemy(int myhand, int rscore)
{	
	int enemyhand;
	
	if(rscore ==1)//win
		enemyhand = myhand -1;
	else if(rscore ==2)//draw
		enemyhand = myhand;
	else if(rscore ==3)
		enemyhand = (myhand+1)%3;

	if(enemyhand ==0)
		enemyhand +=3;

	return enemyhand;
}

int algorithm1(int myhand, int rscore)
{

	int m,e; // m = myselect, e = enemyhand;
	
	e = enemy(myhand, rscore);
	m = (e+2)%3;

	if(m==0)
		m +=3;

	return m;
}

int my_select()
{
	printf("%d \n", enemy(myselect, score));

	myselect = algorithm1(myselect, score);

	return myselect;
}


void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputs("\n", stderr);
	exit(1);
}
