#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<signal.h>
#include<sys/wait.h>
#include<sys/sem.h>
#include<netinet/tcp.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<netdb.h>
#include <arpa/inet.h>

#define PORT 9090
#define MAXSZ 100

int main(int argc, char *argv[])
{
 int sockfd;
 char server_ip[20] = "127.0.0.1";
 struct sockaddr_in serverAddress;
 if(argc > 1){
 	strcpy(server_ip, argv[1]);
 }
 int n;
 char msg1[MAXSZ];
 char msg2[MAXSZ];
 sockfd=socket(AF_INET,SOCK_STREAM,0);

 memset(&serverAddress,0,sizeof(serverAddress));
 serverAddress.sin_family=AF_INET;
 serverAddress.sin_addr.s_addr=inet_addr(server_ip);
 serverAddress.sin_port=htons(PORT);

 connect(sockfd,(struct sockaddr *)&serverAddress,sizeof(serverAddress));
 while(1){
  printf("\nSend Message :");
  fgets(msg1,MAXSZ,stdin);
  if(msg1[0]=='#')
   break;

  n=strlen(msg1)+1;
  send(sockfd,msg1,n,0);

  n=recv(sockfd,msg2,MAXSZ,0);

  printf("Message Received:: %s\n",msg2);
 }

 return 0;
}