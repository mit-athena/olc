/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains definitions used by "common.a".
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/common.h,v $
 *	$Id: common.h,v 1.9 1991-04-08 21:04:02 lwvanels Exp $
 *	$Author: lwvanels $
 */

#include <mit-copyright.h>

#ifndef __olc_common_h
#define __olc_common_h

#include <olc/lang.h>
#ifdef m68k
#include <time.h>
#endif

typedef struct tSTATUS
{
  int status;
  char label[TITLE_SIZE];
} STATUS;

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

/* io.c */
int send_dbinfo P((int fd , DBINFO *dbinfo ));
int read_dbinfo P((int fd , DBINFO *dbinfo ));
ERRCODE send_response P((int fd , RESPONSE response ));
ERRCODE read_response P((int fd , RESPONSE *response ));
ERRCODE write_int_to_fd P((int fd , int response ));
ERRCODE read_int_from_fd P((int fd , int *response ));
ERRCODE read_text_into_file P((int fd , char *filename ));
ERRCODE write_file_to_fd P((int fd , char *filename ));
ERRCODE write_text_to_fd P((int fd , char *buf ));
char *read_text_from_fd P((int fd ));
int sread P((int fd , char *buf , int nbytes ));
int swrite P((int fd , char *buf , int nbytes ));

/* perror.c */
char *format_time P((char *time_buf , struct tm *time_info ));
void time_now P((char *time_buf ));
void olc_perror P((char *msg ));

/* status.c */
int OGetStatusString P((int status , char *string ));
int OGetStatusCode P((char *string , int *status ));

/* string_utils.c */
void uncase P((char *string ));
void upcase_string P((char *string ));
char *cap P((char *string ));
int isnumber P((char *string ));
char *get_next_word P((char *line , char *buf , int (*func )(int c)));
int IsAlpha P((int c ));
int NotWhiteSpace P((int c ));
void make_temp_name P((char *name ));

#undef P

#endif /* __olc_common_h */
