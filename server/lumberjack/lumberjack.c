/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains the lumberjack program, which gets rid of old logs.
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/lumberjack/lumberjack.c,v $
 *	$Id: lumberjack.c,v 1.15 1991-05-06 23:19:03 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/lumberjack/lumberjack.c,v 1.15 1991-05-06 23:19:03 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>

#include <sys/types.h>
#include <sys/dir.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <strings.h>
#include <stdio.h>
#if !defined(_AIX) && !defined(SYSV) && !defined(WEXITSTATUS)
/* BSD, need to define macro to get at exit status */
#define	WEXITSTATUS(st)	(st).w_retcode
#define	WTERMSIG(st)	(st).w_termsig
#endif

#include <olcd.h>

#include <lumberjack.h>		/* contains the name of the dir we want */

main (argc, argv)
     int argc;
     char **argv;
{
  DIR *dirp;			/* pointer to directory */
  struct direct *next;		/* directory entry */
  int lock_fd;			/* file descriptor of lock file */
  int fd;			/* file descriptor of control file */
  int retval;			/* Error code returned by system */
  FILE *file;			/* file stream used to read control file */
#if !defined(_i386) && !defined(SYSV)
  union wait status;
#else
  int status;
#endif

  char logname[SIZE];		/* name of log file */
  char title[SIZE];		/* title assigned to log */
  char topic[SIZE];		/* topic of question, also meeting name */
  char username[SIZE];		/* name of person that asked the question */

  char logbuf[SIZE];		/* buffer for constructing error message */

  char  prefix[128];		/* prefix for discuss meeting names */
  char *temp;

/*
 *  Set up syslogging
 */

#ifdef mips
#ifdef LOG_CONS
	openlog ("rpd", LOG_CONS | LOG_PID);
#else
	openlog ("rpd", LOG_PID);
#endif /* LOG_CONS */
#else
#ifdef LOG_CONS
	openlog ("rpd", LOG_CONS | LOG_PID, SYSLOG_LEVEL);
#else
	openlog ("rpd", LOG_PID, SYSLOG_LEVEL);
#endif /* LOG_CONS */
#endif /* ultrix */

/*
 *  Chdir to the directory containing the done'd logs, in case we dump
 *  core or something.
 */

  if (chdir(DONE_DIR))
    {
      syslog(LOG_ERR,"chdir: %m");
      exit(-1);
    }

/*
 *  If we can't open/create the lock file and lock it, exit.
 */

  if ((lock_fd = open(LOCKFILE, O_CREAT, 0666)) < 0)
    {
      syslog(LOG_ERR,"open (lock file): %m");
      exit(-1);
    }
  if (flock(lock_fd, LOCK_EX | LOCK_NB))
    {
      syslog(LOG_ERR,"flock: %m");
      close(lock_fd);
      exit(-1);
    }

/*
 * Find out where we're supposed to be putting these logs...
 */
  if ((fd = open(PREFIXFILE, O_RDONLY)) < 0) {
    syslog(LOG_ERR,"open (prefix file): %m");
    exit(-1);
  }
  retval = read(fd,prefix,128);
  if (retval == -1) {
    syslog(LOG_ERR,"read (prefix file): %m");
    exit(-1);
  }
  close(fd);
  temp = index(prefix,'\n');
  if (temp != NULL) *temp = '\0';
  temp = index(prefix,' ');
  if (temp != NULL) *temp = '\0';

/*
 *  Open the directory so we can get the entries out...
 */
  
  if ((dirp = opendir(".")) == NULL)
    {
      syslog(LOG_ERR,"opendir: %m");
      close(lock_fd);
      flock(lock_fd, LOCK_UN);
      exit(-1);
    }

/*
 *  Read out the entries and process the ones that begin with "ctrl".
 */

  while ((next = readdir(dirp)) != NULL)
    {
      if (!strncmp(next->d_name, "ctrl", 4))
	{
	  if ((fd = open(next->d_name, O_RDONLY, 0)) <= 0)  {
	    sprintf(logbuf,"open (%s) %%m",next->d_name);
	    syslog(LOG_WARNING,logbuf);
	    continue;
	  }
	  file = fdopen(fd, "r");

	  if (fgets(logname, SIZE, file) == NULL)  {
	    sprintf(logbuf,"unable to get logfilename from %s %%m",
		    next->d_name);
	    syslog(LOG_WARNING,logbuf);
	    fclose(file);
	    continue;
	  }
	  logname[strlen(logname) - 1] = '\0'; /* fgets leaves a '\n' on */
					       /* the end.  get rid of it. */

	  if (fgets(title, SIZE, file) == NULL)  {
	    sprintf(logbuf,"unable to get title from %s %%m", next->d_name);
	    syslog(LOG_WARNING,logbuf);
	    fclose(file);
	    continue;
	  }
	  title[strlen(title) - 1] = '\0';
	  
	  if (fgets(topic, SIZE, file) == NULL)  {
	    sprintf(logbuf,"unable to get topic from %s %%m",
		    next->d_name);
	    syslog(LOG_WARNING,logbuf);
	    fclose(file);
	    continue;
	  }
	  topic[strlen(topic) - 1] = '\0';

	  if (fgets(username, SIZE, file) == NULL)  {
	    sprintf(logbuf,"unable to get time from %s %%m",
		    next->d_name);
	    syslog(LOG_WARNING,logbuf);
	    fclose(file);
	    continue;
	  }
	  username[strlen(username) - 1] = '\0';
	  fclose(file);

/* If we've made it this far, we've got everything we need to ship to
 * discuss.
 */ 
#ifdef NO_VFORK
	  retval = fork();
#else
	  retval = vfork();
#endif
	  if (retval == -1) {
	    perror("lumberjack: fork");
	    continue;
	  }
	  if (retval == 0) {
	    char av1[SIZE], av3[SIZE]; 
	    int fd = open(logname, O_RDONLY);
	    if (fd == -1) {
	      perror("lumberjack: open");
	      return -errno;
	    }
	    if (fd != 0)
	      if (dup2(fd, 0) == -1) {
		perror("lumberjack: dup2");
		return -errno;
	      }
	    sprintf (av1, "%s%s", prefix, topic);
	    sprintf (av3, "%s: %s", username, title);
	    retval = execl(DSPIPE, DSPAV0, av1, "-t", av3, NULL);
	    perror("lumberjack: execl");
	    return (-retval);
	  }
 	  /* Assume dspipe is the only child */
 	  retval = wait(&status);
 	  if (retval == -1) {
	    perror("lumberjack: wait");
	    continue;
	  }
 	  if (WIFEXITED(status)) {
	    /* dspipe sometimes loses and returns a bogus error value (36096) */
	    if (WEXITSTATUS(status) != 0) {
	      fprintf(stderr, "lumberjack: %s exited %d\n", DSPIPE,
		      WEXITSTATUS(status));
	    } else {
	      unlink(logname);
	      unlink(next->d_name);
	    }
	  } else /* signal */
	    fprintf(stderr, "lumberjack: %s edited with signal %d\n",
		    DSPIPE, WTERMSIG(status));
	}
    }
  closedir(dirp);
  flock(lock_fd, LOCK_UN);
}
