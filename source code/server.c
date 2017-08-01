
//멀티플렉싱 서버로 구현하였습니다

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <signal.h>

char start[] = "START", win[] = "WIN", lose[] = "LOSE", draw[] = "DRAW", end[] = "END";
int clnt_sock1, clnt_sock2, clnt_msg1 = 0, clnt_msg2 = 0;

void error_handling(char *buf);
int who_win(int a, int b);
void violation (int sig);

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	struct timeval timeout;
	fd_set reads, cpy_reads;
	
	socklen_t adr_sz;
	int fd_max, str_len, fd_num, i, j = 0, clnt_cnt = 0, result = 0;
	double rate1, rate2, total = 0.0, win_cnt1 = 0.0, win_cnt2 = 0.0;
	char buf[2], buf_write[100];

	struct sigaction act;
	act.sa_handler = violation;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGALRM, &act, 0);
	
	if(argc != 3)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");
	if(listen(serv_sock, 5) == -1)
		error_handling("listen() error");

	FD_ZERO(&reads);
	FD_SET(serv_sock, &reads);
	fd_max = serv_sock;
	
	total = atoi(argv[2]);

	//입력시 정해준 횟수만큼 반복합니다.
	while(j < total)
	{
		cpy_reads = reads;
		
		//양쪽 클라이언트의 메세지가 모두 있을 시 가위바위보를 연산합니다.
		if(clnt_msg1 != 0 && clnt_msg2 != 0)
		{
			alarm(0);
			result = who_win(clnt_msg1, clnt_msg2);
			if(result == 0)
			{
				write(clnt_sock1, draw, sizeof(draw));
				write(clnt_sock2, draw, sizeof(draw));
			}
			else if(result == 1)
			{
				win_cnt1++;
				write(clnt_sock1, win, sizeof(win));
				write(clnt_sock2, lose, sizeof(lose));
			}
			else
			{
				win_cnt2++;
				write(clnt_sock1, lose, sizeof(lose));
				write(clnt_sock2, win, sizeof(win));
			}
			
			j++;		
			clnt_msg1 = 0;	//메세지 초기화
			clnt_msg2 = 0;	
		}
		
		if((fd_num = select(fd_max+1, &cpy_reads, 0, 0, NULL)) == -1)
			break;

		for(i = 0; i < fd_max+1; i++)
		{
			if(FD_ISSET(i, &cpy_reads))
			{
				if(i == serv_sock)
				{
					adr_sz = sizeof(clnt_adr);
					
					clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
					FD_SET(clnt_sock, &reads);

					if(fd_max<clnt_sock)
						fd_max = clnt_sock;
					printf("Connected client : %d\n", clnt_sock);
					
					//2개의 클라이언트가 접근하면 "START" 메세지를 전송합니다.
					if(clnt_cnt == 0)
					{
						clnt_sock1 = clnt_sock;
						clnt_cnt++;
					}
					else
					{
						clnt_sock2 = clnt_sock;
						clnt_cnt++;
						write(clnt_sock1, start, sizeof(start));
						write(clnt_sock2, start, sizeof(start));
					}
				}
				else if(i == clnt_sock1)
				{
					//이미 전송받은 메세지가 있을 경우 덮어씌우지 않습니다.
					if(clnt_msg1 == 0)
					{
						alarm(10);
						read(i, buf, 2);
						buf[1] = 0;
						clnt_msg1 = atoi(buf);
			
						//가위, 바위, 보 외에 다른 값이 전송되면 "END" 메세지를 전송하고 종료합니다.
						if(clnt_msg1 != 1 && clnt_msg1 != 2 && clnt_msg1 != 3)
						{
							write(clnt_sock1, end, sizeof(end));
							error_handling("Wrong input");
						}
					}
				}
				else if(i == clnt_sock2)
				{
					if(clnt_msg2 == 0)
					{
						alarm(10);
						read(i, buf, 2);
						buf[1] = 0;
						clnt_msg2 = atoi(buf);
					
						if(clnt_msg2 != 1 && clnt_msg2 != 2 && clnt_msg2 != 3)
						{
							write(clnt_sock2, end, sizeof(end));
							error_handling("Wrong input");
						}
					}
				}
			}
		}
	}
	
	rate1 = (win_cnt1/total)*100.0;
	rate2 = (win_cnt2/total)*100.0;
	printf("Player%d : %.3lf%\n", clnt_sock1, rate1);
	printf("Player%d : %.3lf%\n", clnt_sock2, rate2);
	
	write(clnt_sock1, end, sizeof(end));
	write(clnt_sock2, end, sizeof(end));

	close(clnt_sock1);
	close(clnt_sock2);
	close(serv_sock);
	return 0;
}

void error_handling(char *buf)
{
	fputs(buf, stderr);
	fputc('\n', stderr);
	exit(1);
}

//한쪽 클라이언트의 메세지가 전송되었지만 다른 쪽 클라이언트에서 10초 이상 전송되는 값이 없을 시 몰수패
void violation(int sig)
{
	if(sig == SIGALRM)
	{
		if(clnt_msg1 == 0)
		{
			printf("Player%d violate the rules\n", clnt_sock1);
		}
		else if(clnt_msg2 == 0)
		{
			printf("Player%d violate the rules\n", clnt_sock2);
		}
		write(clnt_sock1, end, sizeof(end));
		write(clnt_sock2, end, sizeof(end));
				
		close(clnt_sock1);
		close(clnt_sock2);
		exit(1);
	}
}

int who_win(int a, int b)
{
	if( a == b ) 
		return 0;
	else if( a % 3 == (b + 1) % 3) 
		return 1;
	else 
		return -1;
}
