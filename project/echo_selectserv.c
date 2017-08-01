#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 100
void error_handling(char *buf);
int clnt_num = 0; // 클라이언트 수
int clnt[10] = { 0 };

struct login_info
{
	char id[50];
	char pw[50];
	int islogin;
	int logined_descripter;
}list[5];

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	struct timeval timeout;
	fd_set reads, cpy_reads;

	//클라이언트에게 전송할 메세지 버퍼
	char message[100]; 
	char message2[100];
	char message3[100];
	socklen_t adr_sz;
	int fd_max, str_len, fd_num, i, j;
	char buf[BUF_SIZE];
	char temp[BUF_SIZE];

	int clnt_id;
	int id_flag = 0;
	int login_flag = 0;

	char *token = NULL;
	char str[] = "<,>";
	char temp2[BUF_SIZE];
	char temp3[BUF_SIZE];

	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if (bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");
	if (listen(serv_sock, 5) == -1)
		error_handling("listen() error");

	FD_ZERO(&reads);
	FD_SET(serv_sock, &reads);
	fd_max = serv_sock;

	while (1)
	{
		cpy_reads = reads;
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;

		if ((fd_num = select(fd_max + 1, &cpy_reads, 0, 0, &timeout)) == -1)
			break;

		if (fd_num == 0)
			continue;

		for (i = 0; i<fd_max + 1; i++)
		{
			if (FD_ISSET(i, &cpy_reads))
			{
				if (i == serv_sock)     // connection request!
				{
					adr_sz = sizeof(clnt_adr);
					clnt_sock =
						accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
					FD_SET(clnt_sock, &reads);
					if (fd_max<clnt_sock)
						fd_max = clnt_sock;

					for (j = 4; j <= 8; j++) //디스크립터 4부터 클라이언트 소켓이 들어오기 때문에
					{
						if (clnt[j] == 0) { //빈 곳 중에 가장 작은 곳에 클라이언트를 넣기 위해서, j번째 클라이언트가 접속하면 값을 1로 바꿈
							clnt_id = j;
							clnt[j] = 1;
							break;
						}
					}

					clnt_num++;
					printf("New client is connected - Number of total client is %d \n", clnt_num);

					sprintf(message, "Connection Success, Your ID is User%d\n", clnt_id - 3);
					write(clnt_sock, message, strlen(message));

				}
				else    // read message!
				{
					str_len = read(i, buf, BUF_SIZE);

					if (str_len == 0)    // close request!
					{   //클라이언트가 나가면  clnt_num--;
						FD_CLR(i, &reads);
						close(i);
						clnt_num--;
						printf("User%d is disconnected - Number of total client is %d \n", i - 3, clnt_num);
						list[i].islogin = 0;
					}

					else 
					{
						strcpy(temp, buf);
						token = strtok(temp, str);

						if (strcmp(token, "/register") == 0) //첫 토큰이 '/register' 이면 회원가입과정 실행
						{
							token = strtok(NULL, str);
							strcpy(temp2, token); //id토큰

							token = strtok(NULL, str);
							strcpy(temp3, token); //pw토큰

							for (j = 4; j <= 8; j++)
							{
								if (strcmp(list[j].id, temp2) == 0) //저장된 id_list에 회원가입하려는 아이디가 존재하면
								{
									sprintf(message2, "Incorect registration\n");
									printf("Incorrect registration\n");
									write(clnt_sock, message2, strlen(message2));
									id_flag = 1; //회원 가입에 실패하면 flag 1, 성공했으면 0이 될것
									break;
								}
							}

							if (id_flag == 0) // 회원가입 성공 시에
							{
								strcpy(list[i].id, temp2);
								strcpy(list[i].pw, temp3);
								sprintf(message3, "Registration success\n");
								printf("%s is registered\n", list[i].id);
								write(clnt_sock, message3, strlen(message3));
							}
							id_flag = 0;
						}

						else if (strcmp(token, "/login") == 0) //첫 토큰이 '/login' 이면 회원가입과정 실행
						{
							token = strtok(NULL, str);
							strcpy(temp2, token); //id토큰

							token = strtok(NULL, str);
							strcpy(temp3, token); //pw토큰

							for (j = 4; j <= 8; j++)
							{
								if (strcmp(list[j].id, temp2) == 0 && strcmp(list[j].pw, temp3) == 0 && list[j].islogin == 0 ) //목록중에 id/pw일치하는게 있으면
								{
									sprintf(message2, "Login Success\n");
									write(clnt_sock, message2, strlen(message2));
									login_flag = 1; // 성공시에 flag 1, 실패시에 flag 0
									list[j].islogin = 1;
									list[j].logined_descripter = i;
								}
							}

							if (login_flag == 0) {
								sprintf(message3, "Login fail\n");
								write(clnt_sock, message3, strlen(message3));
							}

							login_flag = 0;

						}

						else { // echo!
							write(i, buf, str_len);
						}	
					}
				}
			}
		}
	}
	close(serv_sock);
	return 0;
}

void error_handling(char *buf)
{
	fputs(buf, stderr);
	fputc('\n', stderr);
	exit(1);
}


