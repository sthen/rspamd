#ifndef UTIL_H
#define UTIL_H

#include <sys/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/queue.h>
#include <sys/time.h>

#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <signal.h>

#include "main.h"

struct config_file;

/* Create socket and bind it to specified address and port */
int make_socket(struct in_addr *, u_short );
/* Create and bind unix socket */
int make_unix_socket (const char *, struct sockaddr_un *);
/* Parse command line arguments using getopt (3) */
void read_cmd_line (int , char **, struct config_file *);
/* Write pid to file */
int write_pid (struct rspamd_main *);
/* Make specified socket non-blocking */
int event_make_socket_nonblocking(int);
/* Init signals */
void init_signals (struct sigaction *, sig_t);
/* Send specified signal to each worker */
void pass_signal_worker (struct workq *, int );
/* Convert string to lowercase */
void convert_to_lowercase (char *str, unsigned int size);

#ifndef HAVE_SETPROCTITLE
int init_title(int argc, char *argv[], char *envp[]);
int setproctitle(const char *fmt, ...);
#endif

#ifndef HAVE_PIDFILE
struct pidfh {
	int pf_fd;
#ifdef HAVE_PATH_MAX
	char    pf_path[PATH_MAX + 1];
#elif defined(HAVE_MAXPATHLEN)
	char    pf_path[MAXPATHLEN + 1];
#else
	char    pf_path[1024 + 1];
#endif
 	__dev_t pf_dev;
 	ino_t   pf_ino;
};
struct pidfh *pidfile_open(const char *path, mode_t mode, pid_t *pidptr);
int pidfile_write(struct pidfh *pfh);
int pidfile_close(struct pidfh *pfh);
int pidfile_remove(struct pidfh *pfh);
#endif

int open_log (struct config_file *cfg);
int reopen_log (struct config_file *cfg);
void syslog_log_function (const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer arg);
void file_log_function (const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer arg);

/* Replace %r with rcpt value and %f with from value, new string is allocated in pool */
char* resolve_stat_filename (memory_pool_t *pool, char *pattern, char *rcpt, char *from);


#endif
