/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains routines for manipulating the OLC database.
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
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/db.c,v $
 *	$Id: db.c,v 1.11 1990-12-05 21:19:11 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/db.c,v 1.11 1990-12-05 21:19:11 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <stdio.h>
#include <sys/file.h>
#include <ctype.h>		/* Standard type definitions. */
#include <olcd.h>

extern ACL  Acl_List[];

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

static int get_user_info P((USER *user ));

#undef P


/*
 * Function:	get_specialties() searches the OLC database to find a
 *			user's list of specialties.
 * Arguments:	user:	Ptr. to desired user
 * Returns:	nothing
 * Notes:
 */


get_specialties(user)
     USER *user;
{
  TOPIC **t_ptr;
  int *s = user->specialties;
  int i =0;
  char buf[BUFSIZ];

  sprintf(buf,"%s@%s",user->username,user->realm);
  for(t_ptr = Topic_List; *t_ptr != (TOPIC *) NULL; t_ptr++)
    {
      if(acl_check((*t_ptr)->acl,buf))
	{
	  *(s+i) = (*t_ptr)->value;	
	  ++i;
	}

      if(i==SPEC_SIZE)
	break;
    }
  *(s+i) = UNKNOWN_TOPIC;
  user->no_specialties = i;
}


void
get_acls(user)
     USER *user;
{
  ACL *a_ptr;
  char buf[BUFSIZ];

  sprintf(buf,"%s@%s",user->username,user->realm);

  for(a_ptr = Acl_List; a_ptr->code > 0; a_ptr++)
    {
      if(acl_check(a_ptr->file,buf))
	user->permissions |= a_ptr->code;
    }
}


int
load_db()
{
  FILE *fp;
  TOPIC *t;
  ACL *a;
  char *db;
  char db_line[DB_LINE];
  char msgbuf[BUFSIZ];
  char buf[BUFSIZ];
  int i=0;

  if((fp = fopen(TOPIC_FILE,"r")) == (FILE *) NULL)
    {
      (void) sprintf(msgbuf, "load_db: can't open OLC database %s", 
		     TOPIC_FILE);
      log_error(msgbuf);
      perror("load_db");
      return(ERROR);
    }

  while (fgets(db_line, DB_LINE, fp) != (char *)NULL)
    {
      db = db_line;
      if(*db == '#')       /* comment */
	continue;
      t = (TOPIC *) malloc(sizeof(TOPIC));
      if(t == (TOPIC *) NULL)
	{
	  perror("load_db: topic malloc:");
	  fclose(fp);
	  return(ERROR);
	}
      db = get_next_word(db, t->name, NotWhiteSpace);
      sprintf(t->acl,"%s/%s.acl",SPECIALTY_DIR,t->name);
      t->value = i;
      if (insert_topic(t) != SUCCESS) {
	perror("load_db: insert_topic:");
	fclose(fp);
	return(ERROR);
      }

#ifdef TEST
      log_status (fmt ("load_db: %s %d %s\n",t->name,i, t->acl));
#endif /* TEST */
      ++i;
    } 

  
  for(a = Acl_List; a->file != (char *) NULL; a++)
    {
      sprintf(buf,"%s/%s",ACL_DIR,a->file);
      a->file = malloc((strlen(buf)+1) * sizeof(char));
      strcpy(a->file,buf);
    }

  fclose(fp);
  return(SUCCESS);
}

void
load_user(user)
     USER *user;
{
  get_specialties(user);
  get_acls(user);
  get_user_info(user);
}

static int
get_user_info(user)
     USER *user;
{
  FILE *fp;
  char db_line[DB_LINE];  
  char msgbuf[BUFSIZ];
  char *db, buf[BUFSIZ];            
  char canon[BUFSIZ];

  sprintf(canon,"%s@%s",user->username,user->realm);

  if((fp = fopen(DATABASE_FILE,"r")) == (FILE *) NULL)
    {
      (void) sprintf(msgbuf, "load_user: can't open OLC database %s", 
		     DATABASE_FILE);
      log_error(msgbuf);
      perror("load_user");
      return(ERROR);
    }

  while (fgets(db_line, DB_LINE, fp) != (char *) NULL)
    {
      db = db_line;
      if(*db_line == '#')       /* comment */
	continue;
      else
	{
	  db = get_next_word(db, buf, NotWhiteSpace);
	  if(string_eq(canon,buf))
	    {
	      db = get_next_word(db, user->title1, NotWhiteSpace);
	      db = get_next_word(db, buf, NotWhiteSpace);
	      user->max_ask = atoi(buf);
	      db = get_next_word(db, user->title2, NotWhiteSpace);
	      db = get_next_word(db, buf, NotWhiteSpace);
	      user->max_answer = atoi(buf);
#ifdef TEST
	      printf("%s %s %d %s %d\n",canon, user->title1, user->max_ask,
		user->title2,user->max_answer);
#endif /* TEST */
	      fclose(fp);
	      return(SUCCESS);
	    }
	  
	}
    }
  fclose(fp);
  return(ERROR);
}


int
save_user_info(user)
     USER *user;
{
  return(ERROR);
}
