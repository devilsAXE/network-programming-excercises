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

#define MAXCLI 100
#define BUF_SIZE 4096
#define KEY 0x1111
#define PORT 9090

int numC = 0;

static void handleRequest(int cfd){
	char buf[BUF_SIZE];
	ssize_t numRead;
	while ((numRead = read(cfd, buf, BUF_SIZE)) > 0) {
		if (write(cfd, buf, numRead) != numRead) {
			fprintf(stderr, "write() failed: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	if (numRead == -1) {
		fprintf(stderr, "Error from read(): %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
}
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short  *array;
};

struct sembuf p = { 0, -1, 0};
struct sembuf v = { 0, 1, 0};
int semid;

static void no_zombies(int sig){

	while(waitpid(-1, NULL, WNOHANG) > 0){

		if(semop(semid, &v, 1) < 0){
			perror("semop V"); exit(14);
        }
        
        if(numC > 0)
        	numC--;
        else numC = 0;

        fprintf(stderr, "Client Disconnected : Total Connection [%d] \n", numC);
		continue;
	}


}

int main(int argc, char *argv[]){
	int listenfd,connfd;
	int num_clients = 2;
	if(argc > 1){
		num_clients = atoi(argv[1]);
	}
	fprintf(stderr, "Num of Clients Allowed : %d\n", num_clients);
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = no_zombies;
	if(sigaction(SIGCHLD, &sa, NULL) == -1){
		fprintf(stderr, "ERROR from sigaction %s\n", strerror(errno));
	}

	struct sockaddr_in serveraddr;
	struct sockaddr_in clientAddr;

	int def = 0;
	if((listenfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
		perror("Socket: ");
	memset(&serveraddr,0,sizeof(serveraddr));

	serveraddr.sin_port = htons(PORT);
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if((bind(listenfd, (struct sockaddr *) & serveraddr , sizeof(serveraddr))) < 0)
		perror("Bind: ");


	listen(listenfd, 5 + num_clients);
	pid_t pid;

	int semid = semget(KEY, 1, 0666 | IPC_CREAT);
	if(semid < 0) {
		perror("semget");
		exit(1);
	}
	union semun arg;
	arg.val = num_clients;
	if(semctl(semid, 0, SETVAL, arg) == -1){
		perror("semctl");
		exit(1);
	}
	int len = sizeof(clientAddr);
	memset(&clientAddr, 0, sizeof(clientAddr));
	while(1){
		printf("\n------ Waiting for new connection:----\n");
		connfd = accept(listenfd, (struct sockaddr *)&clientAddr, &len);
		printf("Connected to : %s : Total connected [%d] \n ",inet_ntoa(clientAddr.sin_addr), ++numC);
		pid = fork();
		if(pid == 0){
			if(semop(semid, &p, 1)  < 0){
				perror("semop");
				exit(EXIT_FAILURE);
			}
			close(listenfd);
			handleRequest(connfd);
			exit(EXIT_SUCCESS);
			
		}
		else if(pid == -1){
			fprintf(stderr, "Child create error %s\n", strerror(errno));
			close(connfd);
			exit(EXIT_FAILURE);
		}
		else{

			close(connfd);

		}

	}


}
