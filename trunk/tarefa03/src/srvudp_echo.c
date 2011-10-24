/*
 * srvudp_echo.c - Servidor de echo em UDP
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

#define MYPORT 3490     // the port users will be connecting to
#define MAXBUFLEN 1000
#define TIMEOUT 5

int main(int argc, char* argv[])
{
    int sockfd;
    struct sockaddr_in my_addr;    // my address information
    struct sockaddr_in their_addr; // connector's address information
    socklen_t addr_len;
    int numBytes, totalBytes, totalReads;
    int first;
    char buffer[MAXBUFLEN];
    
    struct timeval tv;
    fd_set readfds;
    
    if (argc != 2) {
        fprintf(stderr, "usage: %s mode(0-1)\n", argv[0]);
        fprintf(stderr, "mode 0: without connect\nmode 1: using connect\n");
        exit(1);
    }
    
    FD_ZERO(&readfds);

    my_addr.sin_family = AF_INET;         // host byte order
    my_addr.sin_port = htons(MYPORT);     // short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
    memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    
    if (bind(sockfd, (struct sockaddr *)&my_addr,
        sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }
    
    if(!(atoi(argv[1])))
        printf("nonConnect-mode chosen\n");
    else
        printf("Connect-mode chosen\n");
    
    while(1) {  /* main loop */
    
        totalReads = totalBytes = 0;
        first = 1;
        
        // use sendto and recvfrom
        if(!(atoi(argv[1]))) {
      
            do {
                addr_len = sizeof(struct sockaddr);
                tv.tv_sec = TIMEOUT;
                tv.tv_usec = 0;
                FD_SET(sockfd, &readfds);
              
                if ( !first )
                    select(sockfd+1, &readfds, NULL, NULL, &tv);
                else
                    select(sockfd+1, &readfds, NULL, NULL, NULL);   // waiting for the first packet, no timeout
                
                if (FD_ISSET(sockfd, &readfds)) {            
                    if ((numBytes = recvfrom(sockfd, buffer, MAXBUFLEN-1 , 0,
                        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                        perror("recvfrom");
                        exit(1);
                    }
                }
                else {
                    fprintf( stderr, "Timeout (%ds)\n", TIMEOUT );
                    break;
                }
                
                if(numBytes <= 0) break;
                
                if ((sendto(sockfd, buffer, numBytes, 0,
                (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
                  perror("sendto");
                  exit(1);
                }
                
                first = 0;
                totalReads++;
                totalBytes += numBytes;
                
            } while(numBytes > 0);
        }
        
        // use send and recv
        else {
            
            addr_len = sizeof(struct sockaddr);
            
            // using recvfrom to get the client address
            if ((numBytes = recvfrom(sockfd, buffer, MAXBUFLEN-1 , 0,
                (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                perror("recvfrom");
                exit(1);
            }
            
            if(numBytes > 0) 
            {
              
              if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
                  perror("connect");
                  exit(1);
              }
              
              if ((sendto(sockfd, buffer, numBytes, 0,
              (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
                perror("sendto");
                exit(1);
              }

              totalReads++;
              totalBytes += numBytes;
              
              do {
                  tv.tv_sec = TIMEOUT;
                  tv.tv_usec = 0;
                  FD_SET(sockfd, &readfds);
                              
                  select(sockfd+1, &readfds, NULL, NULL, &tv);                
                
                  if (FD_ISSET(sockfd, &readfds)) {
                  
                      if ((numBytes = recv(sockfd, buffer, MAXBUFLEN, 0)) == -1) {
                          perror("recv");
                          break;
                      }
                      if ((send(sockfd, buffer, numBytes, 0)) == -1) {
                          perror("send");
                          break;
                      }
                      totalReads++;
                      totalBytes += numBytes;
                  }
                  else {
                      fprintf( stderr, "Timeout (%ds)\n", TIMEOUT );
                      break;
                  }
                  
              } while(numBytes > 0);
            }
            
            close(sockfd);
            
            if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
                perror("socket");
                exit(1);
            }
            if (bind(sockfd, (struct sockaddr *)&my_addr,
                sizeof(struct sockaddr)) == -1) {
                perror("bind");
                exit(1);
            }
        }
        
        fprintf(stderr, "Total de leituras:   %d\n", totalReads);
        fprintf(stderr, "Total de caracteres: %d\n\n", totalBytes);
    }

    close(sockfd);

    return 0;
}

