/*
 * Log Replayer Daemon
 *
 * This replays question logs
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 */

#ifndef lint
#ifndef SABER
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/rpd/handle_request.c,v 1.19 1993-05-14 18:23:12 vanharen Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "rpd.h"

#ifndef KERBEROS
/* Still need kerberos defs for compatabile protocols.. */

#define         MAX_KTXT_LEN    1250

struct ktext {
    int length;             /* Length of the text */
    unsigned char dat[MAX_KTXT_LEN];    /* The data itself */
    unsigned long mbz;          /* zero to catch runaway strings */
};

typedef struct ktext *KTEXT;
typedef struct ktext KTEXT_ST;
#endif

void
handle_request(fd, from)
     int fd;
     struct sockaddr_in from;
{

  int len;
  char username[9],tusername[9];
  int instance,tinstance;
  int version;
  int request;
  u_long output_len;
  char *buf;
  int result;
  char *from_addr;

  int ltr;
  KTEXT_ST their_auth;
#ifdef KERBEROS
  AUTH_DAT their_info;
  int auth;
  char principal_buffer[ANAME_SZ+INST_SZ+REALM_SZ];
  static char instance_buffer[INST_SZ];

  if (instance_buffer[0] == '\0')
    instance_buffer[0] = '*';
#endif /* KERBEROS */

  from_addr = inet_ntoa(from.sin_addr);

  if ((len = sread(fd,&version,sizeof(version))) != sizeof(version)) {
    if (len == -1)
      syslog(LOG_ERR, "(%s) reading version: %m", from_addr);
    else
      syslog(LOG_WARNING, "(%s) Not enough bytes for version (%d rcvd)",
	     from_addr, len);
    punt_connection(fd,from);
    return;
  }

  version = ntohl(version);
  if (version > VERSION) {
    syslog(LOG_WARNING, "(%s) Version skew - curr:%d, rcvd:%d\n",
	    from_addr, VERSION, version);
    punt_connection(fd,from);
    return;
  }

  if (version >= 1) {
    if ((len = sread(fd,&request,sizeof(request))) != sizeof(request)) {
      if (len == -1)
	syslog(LOG_ERR, "(%s) reading request: %m", from_addr);
      else
	syslog(LOG_WARNING, "(%s) Not enough bytes for request (%d rcvd)",
	       from_addr, len);
      punt_connection(fd,from);
      return;
    }
    request = ntohl(request);
  }
  else
    request = LIST_REQ;

  if ((len = sread(fd,username,9)) != 9) {
    if (len == -1)
      syslog(LOG_ERR, "(%s) reading username: %m", from_addr);
    else
      syslog(LOG_WARNING, "(%s) Wanted 9 bytes of username, rcvd %d\n",
	     from_addr, len);
    punt_connection(fd,from);
    return;
  }

  if ((len = sread(fd,&instance,sizeof(instance))) != sizeof(instance)) {
    if (len == -1)
      syslog(LOG_ERR, "(%s) reading instance: %m", from_addr);
    else
      syslog(LOG_WARNING, "(%s) Not enough bytes for instance (rcvd %d)\n",
	     from_addr, len);
    punt_connection(fd,from);
    return;
  }

  instance = ntohl(instance);

  if (request == REPLAY_KILL_REQ) {
    /* both of these take a target username */
    if ((len = sread(fd,tusername,9)) != 9) {
      if (len == -1)
	syslog(LOG_ERR, "(%s) reading tusername: %m",
	       from_addr);
      else
	syslog(LOG_WARNING, "(%s) Wanted nine bytes of tusername, rcvd %d\n",
	       from_addr, len);
      punt_connection(fd,from);
      return;
    }

    if ((len = sread(fd,&tinstance,sizeof(tinstance))) != sizeof(tinstance)) {
      if (len == -1)
	syslog(LOG_ERR, "(%s) reading tinstance: %m",
	       from_addr);
      else
	syslog(LOG_WARNING, "(%s) Not enough bytes for tinstance (rcvd %d)\n",
	       from_addr, len);
      punt_connection(fd,from);
      return;
    }

    tinstance = ntohl(tinstance);
  }

  
  if ((len = sread(fd,&their_auth.length, sizeof(their_auth.length))) !=
      sizeof(their_auth.length)) {
    if (len == -1)
      syslog(LOG_ERR, "(%s) reading kticket length: %m",
	     from_addr);
    else
      syslog(LOG_WARNING, "(%s) Not enough bytes for ticket (rcvd %d)\n",
	     from_addr, len);
    punt_connection(fd,from);
    return;
  }

  their_auth.length = ntohl(their_auth.length);

  if (their_auth.length != 0) {
    bzero(their_auth.dat,sizeof(their_auth.dat));
    ltr =MIN(sizeof(unsigned char)*their_auth.length,
	     sizeof(their_auth.dat));
    if ((len = sread(fd,their_auth.dat,ltr)) != ltr) {
      if (len == -1)
	syslog(LOG_ERR, "(%s) reading kticket: %m", from_addr);
      else
	syslog(LOG_WARNING, "(%s) Error reading kerberos ticket (rcvd %d)\n",
	       from_addr, len);
      punt_connection(fd,from);
      return;
    }
  }

#ifdef KERBEROS
  auth = krb_rd_req(&their_auth,K_SERVICE,instance_buffer,
		    (unsigned long) from.sin_addr.s_addr,&their_info,SRVTAB);
  if (auth != RD_AP_OK) {
    /* Twit! */
    syslog(LOG_WARNING, "(%s) Kerberos error: %s\n",
	   from_addr,
	   krb_err_txt[auth]);
    output_len = htonl(-auth);
    write(fd,&output_len,sizeof(long));
    punt_connection(fd,from);
    return;
  }

  sprintf(principal_buffer,"%s.%s@%s",their_info.pname, their_info.pinst,
	  their_info.prealm);
  if ((strcmp(their_info.pname,username) != 0) &&
      !acl_check(MONITOR_ACL,principal_buffer)) {
    /* Twit! */
    syslog(LOG_WARNING, "(%s) Request from %s who is not on the acl\n",
	   from_addr,
	   principal_buffer);
    output_len = htonl(ERR_NO_ACL);
    write(fd,&output_len,sizeof(long));
    punt_connection(fd,from);
    return;
  }

  syslog(LOG_DEBUG, "(%s) %s replays %s [%d]", from_addr,
	 principal_buffer, username, instance);

  if (((request == SHOW_KILL_REQ) && strcmp(their_info.pname,username)) ||
      ((request == REPLAY_KILL_REQ) && strcmp(their_info.pname,tusername))) {
    syslog(LOG_WARNING, "(%s) Request to delete %s's new messages from %s\n",
	   from_addr,
	   username, principal_buffer);
    output_len = htonl(ERR_OTHER_SHOW);
    write(fd,&output_len,sizeof(long));
    punt_connection(fd,from);
    return;
  }

#endif /* KERBEROS */

  switch(request) {
  case SHOW_KILL_REQ:
    buf = get_nm(username,instance,&result,1);
    break;
  case SHOW_NO_KILL_REQ:
    buf = get_nm(username,instance,&result,0);
    break;
  case LIST_REQ:
    buf = get_log(username,instance,&result);
    break;
  case REPLAY_KILL_REQ:
    buf = get_nm(tusername,tinstance,&result,1);
    if ((buf == NULL) && (result != 0))
      break;
    buf = get_log(username,instance,&result);
    break;
  default:
    /* Sorry, not here- */
    output_len = htonl(ERR_NOT_HERE);
    write(fd,&output_len,sizeof(u_long));
    return;
  }
  if (buf == NULL) {
    /* Didn't get response; determine if it's an error or simply that the */
    /* question just doesn't exist based on result */
    if (result == 0)
      output_len = htonl(ERR_NO_SUCH_Q);
    else
      output_len = htonl(ERR_SERV);
    write(fd,&output_len,sizeof(u_long));
  }
  else {
    /* All systems go, write response */
    output_len = htonl((u_long)result);
    write(fd,&output_len,sizeof(u_long));
    write(fd,buf,result);
  }
}

void
punt_connection(fd, from)
     int fd;
     struct sockaddr_in from;
{
  close(fd);
/*
  syslog(LOG_INFO, "Punted connection from %s\n", inet_ntoa(from.sin_addr));
*/
  return;
}
