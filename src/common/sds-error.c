/**
 * *** Copyright Notice ***
 * SDS - Scientific Data Services framework, Copyright (c) 2015, The Regents of the University of California, through Lawrence Berkeley National Laboratory (subject to receipt of any required approvals from the U.S. Dept. of Energy).  All rights reserved.
 * If you have questions about your rights to use or distribute this software, 
 * please contact Berkeley Lab's Technology Transfer Department at TTD@lbl.gov.
 * 
 * NOTICE.  This software was developed under funding from the 
 * U.S. Department of Energy.  As such, the U.S. Government has been granted 
 * for itself and others acting on its behalf a paid-up, nonexclusive, 
 * irrevocable, worldwide license in the Software to reproduce, prepare 
 * derivative works, and perform publicly and display publicly.  
 * Beginning five (5) years after the date permission to assert copyright is 
 * obtained from the U.S. Department of Energy, and subject to any subsequent 
 * five (5) year renewals, the U.S. Government is granted for itself and others
 * acting on its behalf a paid-up, nonexclusive, irrevocable, worldwide license
 * in the Software to reproduce, prepare derivative works, distribute copies to
 * the public, perform publicly and display publicly, and to permit others to
 * do so.
 *
*/

/**
 *
 * Email questions to {dbin, sbyna, kwu}@lbl.gov
 * Scientific Data Management Research Group
 * Lawrence Berkeley National Laboratory
 *
*/
#include <errno.h>      
#include <stdarg.h>   
#include <stdlib.h>  
#include <stdio.h>
#include <string.h>
#include <syslog.h>

#define MAXLINE 4096

static void err_doit(int, int, const char *, va_list);
static void log_doit(int, int, const char *, va_list ap);

/*
 * Nonfatal error related to a system call
 * Print a message and return.
 */
void err_ret(const char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  err_doit(1, errno, fmt, ap);
  va_end(ap);
}

/*
 * Fatal error related to a system call.
 * Print a message and terminate.
 */
void err_sys(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  err_doit(1, errno, fmt, ap);
  va_end(ap);
  exit(1);
}

/*
 * Fatal error unrelated to a system call.
 * Error code passed as an explicit parameter. * Print a message and terminate.
 */
void
err_exit(int error, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  err_doit(1, error, fmt, ap);
  va_end(ap);
  exit(1);
}

/*
 * Fatal error related to a system call.
 * Print a message, dump core, and terminate. */
void err_dump(const char *fmt, ...)
{
  va_list     ap;
  va_start(ap, fmt);
  err_doit(1, errno, fmt, ap);
  va_end(ap);
  abort();        /* dump core and terminate */
  exit(1);        /* shouldn't get here */
}

/*
 * Nonfatal error unrelated to a system call. * Print a message and return.
 */
void err_msg(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  err_doit(0, 0, fmt, ap);
  va_end(ap);
}

/*
 * Fatal error unrelated to a system call. * Print a message and terminate.
 */
void err_quit(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  err_doit(0, 0, fmt, ap);
  va_end(ap);
  exit(1);
}

/*
 * Print a message and return to caller.
 * Caller specifies "errnoflag".
 */
static void err_doit(int errnoflag, int error, const char *fmt, va_list ap) {
  char buf[MAXLINE]; 
  vsnprintf(buf, MAXLINE, fmt, ap); 
  if (errnoflag)
    snprintf(buf+strlen(buf), MAXLINE-strlen(buf), ": %s", strerror(error));
  strcat(buf, "\n");
  fflush(stdout); /* in case stdout and stderr are the same */ 
  fputs(buf, stderr);
  fflush(NULL); /* flushes all stdio output streams */
}




/*
 * Caller must define and set this: nonzero if * interactive, zero if daemon
 */
//extern int log_to_stderr;

/*
 * Initialize syslog(), if running as daemon.
 */
void log_open(const char *ident, int option, int facility) {
  //if (log_to_stderr == 0) 
  openlog(ident, option, facility);
}

/*
* Nonfatal error related to a system call.
* Print a message with the system's errno value and return. */
void log_ret(const char *fmt, ...)
{
  va_list     ap;
  va_start(ap, fmt);
  log_doit(1, LOG_ERR, fmt, ap);
  va_end(ap);
}

/*
 * Fatal error related to a system call.
 * Print a message and terminate.
 */
void log_sys(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  log_doit(1, LOG_ERR, fmt, ap);
  va_end(ap);
  exit(2);
}

/*
* Nonfatal error unrelated to a system call. * Print a message and return.
*/
void log_msg(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  log_doit(0, LOG_ERR, fmt, ap);
  va_end(ap);
}

/*
* Fatal error unrelated to a system call. * Print a message and terminate.
*/
void log_quit(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  log_doit(0, LOG_ERR, fmt, ap);
  va_end(ap);
  exit(2);
}

/*
 * Print a message and return to caller.
* Caller specifies "errnoflag" and "priority". */
static void log_doit(int errnoflag, int priority, const char *fmt, va_list ap) {
  int     errno_save;
  char    buf[MAXLINE];
  errno_save = errno; /* value caller might want printed */ 
  vsnprintf(buf, MAXLINE, fmt, ap);
  if (errnoflag)
    snprintf(buf+strlen(buf), MAXLINE-strlen(buf), ": %s", strerror(errno_save));
  strcat(buf, "\n");
  
  //if(log_to_stderr){
  if(1){
    fflush(stdout);
    fputs(buf, stderr);
    fflush(stderr);
  }else{
    syslog(priority, buf);
  } 
}
