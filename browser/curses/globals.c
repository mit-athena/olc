/*
 *	Win Treese, Jeff Jimenez
 *      Student Consulting Staff
 *	MIT Project Athena
 *
 *	Copyright (c) 1985 by the Massachusetts Institute of Technology
 *
 *      Permission to use, copy, modify, and distribute this program
 *      for any purpose and without fee is hereby granted, provided
 *      that this copyright and permission notice appear on all copies
 *      and supporting documentation, the name of M.I.T. not be used
 *      in advertising or publicity pertaining to distribution of the
 *      program without specific prior permission, and notice be given
 *      in supporting documentation that copying and distribution is
 *      by permission of M.I.T.  M.I.T. makes no representations about
 *      the suitability of this software for any purpose.  It is pro-
 *      vided "as is" without express or implied warranty.
 */

/* This file is part of the CREF finder.  It contains the primary definitions
 * of the global variables.
 *
 *	$Source:
 *	$Author:
 *      $Header:
 */

#ifndef lint
static char *rcsid_globals_c = "$Header: ";
#endif	lint

#include <stdio.h>			/* Standard I/O definitions. */
#include <curses.h>			/* Curses package defs. */
#include <ctype.h>			/* Character type macros. */
#include <pwd.h>			/* Password file entry defs. */

#include "cref.h"			/* CREF definitions. */

/* Function declarations for the command table. */

extern ERRCODE	print_help();
extern ERRCODE	top();
extern ERRCODE	prev_entry();
extern ERRCODE	next_entry();
extern ERRCODE	up_level();
extern ERRCODE	save_to_file();
extern ERRCODE	next_page();
extern ERRCODE	prev_page();
extern ERRCODE	quit();
extern ERRCODE	goto_entry();
extern ERRCODE	add_abbrev();
extern ERRCODE	list_abbrevs();
extern ERRCODE	insert_entry();
extern ERRCODE	delete_entry();
extern ERRCODE  redisplay();

/* Command table. */

COMMAND Command_Table[] = {
        'r',    redisplay,      "Redisplay the screen.",
	'?',	print_help,	"Print help information.",
	'a',	add_abbrev,	"Add a new abbreviation.",
	'g',	goto_entry,	"Go to a specified entry.",
	'h',	print_help,	"Print help information.",
	'i',	insert_entry,	"Insert a new entry.",
	'l',	list_abbrevs,	"List all abbreviations.",
	'n',	next_entry,	"Go to the next entry.",
	'p',	prev_entry,	"Go to the previous entry.",
	'q',	quit,		"Quit CREF.",
	'd',	delete_entry,	"delete an entry from CREF.",
	's',	save_to_file,	"Save an entry to a file.",
	't',	top,		"Go to the top level.",
	'u',	up_level,	"Go up one level.",
	'+',	next_page,	"Display next page of index.",
	'-',	prev_page,	"Display previous page of index."
};

/* State Variables. */

char Current_Dir[FILENAME_SIZE];	/* Current CREF directory. */
char Root_Dir[FILENAME_SIZE];		/* CREF root directory. */
int Current_Index;			/* Current CREF entry. */
int Previous_Index;			/* Upper level CREF entry. */
ENTRY Entry_Table[MAX_ENTRIES];		/* Table of CREF entries. */
int Entry_Count;			/* Number of entries. */
int Index_Start;			/* Current top of index. */
int Command_Count;			/* Number of CREF commands. */
char Save_File[FILENAME_SIZE];		/* Default save file. */
char Abbrev_File[FILENAME_SIZE];	/* Abbreviation filename. */
ABBREV Abbrev_Table[MAX_ABBREVS];	/* Abbreviation table. */
int Abbrev_Count;			/* Number of abbreviations. */
char Log_File[FILENAME_SIZE];           /* Administrative log file. */

/* Function:	init_globals() initializes the global state variables.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 *	All global state variables (listed in globals.h) should be
 *	initialized to their defaults here.  Command line processing
 *	to change these should be done later.
 */

init_globals()
{
  struct passwd *user_info;		/* User password entry. */

  Command_Count = sizeof(Command_Table)/sizeof(COMMAND);
  strcpy(Root_Dir, CREF_ROOT);
  set_current_dir(Root_Dir);
  Current_Index = 1;
  Entry_Count = 0;
  Previous_Index = 0;
  Index_Start = 1;
  Abbrev_Count = 0;
  Log_File[0] = (char) NULL;
  strcpy(Save_File, "cref_info");

  user_info = getpwuid(getuid());
  strcpy(Abbrev_File, user_info->pw_dir);
  strcat(Abbrev_File, "/");
  strcat(Abbrev_File, USER_ABBREV);
}

