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
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_queue.c,v $
 *      $Author: vanharen $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_queue.c,v 1.7 1990-01-17 02:54:16 vanharen Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_parser.h>

do_olc_queue(arguments)
     char **arguments;
{
  REQUEST Request;
  int instance = -1;
  char queue[NAME_SIZE];
  int status;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  if(*++arguments != NULL)
    {
      if(string_equiv(*arguments,"-queue", max(strlen(*arguments),2)) ||
	 string_equiv(*arguments,"-change", max(strlen(*arguments),2)))
	{
          ++arguments;
	  if(*arguments != (char *) NULL)
	    strncpy(queue,*arguments, NAME_SIZE);
/*	  else
	    t_input_queue(&Request,queue,FALSE);*/
	}
      else
	{
	  arguments = handle_argument(arguments, &Request, &status);
	  if(status)
	    return(ERROR);

	  if(arguments == (char **) NULL)
	    {
	      printf("The argument \"%s\" is invalid.\n",*arguments);
	      printf("Usage is: \tqueue [-queue <queue>] [-change]\n");
	    }
	  return(ERROR);
	}
    }
  else
    return(t_queue(&Request, (char *) NULL));
      

  return(t_queue(&Request, queue));
}
