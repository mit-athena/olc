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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_connect.c,v $
 *      $Author: vanharen $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_connect.c,v 1.7 1990-04-25 16:40:27 vanharen Exp $";
#endif


#include <olc/olc.h>
#include <olc/olc_parser.h>

/*
 * Function:    do_olc_grab() grabs a pending question.
 * Arguments:   arguments:      The argument array from the parser.
 * Returns:     SUCCESS or ERROR.
 * Notes:
 */

ERRCODE
do_olc_grab(arguments)
     char **arguments;
{
  REQUEST Request;
  int status;
  int flag = 0;
  int hold = 0;

  if(fill_request(&Request) != SUCCESS)
     return(ERROR);

  for(++arguments; *arguments != (char *) NULL; arguments++)
    {
      if(string_equiv(*arguments,"-create_new_instance",
		      max(strlen(*arguments),2)))
	{
	  flag = 1;
          continue;
	}

      if(string_equiv(*arguments,"-instance",max(strlen(*arguments),2)))
	{
	   if((*(++arguments) != (char *) NULL) && (*arguments[0] != '-'))
	     {
	       if(isnumber(*arguments) != SUCCESS)
		 {
		   printf("Specified instance id \"%s\" is not a number.\n", 
			  *arguments);
		   return(ERROR);
		 }
	       Request.requester.instance = atoi(*arguments);
	       continue;
	     }
	   else
	     {
	       fprintf(stderr,
		       "You must specify an instance after '-instance'.\n");
	       return(ERROR);
	     }
	 }

      if(string_equiv(*arguments,"-do_not_change_instance",
		      max(strlen(*arguments),2)))
	{
	  hold = 1;
          continue;
	}

      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);
      if(arguments == (char **) NULL)   
	{
	  printf("Usage is: \tgrab  [<username> <instance id>] [-create_new_instance]\n\t\t[-do_not_change_instance] [-instance <instance id>]\n");
	  return(ERROR);
	}
      if(*arguments == (char *) NULL)   /* end of list */
        break;
    }

  status = t_grab(&Request,flag,hold);
  return(status);
}

/*
 * Function:	do_olc_forward() forwards the question to another consultant.
 * Arguments:	arguments:	The argument array from the parser.
 *		    arguments[1] may be "-unans(wered)" or "-off".
 * Returns:	An error code.
 * Notes:
 *	First, we send an OLC_FORWARD request to the daemon.  This should
 *	put the question back on the queue for another consultant to answer.
 *	If the first argument is "-unanswered", the question is forwarded
 *	directly tothe log as an unanswered question.  If the first
 *	argument is "-off", the consultant is signed off of OLC instead of
 *	being connected to another user.  These two arguments may not be
 *	used at the same time.
 */

ERRCODE
do_olc_forward(arguments)
     char **arguments;
{
  REQUEST Request;
  int status;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);
	
  for (arguments++; *arguments != (char *)NULL; ) 
    {
      if (string_equiv(*arguments, "-off", max(strlen(*arguments), 2)))
	{
	  set_option(Request.options, OFF_OPT);
	  arguments++;
	  continue;
	}

      if(string_equiv(*arguments, "-unanswered", max(strlen(*arguments), 2)))
	{
	  set_option(Request.options, FORWARD_UNANSWERED);
	  arguments++;
	  continue;
	}

      if(string_equiv(*arguments,"-status",max(strlen(*arguments),2)))
	{
	  arguments++;
	  status = t_input_status(&Request, *arguments);
	  if(status)
	    return(status);
	  if(*arguments != (char *) NULL)
	    arguments++;
	  continue;
	}

      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);
      if(arguments == (char **) NULL)   /* error */
	{
	  printf("Usage is: \tforward  [<username> <instance id>] ");
	  printf("[-status <status>]\n\t\t[-off] [-unanswered] ");
	  printf("[-instance <instance id>]\n");
	  return(ERROR);
	}
    }

  status = t_forward(&Request);
  return(status);
}



