#include<sys/types.h>
#include<sys/msg.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<signal.h>
#include<stdio.h>
#include<stddef.h>
#include<limits.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<time.h>

#define MAXPROC 50

int qid;
pid_t pid[MAXPROC];
int numOfProc = 3;

struct mesg{
	long mtype;
	char data[2];
};

static void grimReaper(int sig){

	if (msgctl(qid, IPC_RMID, NULL) == -1)
		perror("msgctl");
	kill(0, SIGKILL);
}

int main(int argc, char *argv[]){
	
	int msg;
	pid_t pidd;
	struct sigaction sa;
	struct mesg mg;
	for(int i = 0; i < numOfProc; i++){
		pid[i] = 0;
	}
	ssize_t msgLen;

	if(argc > 1){
		numOfProc = atoi(argv[1]);
		//printf("%d\n", numOfProc);
	}

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = grimReaper;
	if(sigaction(SIGINT, &sa, NULL) == -1)
		perror("sigaction");


	if ((qid = msgget (420, IPC_CREAT| IPC_EXCL |0666)) == -1) {
		 	perror ("msgget");
		 	exit (1);
		 }

	for(int i = 0; i < numOfProc; i++){
		pidd = fork();
		pid[i] = pidd;
		if(pidd == -1){
			perror("Fork Unsuccessful");
			abort();
		}
		else if(pidd == 0){
			
			while(1){
				time_t t;
				srand((unsigned) time(&t)^getpid());
				msgLen = msgrcv(qid, &mg, sizeof(mg), getpid() , 0);
				if(msgLen == -1)
					perror("msgrcv");
				//printf("Child %d recieved %s from Parent\n", getpid(), mg.data);

				mg.mtype = getppid();
				
				char result[2]; 
	    		int num = rand() % 1000 + 1;
	    		//printf("%d\n", num);
	    		sprintf(result, "%d", num%2); 
				strcpy(mg.data, result);
				if(msgsnd(qid, &(mg.mtype), sizeof(mg), 0) == -1)
					perror("msgsnd");
				}
			exit(0);
		}
		else{
			mg.mtype = pid[i];
			strcpy(mg.data, "A");
			if(msgsnd(qid, &(mg.mtype), sizeof(mg), 0) == -1)
				perror("msgsnd");
		}
	}
	int k = 0;

	while(1){
		int count = 0;
		for(int i = 0; i < numOfProc; i++){
			msgLen = msgrcv(qid, &mg, sizeof(mg), getpid() , 0);
				if(msgLen == -1)
					perror("msgrcv");
				printf("Parent recieved:  %s\n", mg.data);
				count += atoi(mg.data);

		}
		if(count > numOfProc/2){
			printf("---------> AWESOME ACCEPTED \n");
		}
		else{
			printf("---------> NOPE! GO HOME \n");	
		}
		sleep(1);
		for(int i = 0; i < numOfProc; i++){
			mg.mtype = pid[i];
			strcpy(mg.data, "A");
			if(msgsnd(qid, &(mg.mtype), sizeof(mg), 0) == -1)
				perror("msgsnd");
		}
		
	}

	return 0;
}
