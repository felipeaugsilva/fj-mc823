/*
 * cludp_echo.c - Cliente de echo em UDP
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

#define SERVERPORT 3490 // the port users will be connecting to
#define MAXDATASIZE 1000   /* max number of bytes we can get at once */

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in their_addr; // connector's address information
    struct hostent *he;
    socklen_t addr_len;
    int sentLines, recLines, sentBytes, recBytes, longestLine, lineSize, numBytes;
    clock_t startTime, endTime;
    float elapsedTime;
    char *buffer = (char*)malloc(MAXDATASIZE*sizeof(char));

    if (argc != 3) {
        fprintf(stderr, "usage: talker hostname mode(0-1)\n");
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

    their_addr.sin_family = AF_INET;     // host byte order
    their_addr.sin_port = htons(SERVERPORT); // short, network byte order
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct

    //if ((numbytes = sendto(sockfd, argv[2], strlen(argv[2]), 0,
    //        (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
    //   perror("sendto");
    //   exit(1);
    //}

    sentLines = recLines = sentBytes = recBytes = longestLine = lineSize = 0;
    
    startTime = times(NULL);   /* start time counting */
    
    // use sendto and recvfrom
    if(atoi(argv[2]) == 0) {
    
        while((buffer = fgets(buffer, MAXDATASIZE, stdin)) != NULL) {
          
          lineSize = strlen(buffer);
          sentLines += 1;
          sentBytes += lineSize;
          if(lineSize > longestLine)
              longestLine = lineSize;
          
          if ((sendto(sockfd, buffer, strlen(buffer), 0,
                     (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
              perror("sendto");
              exit(1);
          }
            
          if ((numBytes = recvfrom(sockfd, buffer, MAXDATASIZE-1 , 0,
                      (struct sockaddr *)&their_addr, &addr_len)) == -1) {
              perror("recvfrom");
              exit(1);
          }

          buffer[numBytes] = '\0';
          recLines += 1;
          recBytes += numBytes;

          fputs(buffer, stdout);
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
        if ((send(sockfd, "", 0, 0)) == -1) {
          perror("send_zero_bytes");
          exit(1);
        }
    }    
    
    endTime = times(NULL);   /* stop time counting */
    elapsedTime = (float)((endTime - startTime) / (float)sysconf(_SC_CLK_TCK));

    close(sockfd);
    free(buffer);
    
    /* send statistics to stderr */
    fprintf(stderr, "Linhas enviadas:        %d\n", sentLines);
    fprintf(stderr, "Tamanho da maior linha: %d\n", longestLine);
    fprintf(stderr, "Caracteres enviados:    %d\n", sentBytes);
    fprintf(stderr, "Linhas recebidas:       %d\n", recLines);
    fprintf(stderr, "Caracteres recebidos:   %d\n", recBytes);
    fprintf(stderr, "Tempo total: %4.2fs\n", elapsedTime);

    return 0;
}
