#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#define MAX_BUF 100
#define MAX_FILES 10

int main(int argc, char *argv[])
{
	char buf[MAX_BUF];
	FILE *files[MAX_FILES];
	int tot_files;
	tot_files = argc-1;
	if (tot_files > 0) {
		for (int i = 0; i < tot_files; i++) {
			FILE *pFile = fopen(argv[i+1], "w");
			if (pFile == NULL){
				fprintf(stderr, "Error in opening file %s", argv[i]);
				for(int k = 0; k < i; k++) fclose(files[k]);
				exit(0);
			}
			files[i] = pFile;
		}
		
	size_t len;	
	while ((len = fread(&buf[0], 1, sizeof(buf), stdin)) > 0) {
		fwrite(&buf[0], 1, len, stdout);
		for (int i = 0; i < tot_files; i++) {
			fwrite(&buf[0], 1, len, files[i]);
			fclose(files[i]);
			}
		}
	}
	
	return 0;
}