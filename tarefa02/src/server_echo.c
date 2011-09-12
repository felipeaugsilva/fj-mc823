/*
 * server_echo.c - Servidor concorrente de echo simples em TCP
 * 
 * MC823 - Tarefa 01
 * Felipe Augusto da Silva        RA 096993
 * Jesse de Moura Tavano Moretto  RA 081704
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MYPORT 3490        /* the port users will be connecting to */
#define BACKLOG 10         /* how many pending connections queue will hold */
#define MAXDATASIZE 1000   /* max number of bytes we can get at once */

int main()
{
    int sockfd, new_fd;              /* listen on sock_fd, new connection on new_fd */
    struct sockaddr_in my_addr;      /* my address information */
    struct sockaddr_in their_addr;   /* connector's address information */
    unsigned int sin_size;
    int numBytes, totalBytes, recLines;
    char buffer[MAXDATASIZE];
    int optval = 1;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    
    /* lose the pesky "address already in use" error message */
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    my_addr.sin_family = AF_INET;          /* host byte order */
    my_addr.sin_port = htons(MYPORT);      /* short, network byte order */
    my_addr.sin_addr.s_addr = INADDR_ANY;  /* automatically fill with my IP */
    bzero(&(my_addr.sin_zero), 8);         /* zero the rest of the struct */

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    while(1) {  /* main accept() loop */
        sin_size = sizeof(struct sockaddr_in);
        
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
            perror("accept");
            continue;
        }
        printf("server: got connection from %s\n", inet_ntoa(their_addr.sin_addr));
        
        if(!fork()) {
            
            recLines = totalBytes = 0;
            
            while ((numBytes = recv(new_fd, buffer, MAXDATASIZE, 0)) > 0) {
                
                if (send(new_fd, buffer, numBytes, 0) == -1) {
                    perror("send");
                    exit(1);
                }
                
                recLines += 1;
                totalBytes += numBytes;
            }
            
            fprintf(stderr, "Total de leituras:   %d\n", recLines);
            fprintf(stderr, "Total de caracteres: %d\n", totalBytes);
            
            close(new_fd);
            exit(0);
        }

        close(new_fd);
        
        while(waitpid(-1,NULL,WNOHANG) > 0);   /* clean up all child processes */
    }

    return 0;
}
