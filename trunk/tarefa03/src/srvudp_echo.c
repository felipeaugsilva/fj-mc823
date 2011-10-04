/*
** listener.c -- a datagram sockets "server" demo
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

#define MYPORT 3490 // the port users will be connecting to
#define MAXBUFLEN 1000

int main(void)
{
    int sockfd;
    struct sockaddr_in my_addr;    // my address information
    struct sockaddr_in their_addr; // connector's address information
    socklen_t addr_len;
    int numBytes, totalBytes, totalReads;
    char buffer[MAXBUFLEN];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    my_addr.sin_family = AF_INET;         // host byte order
    my_addr.sin_port = htons(MYPORT);     // short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
    memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

    if (bind(sockfd, (struct sockaddr *)&my_addr,
        sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    while(1) {  /* main accept() loop */
    
        totalReads = totalBytes = 0;
        
        do {
            addr_len = sizeof(struct sockaddr);
            if ((numBytes = recvfrom(sockfd, buffer, MAXBUFLEN-1 , 0,
                (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                perror("recvfrom");
                exit(1);
            }

	        if ((sendto(sockfd, buffer, numBytes, 0,
			         (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
		        perror("sendto");
		        exit(1);
	        }
            
            totalReads++;
            totalBytes += numBytes;
            
        } while(numBytes > 0);
        
        fprintf(stderr, "Total de leituras:   %d\n", totalReads);
        fprintf(stderr, "Total de caracteres: %d\n", totalBytes);
    }

    close(sockfd);

    return 0;
}
