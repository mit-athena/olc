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
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_ask.c,v $
 *	$Id: p_ask.c,v 1.7 1990-05-26 12:10:28 vanharen Exp $
 *	$Author: vanharen $
 */

#ifndef lint
static const char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_ask.c,v 1.7 1990-05-26 12:10:28 vanharen Exp $";
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_parser.h>

ERRCODE
do_olc_ask(arguments)
     char **arguments;
{
  REQUEST  Request;
  int status = 0;
  char topic[TOPIC_SIZE];

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  topic[0]= '\0';

  if(arguments != (char **) NULL)
    {
      for (arguments++; *arguments != (char *) NULL; arguments++)
        {
          if(string_equiv(*arguments, "-topic", max(strlen(*arguments),2)))
            {
	      ++arguments;
              if(*arguments != (char *) NULL)
                {
                  (void) strcpy(topic,*arguments);
                  status = 1;
                }
	      else
		break;
            }
          else
            {
              arguments = handle_argument(arguments, &Request, &status);
	      if(status)
		return(ERROR);
              if(arguments == (char **) NULL) 
		{
		  if(OLC)
		    printf("Usage is: \task [-topic <topic>]\n");
		  else
		    {
		      printf("Usage is: \task [-topic <topic>] ");
		      printf("[<username> <instance id>]\n");
		      printf("\t\t[-instance <instance id]>\n");
		    }
		  
		  return(ERROR);
		}
              if(*arguments == (char *) NULL)   /* end of list */
                break;
            }
        }
    }

  if(!status)
    t_input_topic(&Request,topic,TRUE);

  status = t_ask(&Request,topic);
  if(OLC)
    {
      printf("\nSome other useful OLC commands are: \n\n");
      printf("\tsend  - send a message\n");
      printf("\tshow  - show new messages\n");
      printf("\tdone  - mark your question resolved\n");
      printf("\t?     - see entire listing of commands\n");
    }
  return(status);
}
