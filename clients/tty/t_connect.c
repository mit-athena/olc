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
 *      MIT Project Athena
 *
 *      Copyright (c) 1988 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_connect.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_connect.c,v 1.4 1989-08-04 11:11:28 tjcoppet Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_tty.h>

ERRCODE
t_grab(Request,flag,hold)
     REQUEST *Request;
     int flag;
     int hold;
{
  int status;
  char buf[BUFSIZE];
  int instance;
  
  instance = Request->requester.instance;

  if(flag)
    set_option(Request->options,SPLIT_OPT);

  status = OGrab(Request);
  switch (status)
    {
    case GRAB_ME:
      fprintf(stderr, "You cannot grab yourself in OLC.\n");
      status = NO_ACTION;
      break;

    case PERMISSION_DENIED:
      fprintf(stderr,
              "You cannot grab this question.\n");
      status = NO_ACTION;
      break;

    case CONNECTED:
      fprintf(stderr, "You are connected to another user.\n");

    case HAS_QUESTION:
      get_prompted_input("Would you like to create another instance to grab this question? " ,buf);
      if(string_equiv(buf,"yes",1))
	return(t_grab(Request,TRUE,hold));
      status = NO_ACTION;
      break;

    case SUCCESS:
      printf("User grabbed.\n");
      status = SUCCESS;
      break;

    case ALREADY_CONNECTED:
      printf("Someone is already connected to %s (%d).\n",
             Request->target.username,Request->target.instance);
      status = NO_ACTION;
      break;

    case MAX_ANSWER:
      printf("You cannot answer any more questions simultaneously\n");
      printf("without forwarding one of your existing connections.\n");
      status = NO_ACTION;
      break;

    case NO_QUESTION:
      printf("%s (%d) does not have a question.\n",Request->target.username,
	     Request->target.instance);
      status = ERROR;
      break;

    case FAILURE:
    case ERROR:
      fprintf(stderr, "Error grabbing user.\n");
      status = ERROR;
      break;

    default:
      status = handle_response(status,Request);
      break;
    }

  if((instance != Request->requester.instance) && !hold)
    {
      printf("You are now %s (%d).\n",Request->requester.username,
	   Request->requester.instance);
      User.instance =  Request->requester.instance;
    }
  return(status);
}




ERRCODE
t_forward(Request)
     REQUEST *Request;

{
  int status;
  int instance;

  instance = Request->requester.instance;
  status = OForward(Request);
  
  switch (status) 
    {
    case SUCCESS:
      printf("Question forwarded.\n");
      Request->requester.instance = 0;
      User.instance = 0;
      status = SUCCESS;
      break;

    case CONNECTED:
      printf("Question forwarded. You are now connected to another user.\n");
      status = SUCCESS;
      break;

    case SIGNED_OFF:
      printf("Question forwarded. You are signed off of OLC.\n");
      Request->requester.instance = 0;
      User.instance = 0;
      status = SUCCESS;
      break;

    case NOT_CONNECTED:
      fprintf(stderr,"You have no question to forward.\n");
      status = ERROR;
      break;

    case HAS_QUESTION:
      fprintf(stderr,"You cannot forward your own question.\n");
      status = ERROR;
      break;

    case ERROR:
      fprintf(stderr, "Unable to forward question. Dunno why.\n");
      status = ERROR;
      break;

    default:
      status = handle_response(status, Request);
      break;
    }

  if(instance != Request->requester.instance)
    printf("You are %s (%d), again.\n",Request->requester.username,
	   Request->requester.instance);

  return(status);
}

