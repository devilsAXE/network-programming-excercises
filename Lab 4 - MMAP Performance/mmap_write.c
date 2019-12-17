#include<sys/mman.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<time.h>
#include<fcntl.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>

#define BLOCK_SIZE 1024UL
#define FILE_SIZE 1073741824UL
#define ITERATIONS 268435456UL
#define KB 1024UL
#define MB KB*KB
#define GB MB*KB
#define FILE_GB  ((double) (FILE_SIZE) / (double)(GB))
#define FILE_MB  ((double) (FILE_SIZE) / (double)(MB))
#define FILE_KB  ((double) (FILE_SIZE) / (double)(KB))


int main(int argc, char **argv){
	
	
	int *src , *dst;
	int fdin, fdout;
	struct stat statbuf;
	int fdout2;
	long blocksize;
	int dummy =  1 ;

	//CLA for Blocksize to be used in WRITE part

	if(argc > 1){
		blocksize = atoi(argv[1]);
	}
	else blocksize = BLOCK_SIZE;
	
	clock_t begin = clock();//-------------------------------------------CLOCKING EVENT START--

	if((fdout = open("outfile.txt", O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0){
		printf("Output file create error\n"); return 0;
	}
	if(write(fdout, "", 1) != 1){
		printf("Write error \n"); return 0;
	}

	printf("File size : %lf (GB), %lf (MB),  %lf (KB)\n", FILE_GB, FILE_MB, FILE_KB);
	printf("--------------- MMAP Demo--------------- \n");

	ftruncate(fdout, FILE_SIZE);
	if ((dst = mmap (0, FILE_SIZE, PROT_READ | PROT_WRITE,
	  MAP_SHARED, fdout, 0)) == NULL){
		printf ("mmap error for output");
	   	return 0;
	  }

	for(int i = 0; i < ITERATIONS; i++){
		memcpy (dst ,(int *) &dummy, sizeof (dummy));
		dst++;
		dummy++;
	} 


	clock_t end = clock();//-------------------------------------------CLOCKING EVENT END----
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("Total -> MMAP TIME : %lf\n", time_spent);
	close(fdout);

	
	
	/*-----------------------------------------------------------------*/
	
	printf("\n---------------Write Demo--------------- \n");
	
	clock_t begin2 = clock();
	if(argc > 1){
		blocksize = atoi(argv[1]);
	}
	else blocksize = BLOCK_SIZE;

	printf("BlockSize (KB): %lf\n", (blocksize/(double)KB)  );

	if((fdout2 = open("outfile2.txt", O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0){
		printf("Output file create error\n"); return 0;
	}
	long iter = FILE_SIZE/blocksize;
	long rem = (FILE_SIZE % blocksize);
	int loading = (iter/10);
	printf("Writing in blocks: ");
	for(int i = 0; i < iter; i++){
		if(write(fdout2, dst, blocksize) == -1){
			perror("write\n");
			exit(0);
			//printf("Write error %d\n", errno); return 0;
		}
		//if(!(i % loading)) printf(".");
	}
	printf("->Completed\n");
	if(rem > 0){
		printf("Writing remaining blocks \n");
		blocksize = 1;
		for(int i = 0; i < rem; i++){
			if(write(fdout2, dst, blocksize) == -1){
				perror("write\n");
				exit(0);
				//printf("Write error %d\n", errno); return 0;
			}
		}
	}

	clock_t end2 = clock();
	double time_spent2 = (double)(end2 - begin2) / CLOCKS_PER_SEC;
	printf("TOTAL -> WRITE TIME : %lf\n", time_spent2);
	close(fdout2);
	
 	return 0;

} 

//1.1 GB (1,07,37,41,824 bytes)