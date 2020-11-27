/*-------------------------------------------------------------------------*
 *---									---*
 *---		selfCaller.c						---*
 *---									---*
 *---	    This program demonstrates process programming and		---*
 *---	signalling by "recursively" calling itself a limited number of	---*
 *---	and selectively signally its parent process.			---*
 *---									---*
 *---	----	----	----	----	----	----	----	----	---*
 *---									---*
 *---	Version 1a		2020 April 18		Joseph Phillips	---*
 *---									---*
 *-------------------------------------------------------------------------*/

#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<unistd.h>
#include	<signal.h>

const int	TEXT_LEN	= 16;

const int	NUM_SECS_TO_RUN	= 30;

#define		PROGNAME	"selfCaller"

int		numTimesCalled[3]
				= {0,0,0};

pid_t		pidToSignal	= -1;

pid_t		childPid	= -1;

int		level		= +2;

int		shouldRun	= 1;

void		sigAlarmHandler	(int		sigNum
				)
{
  printf("Process %d: called level 0\n",level);

  if  (pidToSignal > 0)
  {
    kill(pidToSignal,SIGUSR1);
  }

  alarm(rand() % 10 + 1);
  numTimesCalled[0]++;
}


void		sigUser1Handler	(int		sigNum
				)
{
  printf("Process %d: called level 1\n",level);

  if  (pidToSignal > 0)
  {
    kill(pidToSignal,SIGUSR2);
  }

  numTimesCalled[1]++;
}


void		sigUser2Handler	(int		sigNum
				)
{
  printf("Process %d: called level 2\n",level);
  numTimesCalled[2]++;
}


void		sigIntHandler	(int		sigNum
				)
{
  shouldRun	= 0;
}


int		main		(int		argc,
				 char*		argv[]
				)
{
  if  (argc > 1)
  {
    level	= strtol(argv[1],NULL,10);

    if  ( (level < 0) || (level > 2) )
    {
      level	= 2;
    }
  }
  else
  {
    level	= 2;
  }

  if  ( (level > 0) && ( (childPid = fork()) == 0) )
  {
    char	text[TEXT_LEN];

    snprintf(text,TEXT_LEN,"%d",level-1);
    execl(PROGNAME,PROGNAME,text,NULL);
    fprintf(stderr,"Cannot find %s\n",PROGNAME);
    exit(EXIT_FAILURE);
  }

  srand(getpid());

  struct sigaction	act;

  memset(&act,'\0',sizeof(act));
  act.sa_handler	= sigAlarmHandler;
  sigaction(SIGALRM,&act,NULL);
  act.sa_handler	= sigUser1Handler;
  sigaction(SIGUSR1,&act,NULL);
  act.sa_handler	= sigUser2Handler;
  sigaction(SIGUSR2,&act,NULL);
  act.sa_handler	= sigIntHandler;
  sigaction(SIGINT,&act,NULL);

  alarm(rand() % 10 + 1);

  if  (level == 2)
  {
    int		i;

    for  (i = 0;  i < NUM_SECS_TO_RUN;  i++)
    {
      sleep(1);
    }
  }
  else
  {
    pidToSignal	= getppid();

    while  (shouldRun)
    {
      sleep(1);
    }
  }

  if  (level > 0)
  {
    kill(childPid,SIGINT);
    wait(NULL);
  }

  printf("Level %d: %d %d %d\n",level,
	 numTimesCalled[0],numTimesCalled[1],numTimesCalled[2]
	);

  return(EXIT_SUCCESS);  
}
