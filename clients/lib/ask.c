/*
 * This file is part of the OLC On-Line Consulting System.
 * It makes the ask request to the server.
 *
 *      Win Treese
 *      Dan Morgan
 *      Bill Saphir
 *      MIT Project Athena
 *
 *      Ken Raeburn
 *      MIT Information Systems
 *
 *      Tom Coppeto
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: ask.c,v 1.20 1999-01-22 23:12:01 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: ask.c,v 1.20 1999-01-22 23:12:01 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>

ERRCODE
OAsk_buffer(Request,topic,buf)
     REQUEST *Request;
     char *topic;
     char *buf;
{
  int fd;
  int status;
  FILE *f;
  char s[BUF_SIZE], machinfo[BUF_SIZE];
 
  /* Start this early so that things aren't blocked on it later */

  if (! (is_option(Request->options,VERIFY))) {
#ifdef ATHENA
  /* This should be gotten rid of when the Mac gets machtype compiled for it */
#ifdef _AUX_SOURCE
    strcpy(machinfo,"Macintosh AUX, 8M");
#else
#ifdef _IBMR2
    f = popen("/bin/athena/machtype -c -d -M -L", "r");
#else
    f = popen("/bin/athena/machtype -c -d -M -v -L", "r");
#endif
  }

  Request->request_type = OLC_ASK;
  status = open_connection_to_daemon(Request, &fd);
  if(status)
    return(status);

  status = send_request(fd, Request);
  if(status)
    {
      close(fd);
      fclose(f);
      return(status);
    }

  read_response(fd, &status);

  if(status == SUCCESS)
    {
      write_text_to_fd(fd,topic);
      read_response(fd,&status);
    }
  
  if(is_option(Request->options,VERIFY) || status != SUCCESS)
    {
      close(fd);
      return(status);
    }

  write_text_to_fd(fd,buf);
  read_response(fd,&status);

  if (status != SUCCESS)
    {
      close(fd);
      return(status);
    }

  machinfo[0] = '\0';
  while (fgets(s, BUF_SIZE, f) != NULL)
    {
      strncat(machinfo, s, strlen(s) - 1);
      strcat(machinfo, ", ");
    }
  machinfo[strlen(machinfo) - 2] = '\0';
#endif /* m68k */
#else
  /* Put something useful for your machine; typically
     processor, display, memory
  */
  strcpy(machinfo,"No Machine Information compiled in\n");
#endif /* Athena */
  write_text_to_fd(fd, machinfo);
  read_response(fd, &status);

  if ((status == CONNECTED) || (status == NOT_CONNECTED))
    read_int_from_fd(fd,&(Request->requester.instance));
  close(fd);
  fclose(f);
  return(status);
}

ERRCODE
OAsk_file(Request,topic,file)
     REQUEST *Request;
     char *topic;
     char *file;
{
  char *buf = NULL;
  ERRCODE status;

  if (! (is_option(Request->options,VERIFY))) {
    status = read_file_into_text(file,&buf);
    if (status != SUCCESS)
      return(status);
  }

  status = OAsk_buffer(Request,topic,buf);
  if (buf != NULL) {
    free(buf);
  }
  return(status);
}
