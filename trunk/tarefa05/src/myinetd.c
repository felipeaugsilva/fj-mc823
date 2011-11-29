/*
 * myinetd.c - Servidor compativel com inetd, simplificado
 * 
 * MC823 - Tarefa 05
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
#include <syslog.h>
#include <sys/stat.h>
#include <sys/select.h>

#define BACKLOG 10         /* how many pending connections queue will hold */
#define MAXFD 64
#define MAXLOGMSG 1000
#define MAXSERVICES 3      /* max number of services provided */
#define MAXLINESIZE 256


/* struct for services defined in myinetd.conf */
typedef struct {
    char name[16];
    int  port;
    int  socketType;
    char protoc[4];
    char wait[7];
    char pathname[32];
    char args[64];
} service;


/* this array has one element for each service (tcp and udp)
   if the service is tcp, its respective value is always 0.
   if it's udp and is not being used, the value is 0, else, 
   the value is the PID of the respective process. */
int udpServiceUsedPid[ MAXSERVICES ];

/* Total number of services provided */
int totalServ = 0;


void daemon_init(const char *pname)
{
    int i;
    pid_t pid;
    
    if ( (pid = fork()) != 0)
        exit(0);                /* parent terminates */
    
    /* 1st child continues */
    setsid();                   /* become session leader */
    signal(SIGHUP, SIG_IGN);    /* when 1st child dies, may kill his child */
    
    if ( (pid = fork()) != 0)
        exit(0);                /* 1st child terminates */
    
    /* 2nd child continues */
    chdir("/tmp");  /* change working directory */
    umask(0);       /* clear our file mode creation mask */
    
    for (i = 0; i < MAXFD; i++)  /* close all file descriptors */
        close(i);
    
    openlog(pname, LOG_PID, 0);  /* tell syslog to log proc pid */
}


/* appends message to log file */
void mysyslog(char *msg)
{
    FILE * fp;
    char buf[64], * ctime();
    time_t time(), now;
    (void) time(&now);               /* get seconds since Epoch into now */

    sprintf(buf, "%s", ctime(&now)); /* change secs to date and time, put in buf */

    if ((fp= fopen("myinetd.log", "a")) == 0)   /* private log file */
        exit(1);
    
    fprintf(fp, "%s%s\n\n", buf, msg);
    fclose(fp);
}

/* Handle SIGCHLD signal */
void handle_sigchld_signal(int sig)
{
    pid_t pid;
    int status;
    char logMsg [MAXLOGMSG ];
    int servIndex;
    
    pid = wait( &status );
    
    sprintf( logMsg, "Finished service: PID %d", pid );
    mysyslog( logMsg );
    
    for (servIndex = 0; servIndex < totalServ; servIndex++) {
        if ( udpServiceUsedPid[ servIndex ] == pid ) {
            // release the udp service so it can be used again
            udpServiceUsedPid[ servIndex ] = 0;
        }
    }
    
    signal( sig, handle_sigchld_signal );
}
 

int main(int argc, char * argv[])
{
    service services[ MAXSERVICES ];
    char line[ MAXLINESIZE ], logMsg [MAXLOGMSG ];
    int sockfd[ MAXSERVICES ], new_fd;
    int buf[MAXLINESIZE];
    struct sockaddr_in my_addr[ MAXSERVICES ], their_addr;
    int servIndex = 0, optval = 1;
    pid_t pid;
    
    fprintf( stderr, "Starting myinetd...\n" );
    
    // open configuration file
    char configFileName[] = "myinetd.conf";
    FILE* configFile = fopen(configFileName, "r");
    
    // read services
    while(fgets(line, MAXLINESIZE, configFile) != NULL)
    {
        strcpy(services[ servIndex ].name, strtok (line, " "));
        services[ servIndex ].port = atoi(strtok (NULL, " "));
        
        if( !strcmp (strtok (NULL, " "), "stream") )
            services[ servIndex ].socketType = SOCK_STREAM;
        else
            services[ servIndex ].socketType = SOCK_DGRAM;
            
        strcpy(services[ servIndex ].protoc, strtok (NULL, " "));
        strcpy(services[ servIndex ].wait, strtok (NULL, " "));
        strcpy(services[ servIndex ].pathname, strtok (NULL, " "));
        strcpy(services[ servIndex ].args, strtok (NULL, " "));
        
        servIndex++;
    }
    fclose( configFile );
    
    // total number of services
    totalServ = servIndex;
    
    daemon_init( argv[0] );          /* install server as a daemon */
    mysyslog( "myinetd started..." );
    
    // for all services, create the appropriate socket, bind, and start listening (if tcp)
    for (servIndex = 0; servIndex < totalServ; servIndex++)
    {
        my_addr[ servIndex ].sin_family = AF_INET;                           /* host byte order */
        my_addr[ servIndex ].sin_port = htons(services[ servIndex ].port);   /* short, network byte order */
        my_addr[ servIndex ].sin_addr.s_addr = INADDR_ANY;                   /* automatically fill with my IP */
        bzero(&(my_addr[ servIndex ].sin_zero), 8);                          /* zero the rest of the struct */
        
        if ((sockfd[ servIndex ] = socket(AF_INET, services[ servIndex ].socketType, 0)) == -1) {
            perror("socket");
            exit(1);
        }   
            
        // lose the pesky "address already in use" error message
        if (setsockopt(sockfd[ servIndex ], SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        
        if (bind(sockfd[ servIndex ], (struct sockaddr *)&my_addr[ servIndex ], sizeof(struct sockaddr)) == -1) {
            perror("bind");
            exit(1);
        }
        
        // tcp socket, start listening
        if ( ! strcmp(services[ servIndex ].protoc, "tcp") ) {
            if (listen(sockfd[ servIndex ], BACKLOG) == -1) {
                perror("listen");
                exit(1);
            }
        }
        
        udpServiceUsedPid[ servIndex ] = 0;
    }
    
    fd_set fds;
    FD_ZERO(&fds);
    
    signal( SIGCHLD, handle_sigchld_signal );
    
    /* main loop */
    while(1)
    {
        for (servIndex = 0; servIndex < totalServ; servIndex++) {  
            // udp services that are being used are not set
            if ( udpServiceUsedPid[ servIndex ] == 0 )
                FD_SET(sockfd[ servIndex ], &fds);
        }
        
        if (select(sockfd[totalServ-1]+1, &fds, NULL, NULL, NULL) < 0) {
            if (errno == EINTR ) {     // error received when a child dies
                continue;
            } else {
                perror("select");
                exit(1);
            }
        }
        
        unsigned int sin_size = sizeof(struct sockaddr_in);
        
        // iterate through all the services to check which one was called
        for (servIndex = 0; servIndex < totalServ; servIndex++)
        {
            if ( FD_ISSET(sockfd[ servIndex ], &fds) )
            {
                // TCP socket
                if ( ! strcmp(services[ servIndex ].protoc, "tcp") )
                {
                    if ((new_fd = accept(sockfd[ servIndex ], (struct sockaddr *)&their_addr, &sin_size)) == -1) {
                        perror("accept");
                        exit(1);
                    }
                    
                    if ( (pid = fork()) == 0 )
                    {
                        // close all sockets other than new_fd
                        int i;
                        for (i = 0; i < totalServ; i++)
                            close( sockfd[ i ] );
                        
                        dup2( fileno(fdopen(new_fd, "w")), 0 );
                        dup2( fileno(fdopen(new_fd, "r")), 1 );
                        dup2( fileno(fdopen(new_fd, "r")), 2 );

                        if ( execl( services[ servIndex ].pathname, services[ servIndex ].args, (char *) 0 ) == -1 )
                            perror("exec");
                        
                        exit( 0 );
                    }
                    
                    sprintf( logMsg, "Started service: %s with PID %d\nClient: %s:%d",
                             services[ servIndex ].name, pid, inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port) );
                    mysyslog( logMsg );
                    
                    close( new_fd );
                }
                
                // UDP socket
                else
                {
                  
                    if ((recvfrom(sockfd[ servIndex ], buf, MAXLINESIZE-1 , MSG_PEEK, (struct sockaddr *)&their_addr, &sin_size)) == -1) 
                    {
                        perror("recvfrom");
                        exit(1);
                    }

                  
                    if ( (pid = fork()) == 0 )
                    {
                        // close all sockets other than the original socket
                        int i;
                        for (i = 0; i < totalServ; i++)
                        {
                           if( i != servIndex ) 
                             close( sockfd[ i ] );
                        }


                        dup2( fileno(fdopen(sockfd[ servIndex ], "w")), 0 );
                        dup2( fileno(fdopen(sockfd[ servIndex ], "r")), 1 );
                        dup2( fileno(fdopen(sockfd[ servIndex ], "r")), 2 );

                        close( sockfd[ servIndex ] );

                        if ( execl( services[ servIndex ].pathname, services[ servIndex ].args, (char *) 0 ) == -1 )
                            perror("exec");

                        exit( 0 );
                      
                    }
                    
                    FD_CLR(sockfd[ servIndex ], &fds);
                    
                    udpServiceUsedPid[ servIndex ] = pid;
                    
                    sprintf( logMsg, "Started service: %s with PID %d\nClient: %s:%d",
                             services[ servIndex ].name, pid, inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port) );
                    mysyslog( logMsg );                   
                   
                }
            }
        }
    }
    
    return 0;
}
