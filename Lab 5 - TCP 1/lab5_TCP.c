#define _GNU_SOURCE
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include<time.h>

#define MAXBUF 20000

void get_request(int sock, const char *host){

    
    char *msg;
    const char * format =
        "GET /%s HTTP/1.1\r\nHost: %s\r\nUser-Agent: lab5.c\r\n\r\n";
    int status;
    status = asprintf (& msg, format, "", host);
    if(status == -1){
        fprintf(stderr, "asprintf failed\n");
        exit(0);
    }
    
    status = send (sock, msg, strlen (msg), 0);
    if(status == -1){
        fprintf(stderr, "Send failed\n");
        exit(0);
    }

    

    //READING EVERYTHING AT ONCE
    char buffer[MAXBUF];
    int numbytes;
    if ((numbytes=recv(sock, buffer, MAXBUF - 1, 0)) == -1) {
        perror("recv");
        shutdown(sock, SHUT_RDWR); 
        close(sock); 
        printf("CLOSED!\n");
        
    }
   
   
    fprintf(stderr, "%s", buffer);
    /*char http_response[50][MAXBUF];
    int idx = 0;
    char cont_len[10];
    char *temp;
    char *word = "Content-Length:";
    char *cont = "<!DOCTYPE html";
    char *token = NULL;
    int fg = 1;
        token = strtok(buffer, "\n");
        while (token) {
            printf("Token[%d]: %s\n", idx, token);
            temp = strstr(token, word);
            if( temp != NULL && fg) {
                temp = temp+strlen(word);
                //fprintf(stderr, "Token[%d]%s\n",idx,token);
                strcpy(cont_len, temp);
                fg = 0;
            }
            temp = strstr(token, cont);
            if(temp != NULL){
                printf("CONT: %s\n", temp);
                break;
            }
            strcpy(http_response[idx], token);
            token = strtok(NULL, "\n");
            idx++;
        }
*/
    bzero(buffer, MAXBUF);//*/

   /* fprintf(stderr, "Status Code : %s\n", http_response[0]);
    int len = atoi(cont_len);
    if( len != 0){
        fprintf(stderr, "\n ------------------CONTENT -----------%s\n", cont);
    }else{
        fprintf(stderr, "Content-Length: %d\n", len );
    }*/

    
    free (msg);
}

int main (int argc, char *argv[])
{
    struct addrinfo hints, *res, *res0;
    clock_t t;
    int error;
    int sock;
    char host[100];
    strcpy(host, argv[1]);
    memset (&hints, 0, sizeof (hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    error = getaddrinfo (host, "http", & hints, & res0);
    if(error != 0){
        if(error == EAI_ADDRFAMILY){
            fprintf (stderr, "The specified network host does not have any network addresses"
              "in the requested address family.\n");
        }
        else if(error == EAI_SERVICE){
            fprintf (stderr, "The requested service  is  not  available  for  the  requested socket type.\n");
        }
        else 
        fprintf (stderr, "getaddrinfo: %s\n", strerror (errno));
    }
    sock = -1;
    t = clock();
    for (res = res0; res; res = res->ai_next) {
        sock = socket (res->ai_family, res->ai_socktype, res->ai_protocol);
        if(sock < 0){
            fprintf (stderr, "Socket: %s\n", strerror (errno));
            exit(0);
        }
        if (connect (sock, res->ai_addr, res->ai_addrlen) < 0) {
            fprintf (stderr, "connect: %s\n", strerror (errno));
            close (sock);
	    // exit ok
            exit (EXIT_FAILURE);
        }
        break;
    }
    printf("CONNECTED....\n");

    if (sock != -1) {
        get_request(sock, host);
    }
    close(sock);

    t = clock() - t;
    double time_taken = ((double)t);
    printf("\nTIME ELAPSED : %lf ms\n", time_taken);
    printf("DISCONNECTED!\n");
    
    freeaddrinfo (res0);
    return 0;
}
