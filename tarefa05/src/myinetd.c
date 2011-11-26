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

#define BACKLOG 10         /* how many pending connections queue will hold */
#define MAXFD 64
#define MAXLOGMSG 1000
#define MAXSERVICES 3      /* number of services provided */
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

    fprintf(fp, "%s%s\n", buf, msg);
    fclose(fp);
}

void printServices(service services[], int total) {  // só pra testes, remover ao final
    int i;
    for(i=0; i<total; i++) {
        printf("[%s] ", services[i].name);
        printf("[%d] ", services[i].port);
        printf("[%d] ", services[i].socketType);
        printf("[%s] ", services[i].protoc);
        printf("[%s] ", services[i].wait);
        printf("[%s] ", services[i].pathname);
        printf("[%s] \n", services[i].args);
    }
}


int main(int argc, char * argv[])
{
    service services[MAXSERVICES];
    char line[MAXLINESIZE];
    int servIndex = 0;
    int sockfd[MAXSERVICES], new_fd;
    int optval = 1;
    int i;
    struct sockaddr_in my_addr[MAXSERVICES], their_addr;
    
    // open configuration file
    char configFileName[] = "myinetd.conf";
    FILE* configFile = fopen(configFileName, "r");
    
    // read services
    while(fgets(line, MAXLINESIZE, configFile) != NULL)
    {
        strcpy(services[servIndex].name, strtok (line, " "));
        services[servIndex].port = atoi(strtok (NULL, " "));
        
        if( !strcmp (strtok (NULL, " "), "stream") )
            services[servIndex].socketType = SOCK_STREAM;
        else
            services[servIndex].socketType = SOCK_DGRAM;
            
        strcpy(services[servIndex].protoc, strtok (NULL, " "));
        strcpy(services[servIndex].wait, strtok (NULL, " "));
        strcpy(services[servIndex].pathname, strtok (NULL, " "));
        strcpy(services[servIndex].args, strtok (NULL, "\n"));
        
        servIndex++;
    }
    
    // total number of services
    int totalServ = servIndex;
    
    // for all services, create the appropriate socket, bind, and start listening (if tcp)
    for (i = 0; i < totalServ; i++) 
    {
        my_addr[i].sin_family = AF_INET;                 /* host byte order */
        my_addr[i].sin_port = htons(services[i].port);   /* short, network byte order */
        my_addr[i].sin_addr.s_addr = INADDR_ANY;         /* automatically fill with my IP */
        bzero(&(my_addr[i].sin_zero), 8);                /* zero the rest of the struct */
        
        if ((sockfd[i] = socket(AF_INET, services[i].socketType, 0)) == -1) {
            perror("socket");
            exit(1);
        }   
            
        // lose the pesky "address already in use" error message
        if (setsockopt(sockfd[i], SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        
        if (bind(sockfd[i], (struct sockaddr *)&my_addr[i], sizeof(struct sockaddr)) == -1) {
            perror("bind");
            exit(1);
        }
        
        // tcp socket, start listening
        if ( ! strcmp(services[i].protoc, "tcp") ) {
            if (listen(sockfd[i], BACKLOG) == -1) {
                perror("listen");
                exit(1);
            }
        }
    }
    
    fd_set fds;
    FD_ZERO(&fds);
    
    /* main loop */
    while(1)
    {
        for (i = 0; i < totalServ; i++) {    //TODO: setar todos sempre???
            FD_SET(sockfd[i], &fds);
        }
        
        if (select(sockfd[totalServ-1]+1, &fds, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(1);
        }
        
        // iterate through all the services to check which one was called
        for (i = 0; i < totalServ; i++)
        {
            if (FD_ISSET(sockfd[i], &fds))
            {
                if ( ! strcmp(services[i].protoc, "tcp") )      // TCP socket
                {
                    unsigned int sin_size = sizeof(struct sockaddr_in);
                    
                    if ((new_fd = accept(sockfd[i], (struct sockaddr *)&their_addr, &sin_size)) == -1) {
                        perror("accept");
                        exit(1);
                    }
                    if ( ! fork() )
                    {
                        // TODO: trata servico TCP
                    }
                    close( new_fd );
                }
                else    // TODO: UDP socket
                {
                    if ( ! fork() )
                    {
                        //trata servico UDP
                    }
                }
            }
        }
    }
    
    return 0;
}
