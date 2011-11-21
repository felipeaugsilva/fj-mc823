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

#define MYPORT 3490        /* the port users will be connecting to */
#define MAXFD 64
#define MAXLOGMSG 1000
#define MAXSERVICES 64
#define MAXLINESIZE 256


/* struct for services defined in myinetd.conf */
typedef struct {
    char name[16];
    char port[8];
    char socketType[7];
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
        printf("[%s] ", services[i].port);
        printf("[%s] ", services[i].socketType);
        printf("[%s] ", services[i].protoc);
        printf("[%s] ", services[i].wait);
        printf("[%s] ", services[i].pathname);
        printf("[%s] \n", services[i].args);
    }
}


int main(int argc, char * argv[])
{
    char configFileName[] = "myinetd.conf";
    FILE* configFile = fopen(configFileName, "r");
    service services[MAXSERVICES];
    char line[MAXLINESIZE];
    int servIndex = 0;

    // read services
    while(fgets(line, MAXLINESIZE, configFile) != NULL) {
        strcpy(services[servIndex].name, strtok (line, " "));
        strcpy(services[servIndex].port, strtok (NULL, " "));
        strcpy(services[servIndex].socketType, strtok (NULL, " "));
        strcpy(services[servIndex].protoc, strtok (NULL, " "));
        strcpy(services[servIndex].wait, strtok (NULL, " "));
        strcpy(services[servIndex].pathname, strtok (NULL, " "));
        strcpy(services[servIndex].args, strtok (NULL, "\n"));
        servIndex++;
    }
    
    //printServices(services, servIndex);    // só pra testes, remover ao final
    
    return 0;
}



