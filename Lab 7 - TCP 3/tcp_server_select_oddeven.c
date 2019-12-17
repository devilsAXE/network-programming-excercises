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

#define MAXBUF 1024 
fd_set readfds, fds, wfds;
int max_fd;

void close_connections(int sig_num){
	char sg[2];
	for(int i = 0; i < max_fd + 1; i++){
		close(i);
	}
	exit(0);
}


int main(int argc, char *argv[]){ 

	int listen_fd, conn_fd, nready, optval = 1;
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
	char buff[MAXBUF];
	pid_t child_p;
	
	ssize_t n;
	socklen_t len;
	struct sockaddr_in serv_addr, cli_addr;
	signal(SIGINT, close_connections); 
	/* Creating a Socket */

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(str_port);

	/* Binding serv addr */
	bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	listen(listen_fd, 10);

	FD_ZERO(&readfds);
	max_fd = listen_fd;
	for(;;){
		readfds = fds;
		wfds = fds;
		FD_SET(listen_fd, &readfds);
		FD_SET(listen_fd, &wfds);
		nready = select(max_fd + 1, &readfds, &wfds, NULL, NULL);
		int fd_new;
		for(int fd = 0; fd < (max_fd+1); fd++){
			if(FD_ISSET(fd, &readfds)){
				if(fd == listen_fd){
					len = sizeof(cli_addr);
					fd_new = accept(listen_fd, (struct sockaddr*)&cli_addr, &len);
					FD_SET(fd_new, &fds);
					if(fd_new > max_fd) max_fd = fd_new;
					fprintf(stderr, "\n Connection from client %s at fd [%d] \n ", inet_ntoa(cli_addr.sin_addr), fd_new);
				}
				else if(fd % 2 == 1){
					memset(&buff, 0, sizeof(buff));
					ssize_t numbytes = recv(fd, &buff, sizeof(buff), 0);
					if(numbytes == -1) perror("recv");
					else if(numbytes == 0){
						fprintf(stderr, "\n Socket %d closed by [ODD] client\n", fd);
						close(fd);
						FD_CLR(fd, &fds);
					}else{
						printf("Message From TCP client [%d] <: ", fd); 
                		puts(buff);
                		for(int sub_fd = 0; sub_fd < (max_fd + 1); sub_fd += 2){
                			if(FD_ISSET(sub_fd, &wfds)){
                				fprintf(stderr, "sending to even fd [%d]...\n", sub_fd);
                				write(sub_fd, (const char*)buff, sizeof(buff)); 		
                			}
                			
                		}
                		
					}
				}
				else{
					memset(&buff, 0, sizeof(buff));
					ssize_t numbytes = recv(fd, &buff, sizeof(buff), 0);
					if(numbytes == -1) perror("recv");
					else if(numbytes == 0){
						fprintf(stderr, "\n Socket %d closed by [EVEN] client\n", fd);
						close(fd);
						FD_CLR(fd, &fds);
					}else{
						printf("Message From TCP client [%d] <: ", fd); 
                		puts(buff);
                		for(int sub_fd = 1; sub_fd < (max_fd + 1); sub_fd += 2){
                			if(FD_ISSET(sub_fd, &wfds)){
                				fprintf(stderr, " sending to odd fd [%d]...\n", sub_fd);
                				write(sub_fd, (const char*)buff, sizeof(buff)); 		
                			}
                		}
					}

				}

			}	
		}

	}


exit(EXIT_SUCCESS);



}