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

#define MAXDATASIZE 1000   /* max number of bytes we can get at once */


int main()
{
    char buffer[ MAXDATASIZE ];
    
    while ( fgets( buffer, MAXDATASIZE, stdin ) != NULL )
    {
        fputs( buffer, stdout );
        fflush( stdout );
    }
    
    return 0;
}
