/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for exectuting olc commands.
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
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: generic.c,v 1.9 1999-01-22 23:12:05 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: generic.c,v 1.9 1999-01-22 23:12:05 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>


ERRCODE
ORequest(Request,code)
     REQUEST *Request;
     int code;
{
  int fd;
  int status;
  
  Request->request_type = code;
  status = open_connection_to_daemon(Request, &fd);
  if(status)
    return(status);

  status = send_request(fd,Request);
  if(status)
    {
      close(fd);
      return(status);
    }

  read_response(fd,&status);
  close(fd);
  return(status);
}

