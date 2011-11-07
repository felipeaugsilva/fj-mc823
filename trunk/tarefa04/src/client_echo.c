/*
 * client_echo.c - Cliente de eco em TCP usando E/S (I/O) multiplexada
 * 
 * MC823 - Tarefa 04
 * Felipe Augusto da Silva        RA 096993
 * Jesse de Moura Tavano Moretto  RA 081704
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/times.h>
#include <time.h>
#include <sys/wait.h>

#define PORT 3490          /* the port client will be connecting to */
#define MAXDATASIZE 1000   /* max number of bytes we can get at once */
#define TIMEOUT 5


int main(int argc, char *argv[])
{
    int sockfd;
    int sentLines, recLines, sentBytes, recBytes, longestLine, lineSize;
    struct hostent *he;
    struct sockaddr_in their_addr;   /* connector's address information */
    clock_t startTime, endTime;
    float elapsedTime;
    char *rbuffer = (char*)malloc(MAXDATASIZE*sizeof(char));
    char *wbuffer = (char*)malloc(MAXDATASIZE*sizeof(char));
    FILE *rsock, *wsock;
    int sentAll = 0, sent = 1;
    struct timeval tv;
    fd_set readfds, writefds;

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }
    
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);

    if ((he=gethostbyname(argv[1])) == NULL) {  /* get the host info */
        perror("gethostbyname");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    their_addr.sin_family = AF_INET;         /* host byte order */
    their_addr.sin_port = htons(PORT);       /* short, network byte order */
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    bzero(&(their_addr.sin_zero), 8);        /* zero the rest of the struct */

    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
        perror("connect");
        exit(1);
    }

    if ((rsock = fdopen(sockfd, "r")) == NULL) {
        perror("fdopen_rsock");
        exit(1);
    }

    if ((wsock = fdopen(sockfd, "w")) == NULL) {
        perror("fdopen_wsock");
        exit(1);
    }

    sentLines = recLines = sentBytes = recBytes = longestLine = lineSize = 0;
    
    startTime = times(NULL);   /* start time counting */

    setvbuf(wsock, NULL, _IOLBF, 0);
    setvbuf(rsock, NULL, _IOLBF, 0);
    
    while (1) {
        
        tv.tv_sec = TIMEOUT;
        tv.tv_usec = 0;
        FD_SET(sockfd, &readfds);
        
        if (sent && fgets(wbuffer, MAXDATASIZE, stdin) != NULL) {
            lineSize = strlen(wbuffer);
            sentLines += 1;
            sentBytes += lineSize;
            if (lineSize > longestLine)
                longestLine = lineSize;
            
            FD_SET(sockfd, &writefds);
            
            if (select(sockfd+1, &readfds, &writefds, NULL, &tv) < 0) {
                perror("select");
                exit(1);
            }
            sent = 0;

        } else if (sent) {
            shutdown(sockfd, SHUT_WR);
            sentAll = 1;
            if (select(sockfd+1, &readfds, NULL, NULL, &tv) < 0) {
                perror("select");
                exit(1);
            }
        } else {
            FD_SET(sockfd, &writefds);
            if (select(sockfd+1, &readfds, &writefds, NULL, &tv) < 0) {
                perror("select");
                exit(1);
            }
        }
        
        if (FD_ISSET(sockfd, &writefds) && ! sentAll) {
            setvbuf(wsock, NULL, _IOLBF, 0);
            if ((fputs(wbuffer, wsock)) == EOF) {
                perror("send");
                exit(1);
            }
            sent = 1;
        }
        
        if (FD_ISSET(sockfd, &readfds)) {
            setvbuf(rsock, NULL, _IOLBF, 0);
            if (fgets(rbuffer, MAXDATASIZE, rsock) == NULL)
                break;
            recLines += 1;
            recBytes += strlen(rbuffer);
            fputs(rbuffer, stdout);
        }
    }
    
    endTime = times(NULL);   /* stop time counting */
    elapsedTime = (float)((endTime - startTime) / (float)sysconf(_SC_CLK_TCK));

    close(sockfd);
    free(rbuffer);
    free(wbuffer);
    
    /* send statistics to stderr */
    fprintf(stderr, "Linhas enviadas:        %d\n", sentLines);
    fprintf(stderr, "Linhas recebidas:       %d\n", recLines);
    fprintf(stderr, "Caracteres enviados:    %d\n", sentBytes);
    fprintf(stderr, "Caracteres recebidos:   %d\n", recBytes);
    fprintf(stderr, "Tamanho da maior linha: %d\n", longestLine);
    fprintf(stderr, "Tempo total: %4.2fs\n", elapsedTime);
    
    return 0;
}
