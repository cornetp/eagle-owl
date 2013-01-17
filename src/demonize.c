/*+--------------------------------------------------------------+
  |            demonize.c  -  description                        |
  |                -------------------                           |
  | begin      : 08/01/2010 13.00                                |
  | copyright  : (C) 2010 xAppSoftware                           |
  | author     : Luigi D'Andrea                                  |
  | email      : gg1 ( at ) xappsoftware.com                     |
  | compiling  : gcc -o server server.c                          |
  |                                                              |
  | Latest version on http://www.xappsoftware.com                |
  +--------------------------------------------------------------+
  | demonize library may be redistributed and modified under     |
  | certain conditions. This software is distributed on an       |
  | "AS IS" basis WITHOUT WARRANTY OF ANY KIND, either express or|
  ! implied. See the file License.txt for details.               |
  +--------------------------------------------------------------+*/
/*+--------------------------------------------------------------+
  | SYSTEM INCLUDES                                              |
  +--------------------------------------------------------------+*/
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>

/*+--------------------------------------------------------------+
  | SPECIFIC INCLUDES                                            |
  +--------------------------------------------------------------+*/
#include "demonize.h"

/*+--------------------------------------------------------------+
  | Function name :  demonize                                    |
  | Parameters    :  the executable name                         |
  | Description   :  Demonizes the caller application            |
  +--------------------------------------------------------------+*/
void demonize(char *arg)
{
	int      childpid;

	(void) signal(SIGHUP, SIG_IGN);
	(void) signal(SIGCHLD, SIG_IGN);
	(void) signal(SIGQUIT, SIG_IGN);

	if ((childpid=fork()) < 0)
	{
		printf("An error occurred while demonizing %s...\n", arg);
	}
	else
	{
		if (!childpid)  /* child */
		{
			printf("Becoming a daemon...\n");
			detachFromTerminal();
		}
		else            /* father */
		{
			exit(EXIT_SUCCESS);
		}
	}

}

/*+--------------------------------------------------------------+
  | Function name :  detachFromTerminal                          |
  | Parameters    :  None                                        |
  | Description   :  Detaches from the terminal the calling      |
  |                  application.                                |
  +--------------------------------------------------------------+*/
void detachFromTerminal(void)
{
	if(setsid()==-1)           /* detach from the terminal */
	{
		perror("setsid()");
		exit(EXIT_FAILURE);
	}

	if (freopen("/dev/null", "r", stdin) == NULL)
	{
		fprintf(stderr, "Unable to replace stdin with /dev/null: %s\n",
		        strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (freopen("/dev/null", "w", stdout) == NULL)
	{
		fprintf(stderr, "Unable to replace stdout with /dev/null: %s\n",
		        strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (freopen("/dev/null", "w", stderr) == NULL)
	{
		fprintf(stderr, "Unable to replace stderr with /dev/null: %s\n",
		        strerror(errno));
		exit(EXIT_FAILURE);
	}
}

