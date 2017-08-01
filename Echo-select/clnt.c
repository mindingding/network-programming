/* 
�̽�û 1��
2000 029 058 ��ǻ���к� �迵��
2011 105 108 ��ǻ���к� ȫ����
*/

/*
Ŭ���̾�Ʈ�� ���̽� �ҽ��� 4���� echo_client.c�̸�, ��ȭ�� ���� �����ϴ� ������� 18���� chat_clnt.c�� ������
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
	//  �̸� + �޼����� ���� ����
	char *msg_ptr, *gamebuf;
	// name_msg���� �޼��� �κ��� ������
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
	// name_msg�� �պκп� �̸��� ����
	// "��ȭ�� -> �޼���" �� ���·� �޼����� ���۵ǰ� ��
	name_len = strlen(name_msg);
	msg_ptr = name_msg + name_len;
	//�޼��� ���� �κ� ����

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

	FD_ZERO(&read_fds); // fds 0���� ����
  
	while(1) 
	{
		timeout.tv_sec=5;
		timeout.tv_usec=5000;

		FD_SET(0, &read_fds);
		FD_SET(sock, &read_fds);
		// ������ ��ȭ�� üũ
		
		if(select(sock+1, &read_fds, 0, 0, &timeout) < 0)  
			error_handling("Select Error");
		
		if(FD_ISSET(sock, &read_fds)) 
		{
			if((msg_len = recv(sock, msg_ptr, BUF_SIZE, 0)) > 0) 
			{
				if(!strcmp(msg_ptr, "Finish Game!\n")) { // ������ ���� Finish Game�� ���۹����� ���Ӹ�忡�� ����������
														 // �ش� Ŭ���̾�Ʈ���Ե� �޼����� �ѷ��ش�.
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
		// �����κ��� ���۹����� �״�� ���
    
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
					// �Էµ� ���� "ready"�̸� "��ȭ�� -> �޼���"�� ���¸� send �ϴ°� �ƴ϶�, �׳� /1�� �����Ե�. �׳� ���� 1�� ������ �� ���Ӹ��� ���Ҽ� ������ /1�� �مf���~
				
					else if(!strcmp(msg_ptr, "q\n") || !strcmp(msg_ptr, "Q\n"))
					{
						puts("Disconnect");
						close(sock);
						exit(0);
					}
					// �Էµ� ���� q�� Q�� ����
				
					else if(send(sock, name_msg, name_len+strlen(msg_ptr),0) < 0)
						error_handling("Can't write socket");
					// ���� ��찡 �ƴ϶�� ������ ���� name_msg�� ����.
					// ������ �����Ҷ��� "�̸� -> �޼���" �� ����
				}
				else
				{
					if(!strcmp(msg_ptr, "endgame\n")) // endgame�� �Է¹����� ���ӻ��¿��� ���������� ������ game���¿���
													 // ���ü� �ֵ��� /2�� �Ѱ��ش�.
					{
						if(send(sock, "/2", 2,0) < 0)
							error_handling("Can't write socket");
						gameflag = 0;
					}

					else if(!strcmp(msg_ptr, "?\n")) // ?�� �Է¹����� ���� ������ ���� �˷��ش�.
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
// ���� ������ ���� �ڵ鸵 �Լ�

void info()
{
	printf("\n----------------------- Digit-baseball Game -----------------------\n");
	printf("1. It's game to match server's 3 digits\n");
	printf("2. If you match some digits with correct position then strike\n");
	printf("3. If you match some digits with wrong position then ball\n");
	printf("4. The one who matches 3 digits fastly is WINNER!\n");
	printf("5. When you want to quit the game, command q \n");
}