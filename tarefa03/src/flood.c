/*
 * flood.c - Cliente de flood em UDP
 * 
 * MC823 - Tarefa 03
 * Felipe Augusto da Silva        RA 096993
 * Jesse de Moura Tavano Moretto  RA 081704
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <sys/times.h>
#include <sys/wait.h>

#define SERVERPORT 3490     // the port users will be connecting to
#define MAXDATASIZE 1000    // max number of bytes we can get at once

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in their_addr; // connector's address information
    struct hostent *he;
    char *buffer = (char*)malloc(MAXDATASIZE*sizeof(char));

    if (argc != 3) {
        fprintf(stderr,"usage: %s hostname mode(0-1)\n", argv[0]);
        fprintf(stderr, "mode 0: without connect\nmode 1: using connect\n");
        exit(1);
    }

    if ((he=gethostbyname(argv[1])) == NULL) {  // get the host info
        perror("gethostbyname");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    their_addr.sin_family = AF_INET;                         // host byte order
    their_addr.sin_port = htons(SERVERPORT);                 // short, network byte order
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(their_addr.sin_zero), '\0', 8);                 // zero the rest of the struct
    
    // use sendto and recvfrom
    if(atoi(argv[2]) == 0) {
        while((buffer = fgets(buffer, MAXDATASIZE, stdin)) != NULL) {
          
          if ((sendto(sockfd, buffer, strlen(buffer), 0,
                     (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
              perror("sendto");
              exit(1);
          }
          
        }
        if ((sendto(sockfd, "", 0, 0,
                 (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
          perror("sendto_zero_bytes");
          exit(1);
        }        
    }

    // use send and recv
    else {
        if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
          perror("connect");
          exit(1);
        }

        while((buffer = fgets(buffer, MAXDATASIZE, stdin)) != NULL) {
          fputs(buffer, stdout); 
          if ((send(sockfd, buffer, strlen(buffer), 0)) == -1) {
              perror("send");
              exit(1);
          }
        }
        
        sleep(1);
        
        if ((send(sockfd, "", 0, 0)) == -1) {
          perror("send_zero_bytes");
          exit(1);
        }
        
        // disconnect
        their_addr.sin_family = AF_UNSPEC;
        if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
          perror("connect_unspec");
          exit(1);
        }
    }    
    printf("\n\nFIM!\n\n");
    close(sockfd);
    free(buffer);

    return 0;
}
