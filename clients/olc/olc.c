/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains the main routine of the user program, "olc".
 *
 *      Win Treese
 *      Dan Morgan
 *      Bill Saphir
 *      MIT Project Athena
 *
 *      Ken Raeburn
 *      MIT Information Systems
 *
 *      Steve Dyer
 *      IBM/MIT Project Athena
 *      converted to use Hesiod in place of clustertab
 *
 *      Tom Coppeto
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/olc/olc.c,v $
 *	$Id: olc.c,v 1.36 1994-08-21 18:16:58 cfields Exp $
 *	$Author: cfields $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/olc/olc.c,v 1.36 1994-08-21 18:16:58 cfields Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_parser.h>
#include <olc/olc_tty.h>

#include <signal.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <strings.h>

#ifdef KERBEROS
extern int krb_ap_req_debug;
#endif /* KERBEROS */


/*
 * OLC command table.  Each line is an entry for a function that OLC will
 * perform.  The syntax is:
 *
 *     "command name", function-name, "description"
 *
 *  "command name" is the name of the function as the user would type it.
 *  <function name> is the name of the function to be executed when the user
 *	types the name.
 *  "description" is a short descriptive phrase about what the command does
 *
 * Both of the function names above serve as pointers to the functions they
 * name.  They should be declared above as "extern" so the compiler knows
 * what to do.
 */

COMMAND OLC_Command_Table[] = {
  "?",		do_olc_list_cmds,	"List available commands",
  "help",	do_olc_help,		"Describe the various commands.",
#ifndef OLTA
  "answers",	do_olc_stock,		"Read answers to common questions",
#endif
  "ask",	do_olc_ask,		"Ask a question",
  "cancel",	do_olc_cancel,		"Cancel your question",
  "done",	do_olc_done,		"Mark your question resolved",
  "exit",	do_quit,		"Temporarily exit",
  "hours",      do_olc_hours,		"Print the staffed hours",
  "motd",	do_olc_motd,		"See message of the day",
  "quit",	do_quit,		"Temporarily exit",
  "replay",	do_olc_replay,		"Replay the conversation so far",
  "send",	do_olc_send,		"Send a message",
  "show",	do_olc_show,		"Show any new messages",
  "status",	do_olc_status,		"Print your status",
  "topic",	do_olc_topic,		"Find question topic",
  "version",	do_olc_version,		"Print version number",
  "who",	do_olc_who,		"Find out who you're connected to",
  (char *) NULL, (ERRCODE (*)()) NULL,	""
  };
  
COMMAND OLCR_Command_Table[] = {
  "?",		do_olc_list_cmds,	"List available commands",
  "help",	do_olc_help,		"Describe the various commands",
  "acl",	do_olc_acl,		"Display/Change accesses.",
#ifndef OLTA
  "answers",	do_olc_stock,		"Read answers to common questions",
#endif
  "ask",	do_olc_ask,		"Ask a question",
  "cancel",	do_olc_cancel,		"Cancel a question",
  "comment",	do_olc_comment, 	"Make a comment",
  "dbinfo",	do_olc_dbinfo,		"Display database info.",
  "dbload",	do_olc_load_user,	"Reload user attributes.",
  "describe",	do_olc_describe,	"Show/Change summary info",
  "done",	do_olc_done,		"Resolve question",	
  "exit",	do_quit,		"Quit",
  "forward",	do_olc_forward,		"Forward a question",
  "grab",	do_olc_grab,		"Grab a user",
  "hours",      do_olc_hours,		"Print staffed hours",
  "instance",	do_olc_instance,	"Show/Change default instance",
  "list",	do_olc_list,		"List the world",
  "mail",	do_olc_mail,		"Mail a message",
  "motd",	do_olc_motd,		"See motd",
  "off",	do_olc_off,		"Sign off",
  "on",		do_olc_on,		"Sign on",
  "quit",	do_quit,		"Quit",
  "replay",	do_olc_replay,		"Replay the conversation",	   
  "send",	do_olc_send,		"Send a message",		       
  "show",	do_olc_show,		"Show any new messages",
  "status",	do_olc_status,		"Find your status",
#ifndef OLTA
  "stock",	do_olc_stock,		"Browse thru stock answers",
#endif
  "topic",	do_olc_topic,		"Show/Change question topic",
  "version",	do_olc_version,		"Print version number",
  "who",	do_olc_who,		"Find status for current instance",
#ifdef ZEPHYR
  "zephyr",	do_olc_zephyr,		"Toggle daemon zephyr use",
#endif
  (char *) NULL, (ERRCODE(*)()) NULL,	""
  };


COMMAND *Command_Table;
char *program;
int OLC=0;
char *HELP_FILE;
char *HELP_EXT;
char *HELP_DIR;

int select_timeout = 300;

/*
 * Function:	main() is the startup for OLC.  It initializes the
 *			environment and starts the command interpreter.
 * Arguments:	argc:	Number of arguments in the command line.
 *		argv:	Array of words from the command line.
 *
 * Returns:	Nothing.
 * Notes:
 *	First, check the terminal mode to make sure that the program is
 *	interactive and that the user can receive 'write' messages from
 *	the daemon.  Then look up the necessary information about how to
 *	find the daemon in the Hesiod database.  Next, we get general
 *	information about the user, including some from her /etc/passwd
 *	entry.  The relevant information is copied into an OLC "person"
 *	structure, and the OLC request structure is initialized.  From the
 *	password entry we take the username and the real name, excluding
 *	any office or phone information.  Also, we construct the user
 *	identification string, which has the format "realname (user@machine)".
 *	We then	call olc_init() to complete the initialization and then invoke
 *	the command interpreter.
 */

main(argc, argv)
     int argc;
     char **argv;
{
  char *prompt;
#ifdef PUTENV
  char buf[BUFSIZ];
#endif
  ERRCODE status;
#ifdef POSIX
  struct sigaction act;
#endif

/*
 * All client specific stuff should be initialized here, if they wish
 * to play after dinner.
 */

  program = rindex(*argv,'/');
  if(program == (char *) NULL)
     program = *argv;
  if(*program == '/')
     ++program;

  if(string_eq(program,"olc"))
    {
      Command_Table = OLC_Command_Table;
      prompt=OLC_PROMPT;
      HELP_FILE = OLC_HELP_FILE;
      HELP_DIR =  OLC_HELP_DIR;
      HELP_EXT =  OLC_HELP_EXT;
      OLC=1;
    }
  else
    {       
      Command_Table = OLCR_Command_Table;
      prompt=OLCR_PROMPT;
      HELP_FILE = OLCR_HELP_FILE;
      HELP_DIR =  OLCR_HELP_DIR;
      HELP_EXT =  OLCR_HELP_EXT;
    }

  ++argv, --argc;
  while (argc > 0 && argv[0][0] == '-') {
      if (!strcmp (argv[0], "-prompt")) {
	  prompt = argv[1];
	  ++argv, --argc;
      }
      else if (!strcmp (argv[0], "-server")) {
	  /*
	   * this is a kludge, but the other interface is already
	   * there
	   */
#ifdef PUTENV
	  sprintf(buf,"OLCD_HOST=%s",argv[1]);
	  putenv(buf);
#else
	  (void) setenv ("OLCD_HOST", argv[1], 1);
#endif
	  ++argv, --argc;
      }
      else if (!strcmp (argv[0], "-port")) {
#ifdef PUTENV
	  sprintf(buf,"OLCD_PORT=%s",argv[1]);
	  putenv(buf);
#else
	  (void) setenv ("OLCD_PORT", argv[1], 1);
#endif
	  ++argv, --argc;
      }
      else if (!strcmp (argv[0], "-inst")) {
#ifdef PUTENV
	sprintf(buf,"OLCD_INST=%s",argv[1]);
	putenv(buf);
#else
	(void) setenv("OLCD_INST", argv[1], 1);
#endif
	++argv, --argc;
      }
      else {
	  fprintf (stderr, "%s: unknown control argument %s\n",
		   program, argv[0]);
	  exit (1);
      }
      ++argv;
      --argc;
  }

#ifdef POSIX
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler= (void (*)()) SIG_IGN;
  (void) sigaction(SIGPIPE, &act, NULL);
#else
  signal(SIGPIPE, SIG_IGN);
#endif
  if (argc)
    {
      OInitialize();
      status = do_command(Command_Table, argv);
      exit(status);
    }
  else 
    {
      (void) do_olc_init();
      command_loop(Command_Table, prompt);
    }
}


/*
 * Function:	olc_init() completes the initialization process for
 *			the user program.
 * Arguments:	None.
 * Returns:	an error code (which is always 0).
 * Notes:
 *	First, find out if the user has a current question.  If not,
 *	prompt for topic and question.  Next, send an OLC_STARTUP
 *	request to the daemon, either starting a new conversation or
 *	continuing an old one.  A message notifying the user of the
 *	status of the question is then printed.  Finally, we return
 *	to the OLC main loop.
 */

ERRCODE
do_olc_init() 
{
  int fd;			/* File descriptor for socket. */
  REQUEST Request;
  ERRCODE errcode=0;	        /* Error code to return. */
  int n,first=0;
  char file[NAME_SIZE];
  char topic[TOPIC_SIZE];
  int status;

  OInitialize();

  fill_request(&Request);
  Request.request_type = OLC_STARTUP;

  status = open_connection_to_daemon(&Request, &fd);
  if(status)
    {
      handle_response(status, &Request);
      exit(ERROR);
    }
  status = send_request(fd, &Request);
  if(status)
    {
      handle_response(status, &Request);
      exit(ERROR);
    }
  read_response(fd, &status);

  switch(status) 
    {
    case USER_NOT_FOUND:
      printf("Welcome to %s, ",OLC_SERVICE_NAME);
      printf("Project Athena's On-Line Consulting system. (v %s)\n",
	     VERSION_STRING);
      printf("Copyright (c) 1989 by ");
      printf("the Massachusetts Institute of Technology.\n\n");
      first = 1;
	
      break;
         
    case CONNECTED:
    case SUCCESS:
      read_int_from_fd(fd, &n);
      printf("Welcome back to %s. ",OLC_SERVICE_NAME);
      printf("Project Athena's On-Line Consulting system. (v %s)\n",
	     VERSION_STRING);
      printf("Copyright (c) 1989 by ");
      printf("the Massachusetts Institute of Technology.\n\n");

      if(t_set_default_instance(&Request) != SUCCESS)
	fprintf(stderr,"Error setting default instance... continuing.\n");

      if(t_personal_status(&Request,FALSE) != SUCCESS)
	fprintf(stderr,"Error getting status... continuing.\n");

      break; 
   case PERMISSION_DENIED:
      printf("You are not allowed to use %s.\n",OLC_SERVICE_NAME);
      exit(1);
    default:
      if(handle_response(status, &Request) == ERROR)
	exit(ERROR);
    }

  (void) close(fd);

  make_temp_name(file);
  Request.request_type = OLC_MOTD;
  t_get_file(&Request,OLC,file,FALSE);
  unlink(file);

  if(OLC)
    {
      printf("\nTo see answers to common questions, type:      answers\n");
      printf("To see the hours %s is staffed, type:         hours\n",
	     OLC_SERVICE_NAME);
      if (first)
	{
	  topic[0]='\0';
	  printf("To ask a question, type:          ask\n");
	}
    }

  return(errcode);
}
