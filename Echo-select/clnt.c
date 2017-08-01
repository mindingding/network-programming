/* 
미신청 1조
2000 029 058 컴퓨터학부 김영민
2011 105 108 컴퓨터학부 홍진경
*/

/*
클라이언트의 베이스 소스는 4장의 echo_client.c이며, 대화명 등을 지정하는 방식으로 18장의 chat_clnt.c를 참고함
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>


#define BUF_SIZE 200
#define NAME_SIZE 20 

void error_handling(char*);
void info();

int main(int argc, char *argv[]) 
{
	socklen_t adr_sz;
	struct timeval timeout;
	struct sockaddr_in serv_adr;
	int sock;

	char name_msg[BUF_SIZE+NAME_SIZE];
	//  이름 + 메세지를 위한 버퍼
	char *msg_ptr, *gamebuf;
	// name_msg에서 메세지 부분의 포인터
	int name_len;
	int msg_len;
	int gameflag = 0;

	fd_set read_fds;
  
	if(argc != 4)
	{
		printf("Usage : %s <IP> <PORT> <Name>\n",argv[0]);
		exit(0);
	}

	sprintf(name_msg, "%s : ", argv[3]);
	// name_msg의 앞부분에 이름을 저장
	// "대화명 -> 메세지" 의 형태로 메세지가 전송되게 됨
	name_len = strlen(name_msg);
	msg_ptr = name_msg + name_len;
	//메세지 시작 부분 지정

	sock=socket(PF_INET, SOCK_STREAM, 0);
	if (sock==-1)
		error_handling("Socket error");


	memset(&serv_adr,0,sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));

	if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("Connect error");
	else
		puts("Connected to server\nPress 'q' to exit & 'ready' to play game.\nStart chat.");

	FD_ZERO(&read_fds); // fds 0으로 리셋
  
	while(1) 
	{
		timeout.tv_sec=5;
		timeout.tv_usec=5000;

		FD_SET(0, &read_fds);
		FD_SET(sock, &read_fds);
		// 소켓의 변화를 체크
		
		if(select(sock+1, &read_fds, 0, 0, &timeout) < 0)  
			error_handling("Select Error");
		
		if(FD_ISSET(sock, &read_fds)) 
		{
			if((msg_len = recv(sock, msg_ptr, BUF_SIZE, 0)) > 0) 
			{
				if(!strcmp(msg_ptr, "Finish Game!\n")) { // 서버로 부터 Finish Game을 전송받으면 게임모드에서 빠져나오고
														 // 해당 클라이언트에게도 메세지를 뿌려준다.
					msg_ptr[msg_len] = 0;
					printf("%s", msg_ptr);
					gameflag = 0;
				}
				else {
					msg_ptr[msg_len] = 0;
					printf("%s", msg_ptr);
				}
			}
		}
		// 서버로부터 전송받으면 그대로 출력
    
		if(FD_ISSET(0, &read_fds)) 
		{
			if(fgets(msg_ptr, BUF_SIZE,stdin))
			{
				if(!gameflag)
				{
					if(!strcmp(msg_ptr, "ready\n"))
					{
						if(send(sock, "/1", 2,0) < 0)
							error_handling("Can't write socket");
						gameflag = 1;
					}
					// 입력된 값이 "ready"이면 "대화명 -> 메세지"의 형태를 send 하는게 아니라, 그냥 /1을 보내게됨. 그냥 숫자 1만 보냈을 때 게임모드로 변할수 있으니 /1로 바꿧어요~
				
					else if(!strcmp(msg_ptr, "q\n") || !strcmp(msg_ptr, "Q\n"))
					{
						puts("Disconnect");
						close(sock);
						exit(0);
					}
					// 입력된 값이 q나 Q면 종료
				
					else if(send(sock, name_msg, name_len+strlen(msg_ptr),0) < 0)
						error_handling("Can't write socket");
					// 위의 경우가 아니라면 소켓을 통해 name_msg를 보냄.
					// 서버로 전송할때는 "이름 -> 메세지" 의 형태
				}
				else
				{
					if(!strcmp(msg_ptr, "endgame\n")) // endgame을 입력받으면 게임상태에서 빠져나오고 서버도 game상태에서
													 // 나올수 있도록 /2를 넘겨준다.
					{
						if(send(sock, "/2", 2,0) < 0)
							error_handling("Can't write socket");
						gameflag = 0;
					}

					else if(!strcmp(msg_ptr, "?\n")) // ?를 입력받으면 게임 정보에 대해 알려준다.
					{
						info();
					}
					
					else if(!strcmp(msg_ptr, "q\n") || !strcmp(msg_ptr, "Q\n"))
					{
						puts("Disconnect");
						close(sock);
						exit(0);
					}

					else 
						send(sock, msg_ptr, sizeof(msg_ptr),0);
				}
			}
		}
	}
}

void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
// 교재 동일한 에러 핸들링 함수

void info()
{
	printf("\n----------------------- Digit-baseball Game -----------------------\n");
	printf("1. It's game to match server's 3 digits\n");
	printf("2. If you match some digits with correct position then strike\n");
	printf("3. If you match some digits with wrong position then ball\n");
	printf("4. The one who matches 3 digits fastly is WINNER!\n");
	printf("5. When you want to quit the game, command q \n");
}