/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for dealing with topics.
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
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/info.c,v $
 *	$Id: info.c,v 1.2 1990-05-26 11:52:10 vanharen Exp $
 *	$Author: vanharen $
 */

#ifndef lint
static const char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/info.c,v 1.2 1990-05-26 11:52:10 vanharen Exp $";
#endif

#include <mit-copyright.h>
#include <olc/olc.h>


/*
 * Function:	OGetTopic() 
 * Description: Gets the current conversation topic specified in the Request.
 * Returns:	ERRCODE
 */


ERRCODE
OGetNewTopic(Request,topic)
     REQUEST *Request;
     char *topic;
{
  int status;
  LIST data;

  status = OWho(Request,&data);
  if(status == SUCCESS)      
    strcpy(topic,data.topic);
  return(status);
}

ERRCODE
OGetUsername(Request,name)
     REQUEST *Request;
     char *name;
{
  int status;
  LIST data;

  strcpy(name,Request->requester.username);
  return(SUCCESS);
}

ERRCODE
OGetConnectedUsername(Request,name)
     REQUEST *Request;
     char *name;
{
  int status;
  LIST data;

  status = OWho(Request,&data);
  if(status == SUCCESS)      
    strcpy(name,data.connected.username);
  return(status);
}

ERRCODE
OGetHostname(Request,name)
     REQUEST *Request;
     char *name;
{
  int status;
  LIST data;

  strcpy(name,Request->requester.machine);
  return(SUCCESS);
}


ERRCODE
OGetConnectedHostname(Request,name)
     REQUEST *Request;
     char *name;
{
  int status;
  LIST data;

  status = OWho(Request,&data);
  if(status == SUCCESS)      
    strcpy(name,data.connected.machine);
  return(status);
}



