/* mydaemon.c -- Um exemplo de daemon a nivel de usuario
daemon_init adaptado de Richard Stevens, Unix Network
Programming vol 2, p 306 Celio G. Abril 2000 */

//#include "myunp.h"
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

#define MAXFD 64

void mysyslog(char * progname);

void daemon_init(const char *pname)
{
    int i;
    pid_t pid;
    
    if ( (pid = fork()) != 0)
        exit(0); /* parent terminates */
    
    /* 1st child continues */
    setsid(); /* become session leader */
    signal(SIGHUP, SIG_IGN); /* when 1st child dies, may kill his child*/
    
    if ( (pid = fork()) != 0)
        exit(0); /* 1st child terminates */
    
    /* 2nd child continues */
    chdir("/tmp"); /* change working directory */
    umask(0); /* clear our file mode creation mask */
    
    for (i = 0; i < MAXFD; i++) /*close all file descriptors*/
        close(i);
    
    openlog(pname, LOG_PID, 0); /*tell syslog to log proc pid*/
}

int main(int argc, char * argv[])
{
    printf("Comecando...\n");
    
    daemon_init( argv[0] ); /* instals me as a daemon */
    
    for (; ;){
        mysyslog(argv[0]); /* log program name */
        sleep(20); /* log one line every 20 secs*/
    }
}

void mysyslog(char *progname) /*appends message to log file*/
{
    FILE * fp;
    char buf[64], * ctime();
    time_t time(), now;
    (void) time(&now); /*get seconds since Epoch into now*/
    
    sprintf(buf, "%s", ctime(&now)); /* change secs to date and time, put in buf */
    
    if ((fp= fopen("ERROR.LOG", "a")) ==0) /* private log file*/
        exit(1);
    
    /* syslog(LOG_INFO|LOG_USER, "%s %s %m\n",progname, buf);*/
    fprintf(fp, "%s %s\n", progname, buf);
    fclose(fp);
}

