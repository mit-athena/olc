/*
 * Log Replayer Daemon
 *
 * This replays question logs
 */

#ifndef lint
#ifndef SABER
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/rpd/handle_request.c,v 1.7 1990-12-11 09:14:31 lwvanels Exp $";
#endif
#endif

#include "rpd.h"

void
handle_request(fd, from)
     int fd;
     struct sockaddr_in from;
{

  int len;
  char username[9];
  int instance;
  int version;
  long output_len;
  char *buf;
  int result;

#ifdef KERBEROS
  KTEXT_ST their_auth;
  AUTH_DAT their_info;
  int ltr;
  int auth;
  char principal_buffer[ANAME_SZ+INST_SZ+REALM_SZ];
  static char instance_buffer[INST_SZ];
#endif /* KERBEROS */

  if (instance_buffer[0] == '\0')
    instance_buffer[0] = '*';

  if ((len = sread(fd,&version,sizeof(version))) != sizeof(version)) {
    if (len == -1)
      syslog(LOG_ERR,"reading version: %m");
    else
      syslog(LOG_WARNING,"Not enough bytes for version (%d received)",len);
    punt_connection(fd,from);
    return;
  }

  version = ntohl(version);
  if (version != VERSION) {
    syslog(LOG_WARNING,"Version skew from %s\n curr = %d, recvd = %d\n",
	    inet_ntoa(from.sin_addr),VERSION,version);
    punt_connection(fd,from);
    return;
  }

  if ((len = sread(fd,username,9)) != 9) {
    if (len == -1)
      syslog(LOG_ERR,"reading username: %m");
    else
      syslog(LOG_WARNING,"Wanted nine bytes of username, only got %d\n",len);
    punt_connection(fd,from);
    return;
  }

  if ((len = sread(fd,&instance,sizeof(instance))) != sizeof(instance)) {
    if (len == -1)
      syslog(LOG_ERR,"reading instance: %m");
    else
      syslog(LOG_WARNING,"Not enough bytes for instance (%d)\n",len);
    punt_connection(fd,from);
    return;
  }

  instance = ntohl(instance);

#ifdef KERBEROS

  if ((len = sread(fd,&their_auth.length, sizeof(their_auth.length))) !=
      sizeof(their_auth.length)) {
    if (len == -1)
      syslog(LOG_ERR,"reading kticket length: %m");
    else
      syslog(LOG_WARNING,"Not enought bytes for ticket (%d)\n",len);
    punt_connection(fd,from);
    return;
  }

  their_auth.length = ntohl(their_auth.length);
  bzero(their_auth.dat,sizeof(their_auth.dat));
  ltr =MIN(sizeof(unsigned char)*their_auth.length,
	   sizeof(their_auth.dat));
  if ((len = sread(fd,their_auth.dat,ltr)) != ltr) {
    if (len == -1)
      syslog(LOG_ERR,"reading kticket: %m");
    else
      syslog(LOG_WARNING,"Error reading kerberos ticket (%d)\n",len);
    punt_connection(fd,from);
    return;
  }
  auth = krb_rd_req(&their_auth,K_SERVICE,instance_buffer,
		    (unsigned long) from.sin_addr.s_addr,&their_info,"");
  if (auth != RD_AP_OK) {
    /* Twit! */
    syslog(LOG_WARNING,"Kerberos error: %s\n from %s",krb_err_txt[auth],
	   inet_ntoa(from.sin_addr));
    output_len = htonl(-auth);
    write(fd,&output_len,sizeof(long));
    punt_connection(fd,from);
    return;
  }

  sprintf(principal_buffer,"%s.%s@%s",their_info.pname, their_info.pinst,
	  their_info.prealm);
  if (!acl_check(MONITOR_ACL,principal_buffer)) {
    /* Twit! */
    syslog(LOG_WARNING,"Request from %s@%s who is not on the acl\n",
	    principal_buffer, inet_ntoa(from.sin_addr));
    output_len = htonl(-13);
    write(fd,&output_len,sizeof(long));
    punt_connection(fd,from);
    return;
  }

  syslog(LOG_DEBUG,"%s replays %s [%d]",principal_buffer, username,
	 instance);


#endif /* KERBEROS */

  buf = get_log(username,instance,&result);
  if (buf == NULL) {
    /* Didn't get response; determine if it's an error or simply that the */
    /* question just doesn't exist based on result */
    if (result == 0)
      output_len = htonl(-11L);
    else
      output_len = htonl(-12L);
    write(fd,&output_len,sizeof(long));
  }
  else {
    /* All systems go, write response */
    output_len = htonl((long)result);
    write(fd,&output_len,sizeof(long));
    write(fd,buf,result);
  }
}

void
punt_connection(fd, from)
     int fd;
     struct sockaddr_in from;
{
  shutdown(fd,2);   /* Not going to send or receive from this guy again.. */
  close(fd);
  syslog(LOG_INFO,"Punted connection from %s\n",inet_ntoa(from.sin_addr));
  return;
}
