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
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: p_ask.c,v 1.14 1999-01-22 23:12:42 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_ask.c,v 1.14 1999-01-22 23:12:42 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_parser.h>
#include <sys/param.h>

ERRCODE
do_olc_ask(arguments)
     char **arguments;
{
  REQUEST  Request;
  int status = 0;
  char topic[TOPIC_SIZE];
  char file[MAXPATHLEN];

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  topic[0]= '\0';
  file[0] = '\0';

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
		  continue;
                }
	      else
		break;
            }

	  if(string_equiv(*arguments, "-file",max(strlen(*arguments),2)))
	    {
	      arguments++;
	      if(*arguments != (char *) NULL) {
		(void) strcpy(file, *arguments);
		continue;
	      } else
		break;
	    }

	  arguments = handle_argument(arguments, &Request, &status);
	  if(status)
	    return(ERROR);
	  if(arguments == (char **) NULL) 
	    {
	      if(client_is_user_client())
		printf("Usage is: \task [-topic <topic>]\n");
	      else
		{
		  printf("Usage is: \task [-topic <topic>] ");
		  printf("[<username> <instance id>]\n");
		  printf("\t\t[-instance <instance id]>\n");
		  printf("\t\t[-file <filename>]\n");
		}
	      
	      return(ERROR);
	    }
	  if(*arguments == (char *) NULL)   /* end of list */
	    break;
        }
    }

  if(topic[0] == '\0')
    t_input_topic(&Request,topic,TRUE);

  status = t_ask(&Request,topic,file);
  if(client_is_user_client())
    {
      printf("\nSome other useful %s commands are: \n\n", client_service_name());
      printf("\tsend  - send a message\n");
      printf("\tshow  - show new messages\n");
      printf("\tdone  - mark your question resolved\n");
      printf("\tquit  - exit %s, leaving your question active\n",
	     client_service_name());
      if (client_has_hours())
	printf("\thours - Find hours %s is staffed\n", client_service_name());
      printf("\t?     - see entire listing of commands\n");
    }
  return(status);
}
