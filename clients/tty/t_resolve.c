/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for resolving questions.
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
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_resolve.c,v $
 *	$Id: t_resolve.c,v 1.15 1992-02-14 21:22:01 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_resolve.c,v 1.15 1992-02-14 21:22:01 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_tty.h>


ERRCODE
t_done(Request,title,check)
     REQUEST *Request;
     char *title;
     int check;
{
  int status;
  char buf[LINE_SIZE];
  int instance; 

  instance = Request->requester.instance;

  if(title == (char *) NULL)
    {
      set_option(Request->options,VERIFY);
      status = ODone(Request,title);
      unset_option(Request->options, VERIFY);

      switch(status)
	{
	case SEND_INFO:
	  if (check) {
	    if(isme(Request))
	      status = t_check_connected_messages(Request);
	    else
	      status = t_check_messages(Request);
	    
	    if(status == SUCCESS)
	      {
		*buf = '\0';
		get_prompted_input("Do you wish to resolve this question? ",
				   buf,LINE_SIZE,0);
		if(*buf != 'y')
		  return(SUCCESS);
	      }
	  }
	  get_prompted_input("Enter a title for this conversation: ", buf,
			     LINE_SIZE,0);
	  title = &buf[0];
	  break;

	case OK:
	  if (check) {
	    status = t_check_messages(Request);
	    if(status == SUCCESS)
	      {
		*buf = '\0';
		get_prompted_input("Do you wish to mark this question \"done\"? "
				   ,buf, LINE_SIZE,0);
		if(*buf != 'y')
		  return(SUCCESS);
	      }
	    printf("Using this command means that the %s has satisfactorly answered\n",DEFAULT_CONSULTANT_TITLE);
	    printf("your question.  If this is not the case, you can exit using the 'quit' command,\n");
	    printf("and %s will save your question until a %s can answer it.  If you\n",
                    OLC_SERVICE_NAME,DEFAULT_CONSULTANT_TITLE);
	    printf("wish to withdraw your question, use the 'cancel' command.\n");
	    buf[0] = '\0';
	    get_prompted_input("Really done? [y/n] ", buf, LINE_SIZE,0);
	    if(buf[0] != 'y')             
	      {
		printf("OK, your question will remain in the queue.\n");
		return(NO_ACTION);
	      }
	  }
	  break;
	case PERMISSION_DENIED:
	  fprintf(stderr, "You are not allowed to resolve %s's question.\n",
		  Request->target.username);
	  return(ERROR);
	  
	case NOT_CONNECTED:
	case NO_QUESTION:
	  if(OLC) {
	    printf("You do not have a question in %s.\n", OLC_SERVICE_NAME);
	    printf("Type \"ask\" to ask a question, or \"quit\" to quit.\n");
	    return(NO_ACTION);
	  }
	  else {
	    fprintf(stderr,"You do not have a question to resolve.\n");
	    return(ERROR);
	  }
	default:
	  status = handle_response(status, Request);
	  if(status != SUCCESS)
	    return(status);
	  break;
	}
    }

  status = ODone(Request,title);
  
  switch(status)
    {
    case SIGNED_OFF:
      printf("Question resolved. ");
      if(is_option(Request->options,OFF_OPT))
	printf("You have signed off %s.\n", OLC_SERVICE_NAME);
      else
	printf("You are signed off %s.\n", OLC_SERVICE_NAME);

      t_set_default_instance(Request);
      status = SUCCESS;
      break;

    case CONNECTED:
      printf("Question resolved.  You have been connected to another user.\n");
      status = SUCCESS;
      break;

    case OK:
      printf("The %s has been notified that you are finished with your question.\n",DEFAULT_CONSULTANT_TITLE);
      printf("Thank you for using %s!\n", OLC_SERVICE_NAME);
      if(OLC) {
	exit(0);
      }

      t_set_default_instance(Request);
      break;

    case SUCCESS:
      if(isme(Request))
	printf("Your question is resolved. Thank you for using %s.\n",
	       OLC_SERVICE_NAME);
      else
	  printf("%s's [%d] question is resolved.\n",
		 Request->target.username, Request->target.instance);
      if(OLC) {
	exit(0);
      }

      t_set_default_instance(Request);
      break;

    case PERMISSION_DENIED:
      fprintf(stderr, "You are not allowed to resolve %s's question.\n",
              Request->target.username);
      return(ERROR);

    case ERROR:
      fprintf(stderr, "An error has occurred.  The question\n");
      fprintf(stderr, "has not been marked resolved.\n");
      status = ERROR;
      break;

    default:
      status = handle_response(status, Request);
      break;
    }

    if(instance != Request->requester.instance)
    printf("%s [%d] has been deactivated.  You are %s [%d].\n",
  	Request->requester.username, instance,
        Request->requester.username, Request->requester.instance);

  return(status);
}

ERRCODE
t_cancel(Request,title)
     REQUEST *Request;
     char *title;
{
  int status;
  char buf[BUFSIZ];
  int instance;

  instance = Request->requester.instance;
  
  set_option(Request->options,VERIFY);
  status = OCancel(Request,title);
  unset_option(Request->options, VERIFY);

  switch(status)
    {
    case OK:
      if(OLC)
	{
	  printf("Using this command means that you want to withdraw your question. If you \n");
	  printf("do not, %s will store your question until a %s can answer it.\n",
		OLC_SERVICE_NAME,DEFAULT_CONSULTANT_TITLE);
	  printf("In that case, exit using the 'quit' command. If the %s has\n",DEFAULT_CONSULTANT_TITLE);
	  printf("satisfactorily answered your question, use the 'done' command to exit %s.\n", OLC_SERVICE_NAME);
	}

      buf[0] = '\0';
      get_prompted_input("Are you sure you wish to cancel this question? ",
			 buf, BUFSIZ,0);
      if(buf[0] != 'y')            
	{
	  printf("OK, your question will remain in the queue.\n");
	  return(NO_ACTION);
	}
      break;

    case NOT_CONNECTED:
    case NO_QUESTION:
      if(OLC) {
	printf("You do not have a question in %s.\n", OLC_SERVICE_NAME);
	printf("Type \"ask\" to ask a question, or \"quit\" to quit.\n");
	return(NO_ACTION);
      }
      else {
	fprintf(stderr,"You do not have a question to cancel.\n");
	return(ERROR);
      }

    default:
      status = handle_response(status, Request);
      if(status != SUCCESS)
	return(status);
      break;
    }

  Request->request_type = OLC_CANCEL;
  
  if(title == (char *) NULL)
    status = OCancel(Request,"Cancelled Question");
  else
    status = OCancel(Request,title);

  switch(status)
    {
    case SUCCESS:
      printf("Question cancelled. \n");
      t_set_default_instance(Request);
      if(OLC) {
	exit(0);
      }
      status = SUCCESS;
      break;

    case OK:
      printf("Your question has been cancelled.\n");
      if(OLC) {
	exit(0);
      }
      
      t_set_default_instance(Request);
      break;

    case SIGNED_OFF:
      printf("Question cancelled.  ");
      if(is_option(Request->options,OFF_OPT))
	printf("You have signed off %s.\n", OLC_SERVICE_NAME);
      else
	printf("You are signed off %s.\n", OLC_SERVICE_NAME);
      
      t_set_default_instance(Request);
      status = SUCCESS;
      break;

    case CONNECTED:
      printf("You have been connected to another user.\n");
      status = SUCCESS;
      break;

    case ERROR:
      fprintf(stderr, "An error has occurred.  The question ");
      fprintf(stderr, "has not been cancelled.\n");
      status = ERROR;
      break;

    default:
      status = handle_response(status, Request);
      break;
    }

  if(instance != Request->requester.instance)
    printf("%s [%d] has been deactivated.  You are now %s [%d].\n",
	Request->requester.username, instance,
        Request->requester.username, Request->requester.instance);

  return(status);
}



