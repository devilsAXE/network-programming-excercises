#include <arpa/inet.h> 
#include <errno.h> 
#include <netinet/in.h> 
#include <signal.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <strings.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>
#include <stdbool.h>
#include <sys/select.h>
#include <string.h>
#include <sys/types.h> 
#include <unistd.h>
#include <fcntl.h>

#define MAXLINE 1024 

int main(int argc, char *argv[]){ 
    int sockfd, serverSocket; 
    char buffer[MAXLINE];
    char buffer2[MAXLINE];  
    char* message = "Hello Server"; 
    struct sockaddr_in servaddr; 
    unsigned int str_port;
    if(argc > 1){
        str_port = atoi(argv[1]);
        if(str_port <= 1024){
            fprintf(stderr, "enter valid port number  [1025 - 65535] \n");
            exit(1);
        }
    }
    else{
        fprintf(stderr, "enter valid port number [1025 - 65535]\n");
        exit(1);
    }
    int n, len; 
    // Creating socket file descriptor 
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        printf("socket creation failed"); 
        exit(0); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
  
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(str_port); 
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
  
    if (connect(sockfd, (struct sockaddr*)&servaddr,  
                             sizeof(servaddr)) < 0) { 
        printf("\n Error : Connect Failed \n"); 
    }

    fd_set readfds, fds;
    serverSocket = sockfd;
    FD_SET(sockfd, &readfds);
    memset(buffer, 0, sizeof(buffer)); 
    int sendLen;
    fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
    while(1) {
        fd_set rfds, wfds;
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_SET(sockfd, &rfds);
        FD_SET(sockfd, &wfds);
        FD_SET(0, &rfds);
        int error = 0;
        socklen_t len = sizeof (error);
        ///int retval = getsockopt (socket_fd, SOL_SOCKET, SO_ERROR, &error, &len);
        int state = select(sockfd+1, &rfds, &wfds, NULL, NULL);
        if( state < 0) {
            perror("select");
            exit(-1);
        }
        else if( state == 0){
            fprintf(stderr, "Closing socket\n");
            close(sockfd);
            exit(0);
        }

        if(FD_ISSET(sockfd, &rfds)) {
            bzero(buffer2, MAXLINE);
            n = read(sockfd, buffer2, MAXLINE);
            if(n > 0)
            fprintf(stderr, "Message Received:: %s\n", buffer2);
            else{
                fprintf(stderr, "Closing socket, server down!\n");
                close(sockfd);
                exit(0);
            }
        }

        if(FD_ISSET(sockfd, &wfds)) {
         //printf(">");
         sendLen = 0;
         while(read(0,buffer+sendLen,1)>0){
            sendLen++;
            if(buffer[sendLen - 1] == '\n') break;
         }
         buffer[sendLen] = '\0';
         send(sockfd,buffer,sendLen,0);
         // send the data
        }
    }
    close(sockfd); 
} 