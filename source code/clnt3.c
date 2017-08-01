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
			send_msg(sock);
			r.score = 1;
		}else if( !strcmp(buffer,"WIN"))
		{
			send_msg(sock);
			r.score = 1;
			
		}else if( !strcmp(buffer,"DRAW"))
		{
			send_msg(sock);
			r.score = 2;

		}else if( !strcmp(buffer,"LOSE"))
		{
			send_msg(sock);
			r.score = 3;

		}else if( !strcmp(buffer,"END"))
		{
			exit(0);
		}
	}
}

int enemy(int myhand, int score)
{//상대방이 전에 낸 것을 알아보는 함수다. 
	int enemyhand;
	//묵 2 찌 1 빠 3	
	if(score ==1) //win
		enemyhand = myhand -1;
	else if(score ==2) //draw
		enemyhand = myhand;
	else if(score ==3) // lose
		enemyhand = (myhand +1)%3;

	if(enemyhand ==0)
		enemyhand = 3;
	
	return enemyhand;
}	


	
int my_select() 
{
	int e;

	srand(time(NULL));
	if(cnt ==1){
		myselect = rand()%3 + 1;
		return myselect;
	}
	cnt++;
	// 상대방이 전에 낸 것을 다시 내지 않는다는 전제하에
	e = enemy(myselect, r.score);
	myselect =(e+2)%3;
	
	if(myselect==0)
		myselect +=3;

	return myselect;

}	


void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputs("\n", stderr);
	exit(1);
}
