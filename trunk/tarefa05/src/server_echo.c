/*
 * server_echo.c - Servidor de eco compativel com inetd
 * 
 * MC823 - Tarefa 05
 * Felipe Augusto da Silva        RA 096993
 * Jesse de Moura Tavano Moretto  RA 081704
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXDATASIZE 1000   /* max number of bytes we can get at once */


/* appends message to log file */
void mysyslog(char *msg)
{
    FILE * fp;
    char buf[64], * ctime();
    time_t time(), now;
    (void) time(&now);               /* get seconds since Epoch into now */
    
    sprintf(buf, "%s", ctime(&now)); /* change secs to date and time, put in buf */
    
    if ((fp= fopen("server_echo.log", "a")) == 0)   /* private log file */
        exit(1);
    
    fprintf(fp, "%s%s\n\n", buf, msg);
    fclose(fp);
}


int main()
{
    char buffer[ MAXDATASIZE ];
    struct sockaddr_in their_addr;
    
    unsigned int sin_size = sizeof(struct sockaddr_in);
    
    getpeername( 0, (struct sockaddr *)&their_addr, &sin_size );
    
    sprintf( buffer, "Client: %s:%d", inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port) );
    mysyslog( buffer );
    
    while ( fgets( buffer, MAXDATASIZE, stdin ) != NULL )
    {
        fputs( buffer, stdout );
        fflush( stdout );
    }
    
    return 0;
}
