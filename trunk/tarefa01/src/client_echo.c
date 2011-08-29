/*
** client.c -- a stream socket client demo
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

#define PORT 3490          /* the port client will be connecting to */
#define MAXDATASIZE 1000   /* max number of bytes we can get at once */

/* TODO list
 * (1) colocar tempo no lugar correto
 */

int main(int argc, char *argv[])
{
    int sockfd, numBytes;
    struct hostent *he;
    struct sockaddr_in their_addr;   /* connector's address information */
    clock_t startTime, endTime;
    struct tms start, end;
    float elapsedTime;
    char *buffer = (char*)malloc(MAXDATASIZE*sizeof(char));
    int sentLines, recLines, sentBytes, recBytes, longestLine, lineSize;
    
    startTime = times(&start); //TODO (1)

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

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
    
    sentLines = recLines = sentBytes = recBytes = longestLine = lineSize = 0;
    
    while((buffer = fgets(buffer, MAXDATASIZE, stdin)) != NULL) {
        
        lineSize = strlen(buffer);
        sentLines += 1;
        sentBytes += lineSize;
        if(lineSize > longestLine)
            longestLine = lineSize;
        
        if ((send(sockfd, buffer, strlen(buffer), 0)) == -1) {
            perror("send");
            exit(1);
        }
        
        if ((numBytes = recv(sockfd, buffer, MAXDATASIZE, 0)) == -1) {
            perror("recv");
            exit(1);
        }

        buffer[numBytes] = '\0';
        recLines += 1;
        recBytes += numBytes;

        fputs(buffer, stdout);
    }

    close(sockfd);
    free(buffer);
    
    fprintf(stderr, "Linhas enviadas:        %d\n", sentLines);
    fprintf(stderr, "Tamanho da maior linha: %d\n", longestLine);
    fprintf(stderr, "Caracteres enviados:    %d\n", sentBytes);
    fprintf(stderr, "Linhas recebidas:       %d\n", recLines);
    fprintf(stderr, "Caracteres recebidos:   %d\n", recBytes);
    
    endTime = times(&end); //TODO (1)
    elapsedTime = (float)((endTime - startTime) / sysconf(_SC_CLK_TCK));
    fprintf(stderr, "Tempo total: %4.1fs\n", elapsedTime);    
    
    return 0;
}
