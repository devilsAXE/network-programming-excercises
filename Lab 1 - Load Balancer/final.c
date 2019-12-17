#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include <errno.h>
#include<string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include<stdatomic.h>
#define MAX_PROC 100
#define MAX_LINES 500


void sigdoWork();
void sigusr2(int, siginfo_t*, void*);

int count = 0;
long offset[MAX_LINES];


int print_lines(const char *filename, long off){
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	if ((fp = fopen(filename, "r")) == NULL)
		return 1;

	if (fseek(fp, off, 0) != 0) {
		fclose(fp);
		return 1;
	}
	read = getline(&line, &len, fp);
	//write(1, line, strlen(line));
	printf("\n%s", line);
	if(line)
		free(line);
	fflush(stdout);	
	fclose(fp);
}
void setProcStatus(long proid, int sta){
	char fileid[20];
	sprintf(fileid, "%ld", proid);
	strcat(fileid, ".status");
	FILE *statusPro = fopen(fileid, "w");
	
	fprintf(statusPro, "%d", sta);
			//write (statusPro, pName, strlen(pName));
	fclose(statusPro);  
}
void preProcess(char *filename){
	long read;
	char *line = NULL;
	int len = 0;
	int linecount=1;
	FILE *fp = fopen(filename, "r");
	int offsetNum = 0;
	while ((read = getline(&line, &len, fp)) != -1){
		offset[count] = offsetNum;
		//printf("Line: %d -> %s ------> %ld -----> %d \totalProc", count, line, offset[count], read);
		offsetNum = offset[count] + read;
		count++;
	}
	fclose(fp);
	if(line) free(line);
	//exit(0);
	// for(int i = 0; i < count; i++){
	// 	print_lines("lorem.txt", offset[i]);
	// }
	//exit(0);
}
int getProcStatus(int proid){
	char line[1024] ;
	int state;
	char fileid[20];
	sprintf(fileid, "%d", proid);
	strcat(fileid, ".status");
	FILE *getstate = fopen(fileid, "r");
	fscanf(getstate,"%d", &state);
	fclose(getstate);
	return state;
}


int child_localid = 0;
int ready[MAX_PROC];
int linenum = 0;
int fds[MAX_PROC][2];
int pids[MAX_PROC];
int totalProc = 5;
int countSig = 0;
char *fileToRead = "lorem.txt";
int main(int argc, char *argv[]){
	//printf("%d", argc);'

	if(argc == 1){
		printf("Running default setup Processes : %d and FileName %s \n",
			totalProc, fileToRead);
	}
	else{
		totalProc = atoi(argv[1]);
		if(argc > 2){
			fileToRead = argv[2]; 
		}		
		printf("Running default setup Processes : %d and FileName %s\n",
			totalProc, fileToRead);
	}	
	int parent = totalProc;
	pid_t curr_pid;
	preProcess(fileToRead);
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = sigusr2;
	if (sigaction(SIGUSR2, &sa, 0) == -1) {
		fprintf(stderr, "%s: %s\n", "sigaction", strerror(errno));
	}
	

	for(int i = 0; i < totalProc; i++){
		if(pipe(fds[i]) == -1){
			fprintf(stderr, "Pipe Failed" ); 
			return 1; 
		}
	}
	
	for(int i = 0; i < totalProc; i++){

		curr_pid = fork();
		if(curr_pid < 0){
			perror("Fork Unsuccessful");
			abort();
		}
		else if(curr_pid == 0){
			signal(SIGUSR1, sigdoWork);
			while(1){
				pause();
			}
			exit(0);
		}
		else{
			setProcStatus(curr_pid, 1);
			pids[i] = curr_pid;
			ready[i] = 1;
			child_localid++;
		}	
	}
	//printf("GET STATUS , %d\totalProc", getProcStatus(pids[0]));
	int staus;
	pid_t pid;
	int m = totalProc;	
	if(parent > 0){
		
		while(linenum < count){
			int i = 0;
			while(i < totalProc && linenum < count){    
				//printf(" Child Id %d : ready %d\totalProc", i, ready[i]);   
				if(ready[i]){
					ready[i] = 0;
					//linenum %= m;`
					setProcStatus(pids[i], 0);
					write(fds[i][1], &linenum, sizeof(int));
					kill(pids[i], SIGUSR1);
					linenum++;
				}
				i++;
						
			}


		}
		
		while ((pid = wait(&staus)) > 0);
	}
	
	return 0;
}



void sigdoWork(){
//	cont();
	int read_data = -1;
	read(fds[child_localid][0], &read_data, sizeof(read_data));
	char strr[] = "Line: %d by :%d \t \n";
	char str2[100];
   // sprintf(str2, strr, read_data, getpid());
	printf("%d : line read by : %d \t",read_data,getpid());
   // write(1, str2, strlen(str2));
	print_lines(fileToRead, offset[read_data]);
	fflush(stdout);	
	sleep(1);
	setProcStatus(getpid(), 1);
	kill(getppid(), SIGUSR2);
		
	//sleep(1);



}
void sigusr2(int signo, siginfo_t *si, void *data) {
	(void)signo;
	(void)data;
	int sid = si->si_pid;
	countSig++;
	//printf("%d\n", countSig);
	for(int i = 0; i < totalProc; i++){
		if(pids[i] == sid || getProcStatus(pids[i]) == 1){
			ready[i] = 1;
		}
	}
  //linenum++;

}