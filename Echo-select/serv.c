#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_SIZE 100
#define MAX_CLIENTS 10	

int serv_sock, clnt_sock;
int clnt_cnt = 0;
int clnt_socks[MAX_CLIENTS];

void addClient(int s, struct sockaddr_in *newclnt_adr);
void removeClient(int s);
int getmax();
void error_handling(char *msg);

int main(int argc, char *argv[]) 
{
	struct sockaddr_in clnt_adr,serv_adr;
	struct timeval timeout;

	socklen_t adr_sz;
	int fd_num,i,j,msg_len, option = 1;	

	
	char buf[BUF_SIZE+1];
	fd_set reads, cpy_reads;  
	if(argc != 2) 
	{
		printf("Usage : %s <port>\n",argv[0]);
		exit(0);
	}

	
	serv_sock=socket(AF_INET, SOCK_STREAM, 0);

	if(setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&option, sizeof(option))!=0)
		error_handling("setsocket() error");

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));


	if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind error");
	else puts("Binding complete.");
	
	if(listen(serv_sock, 5)==-1)
		error_handling("listen error");
	else puts("Listening complete.");
		
	while(1) 
	{
		cpy_reads=reads;
		FD_ZERO(&cpy_reads);			
		FD_SET(serv_sock, &cpy_reads);
				
		timeout.tv_sec=5;
		timeout.tv_usec=5000;		   

		for(i=0; i<clnt_cnt; i++)
			FD_SET(clnt_socks[i], &cpy_reads);
		   
		if((fd_num=select(getmax() + 1, &cpy_reads, 0, 0, &timeout)) == -1)
			break;
		if(fd_num==0)
			continue;		
    

		if(FD_ISSET(serv_sock, &cpy_reads)) 
		{
			clnt_sock=accept(serv_sock, (struct sockaddr *)&clnt_adr, &adr_sz);
			addClient(clnt_sock, &clnt_adr); 	
		}


		for(i=0; i<clnt_cnt; i++) //연결된 클라이언트의 수 만큼 반복문
		{
			if(FD_ISSET(clnt_socks[i], &cpy_reads)) // 디스크립터 정보가 있으면
			{				
				if((msg_len = recv(clnt_socks[i], buf,BUF_SIZE, 0))<=0)
				// recv함수의 응답이 없으면 (음수반환) 해당 클라이언트를 clnt_socks배열에서 제외
				{
					removeClient(i); // 클라이언트 제거 함수 호출
					continue;
				}
				buf[msg_len] = 0;

				for(j=0; j<clnt_cnt; j++) 
				{
					if(i!=j)
						send(clnt_socks[j], buf, msg_len, 0);
				}
			}
		}
	}
	return 0;
}

void addClient(int s, struct sockaddr_in *newclnt_adr)
{
	char buf[20];
	inet_ntop(AF_INET, &newclnt_adr->sin_addr,buf, sizeof(buf));
	printf("new client : %s\n", buf);
	
	clnt_socks[clnt_cnt] = s;
	clnt_cnt++;
}

void removeClient(int i)
{
	close(clnt_socks[i]);
	if(i != clnt_cnt-1) clnt_socks[i] = clnt_socks[clnt_cnt-1];
	clnt_cnt--;
}

int getmax()
{
	int max = serv_sock;
	int i;
	for(i=0; i<clnt_cnt; i++)
		if(clnt_socks[i] > max)
			max = clnt_socks[i];
	return max;
}

void error_handling(char *msg)
{
	fputs(msg,stderr);
	fputc('\n',stderr);
	exit(1);
}