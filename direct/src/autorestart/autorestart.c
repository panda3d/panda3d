/* Filename: autorestart.c
 * Created by:  drose (05Sep02)
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * PANDA 3D SOFTWARE
 * Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
 *
 * All use of this software is subject to the terms of the Panda 3d
 * Software license.  You should have received a copy of this license
 * along with this source code; you will also find a current copy of
 * the license at http://www.panda3d.org/license.txt .
 *
 * To contact the maintainers of this program write to
 * panda3d@yahoogroups.com .
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "dtool_config.h"
#include "dtoolbase.h"

#ifndef HAVE_GETOPT
#include "gnu_getopt.h"
#else
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>  /* for strerror */
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <assert.h>

char **params = NULL;
char *logfile_name = NULL;
int logfile_fd = -1;
int stop_on_terminate = 0;
char *respawn_script = NULL;
int respawn_count_time = 0;

/* We shouldn't respawn more than (spam_respawn_count - 1) times over
   spam_respawn_time seconds. */
int spam_respawn_count = 5;
int spam_respawn_time = 30;

pid_t child_pid = 0;

#define TIME_BUFFER_SIZE 128

/* Keep track of the frequency with which we respawn, so we can report
   this to our respawn script. */
typedef struct respawn_record_struct {
  time_t _time;
  struct respawn_record_struct *_next;
} respawn_record;

respawn_record *respawns = NULL;

int
record_respawn(time_t now) {
  /* Records the respawning event in the respawn_record, and returns
     the number of respawns in the last respawn_count_time
     interval. */
  respawn_record *rec;
  respawn_record *next;
  int count;

  if (respawn_count_time <= 0) {
    /* We're not tracking respawns if respawn_count_time is 0. */
    return 0;
  }

  rec = (respawn_record *)malloc(sizeof(respawn_record));
  rec->_time = now;
  rec->_next = respawns;
  respawns = rec;

  /* Now walk through the rest of the list and count up the number of
     respawn events until we reach a record more than
     respawn_count_time seconds old. */
  count = 0;
  while (rec->_next != NULL &&
         (now - rec->_time) <= respawn_count_time) {
    rec = rec->_next;
    count++;
  }

  /* The remaining respawn records get removed. */
  next = rec->_next;
  rec->_next = NULL;
  while (next != NULL) {
    rec = next;
    next = rec->_next;
    free(rec);
  }

  return count;
}

void
invoke_respawn_script(time_t now) {
  char buffer[32];
  char *new_command;
  int new_command_length;

  /* The process is about to be respawned; run the script that we were
     given on the command line. */
  if (respawn_count_time <= 0) {
    /* We're not counting respawn times, so just run the script
       directly. */
    system(respawn_script);

  } else {
    /* We are counting respawn times, so append that information as a
       parameter to the command. */
    sprintf(buffer, " %d", record_respawn(now));
    new_command_length = strlen(respawn_script) + strlen(buffer);
    new_command = (char *)malloc(new_command_length + 1);
    strcpy(new_command, respawn_script);
    strcat(new_command, buffer);
    assert(strlen(new_command) == new_command_length);

    system(new_command);

    free(new_command);
  }
}

void
exec_process() {
  /* First, output the command line to the log file. */
  char **p;
  for (p = params; *p != NULL; ++p) {
    fprintf(stderr, "%s ", *p);
  }
  fprintf(stderr, "\n");
  execvp(params[0], params);
  fprintf(stderr, "Cannot exec %s: %s\n", params[0], strerror(errno));

  /* Exit with a status of 0, to indicate to the parent process that
     we should stop. */
  exit(0); 
}

int
spawn_process() {
  /* Spawns the child process.  Returns true if the process terminated
     by itself and should be respawned, false if it was explicitly
     killed (or some other error condition exists), and it should not
     respawn any more. */
  pid_t wresult;
  int status;

  child_pid = fork();
  if (child_pid < 0) {
    /* Fork error. */
    perror("fork");
    return 0;
  }

  if (child_pid == 0) {
    /* Child.  Exec the process. */
    fprintf(stderr, "Child pid is %d.\n", getpid());
    exec_process();
    /* Shouldn't get here. */
    exit(1);
  }

  /* Parent.  Wait for the child to terminate, then diagnose the reason. */
  wresult = waitpid(child_pid, &status, 0);
  if (wresult < 0) {
    perror("waitpid");
    return 0;
  }

  /* Now that we've returned from waitpid, clear the child pid number
     so our signal handler doesn't get too confused. */
  child_pid = 0;

  if (WIFSIGNALED(status)) {
    int signal = WTERMSIG(status);
    fprintf(stderr, "\nprocess caught signal %d.\n\n", signal);
    /* A signal exit is a reason to respawn unless the signal is TERM
       or KILL. */
    return !stop_on_terminate || (signal != SIGTERM && signal != SIGKILL);

  } else {
    int exit_status = WEXITSTATUS(status);
    fprintf(stderr, "\nprocess exited with status %d.\n\n", WEXITSTATUS(status));
    /* Normal exit is a reason to respawn if the status indicates failure. */
    return !stop_on_terminate || (exit_status != 0);
  }
}

void
sigterm_handler() {
  pid_t wresult;
  int status;
  time_t now;
  char time_buffer[TIME_BUFFER_SIZE];

  now = time(NULL);
  strftime(time_buffer, TIME_BUFFER_SIZE, "%T on %A, %d %b %Y", localtime(&now));

  fprintf(stderr, "\nsigterm caught at %s; shutting down.\n", time_buffer);
  if (child_pid == 0) {
    fprintf(stderr, "no child process.\n\n");

  } else {
    kill(child_pid, SIGTERM);

    wresult = waitpid(child_pid, &status, 0);
    if (wresult < 0) {
      perror("waitpid");
    } else {
      fprintf(stderr, "child process terminated.\n\n");
    }
  }
  exit(1);
}

void
do_autorestart() {
  char time_buffer[TIME_BUFFER_SIZE];
  time_t now;
  time_t *spam_respawn = NULL;
  int sri, num_sri;
  struct sigaction sa;

  if (spam_respawn_count > 1) {
    spam_respawn = (time_t *)malloc(sizeof(time_t) * spam_respawn_count);
  }

  /* Make our process its own process group. */
  setpgid(0, 0);

  /* Set up a signal handler to trap SIGTERM. */
  sa.sa_handler = sigterm_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  if (sigaction(SIGTERM, &sa, NULL) < 0) {
    perror("sigaction");
  }

  if (logfile_fd >= 0) {
    /* If we have a logfile, dup it onto stdout and stderr. */
    dup2(logfile_fd, STDOUT_FILENO);
    dup2(logfile_fd, STDERR_FILENO);
    close(logfile_fd);
  }

  /* Make sure stdin is closed. */
  close(STDIN_FILENO);

  now = time(NULL);
  strftime(time_buffer, TIME_BUFFER_SIZE, "%T on %A, %d %b %Y", localtime(&now));
  fprintf(stderr, "autorestart begun at %s.\n", time_buffer);

  sri = 1;
  num_sri = 1;
  if (spam_respawn_count > 1) {
    spam_respawn[1] = now;
  }
  
  while (spawn_process()) {
    now = time(NULL);

    if (respawn_script != NULL) {
      invoke_respawn_script(now);
    }

    /* Make sure we're not respawning too fast. */
    if (spam_respawn_count > 1) {
      sri = (sri + 1) % spam_respawn_count;
      spam_respawn[sri] = now;
      if (num_sri < spam_respawn_count) {
        num_sri++;
      } else {
        time_t last = spam_respawn[(sri + 1) % spam_respawn_count];
        if (now - last < spam_respawn_time) {
          fprintf(stderr, "respawning too fast, giving up.\n");
          break;
        }
      }
    }
      
    strftime(time_buffer, TIME_BUFFER_SIZE, "%T on %A, %d %b %Y", localtime(&now));
    fprintf(stderr, "respawning at %s.\n", time_buffer);
  }

  now = time(NULL);
  strftime(time_buffer, TIME_BUFFER_SIZE, "%T on %A, %d %b %Y", localtime(&now));
  fprintf(stderr, "autorestart terminated at %s.\n", time_buffer);
  exit(0);
}

void
double_fork() {
  pid_t child, grandchild, wresult;
  int status;

  /* Fork once, then again, to disassociate the child from the command
     shell process group. */
  child = fork();
  if (child < 0) {
    /* Failure to fork. */
    perror("fork");
    exit(1);
  }

  if (child == 0) {
    /* Child.  Fork again. */
    grandchild = fork();
    if (grandchild < 0) {
      perror("fork");
      exit(1);
    }

    if (grandchild == 0) {
      /* Grandchild.  Begin useful work. */
      do_autorestart();
      /* Shouldn't get here. */
      exit(1);
    }

    /* Child.  Report the new pid, then terminate gracefully. */
    fprintf(stderr, "Spawned, monitoring pid is %d.\n", grandchild);
    exit(0);
  }

  /* Parent.  Wait for the child to terminate, then return. */
  wresult = waitpid(child, &status, 0);
  if (wresult < 0) {
    perror("waitpid");
    exit(1);
  }

  if (!WIFEXITED(status)) {
    if (WIFSIGNALED(status)) {
      fprintf(stderr, "child caught signal %d unexpectedly.\n", WTERMSIG(status));
    } else {
      fprintf(stderr, "child exited with status %d.\n", WEXITSTATUS(status));
    }
    exit(1);
  }
}

void
usage() {
  fprintf(stderr,
          "\n"
          "autorestart [opts] program [args . . . ]\n"
          "autorestart -h\n\n");
}

void
help() {
  usage();
  fprintf(stderr,
          "This program is used to run a program as a background task and\n"
          "automatically restart it should it terminate for any reason other\n"
          "than normal exit or explicit user kill.\n\n"

          "If the program exits with a status of 0, indicating successful\n"
          "completion, it is not restarted.\n\n"

          "If the program is terminated via a TERM or KILL signal (e.g. via\n"
          "kill [pid] or kill -9 [pid]), it is assumed the user meant for the\n"
          "process to stop, and it is not restarted.\n\n"

          "Options:\n\n"

          "  -l logfilename\n"
          "     Route stdout and stderr from the child process into the indicated\n"
          "     log file.\n\n"

          "  -t\n"
          "     Stop on terminate: don't restart if the child process exits\n"
          "     normally or is killed with a SIGTERM.  With this flag, the\n"
          "     child process will be restarted only if it exits with a\n"
          "     non-zero exit status, or if it is killed with a signal other\n"
          "     than SIGTERM.  Without this flag, the default behavior is to\n"
          "     restarted the child process if it exits for any reason.\n\n"

          "  -r count,secs\n"
          "     Give up if the process respawns 'count' times within 'secs'\n"
          "     seconds.  This is designed to prevent respawning from using\n"
          "     too many system resources if something is wrong with the child\n"
          "     process.  The default value is %d,%d.  Use -r 0,0 to disable\n"
          "     this feature.\n\n"

          "  -s \"command\"\n"
          "     Run the indicated command or script each time the process is\n"
          "     respawned, using the system() call.  This may be useful, for\n"
          "     instance, to notify an operator via email each time a respawn\n"
          "     occurs.  If -c is also specified, an additional parameter will\n"
          "     be appended to the command, indicating the number of times the\n"
          "     respawn has occurred in the given time interval.\n\n"

          "  -c secs\n"
          "     Specifies the number of seconds over which to count respawn events\n"
          "     for the purposes of passing an argument to the script named with\n"
          "     -s.\n\n"

          "  -h\n"
          "     Output this help information.\n\n",
          spam_respawn_count, spam_respawn_time);
}

void
parse_int_pair(char *param, int *a, int *b) {
  char *comma = strchr(param, ',');
  if (comma == NULL) {
    fprintf(stderr, "Comma required: %s\n", param);
    exit(1);
  }

  *comma = '\0';
  *a = atoi(param);
  *b = atoi(comma + 1);
}

int 
main(int argc, char *argv[]) {
  extern char *optarg;
  extern int optind;
  /* The initial '+' instructs GNU getopt not to reorder switches. */
  static const char *optflags = "+l:tr:s:c:h";
  int flag;

  flag = getopt(argc, argv, optflags);
  while (flag != EOF) {
    switch (flag) {
    case 'l':
      logfile_name = optarg;
      break;

    case 't':
      stop_on_terminate = 1;
      break;

    case 'r':
      parse_int_pair(optarg, &spam_respawn_count, &spam_respawn_time);
      break;

    case 's':
      respawn_script = optarg;
      break;

    case 'c':
      respawn_count_time = atoi(optarg);
      break;

    case 'h':
      help();
      return 1;

    case '?':
    case '+':
      usage();
      return 1;

    default:
      fprintf(stderr, "Unhandled switch: -%c\n", flag);
      return 1;
    }
    flag = getopt(argc, argv, optflags);
  }

  argc -= (optind - 1);
  argv += (optind - 1);

  if (argc < 2) {
    fprintf(stderr, "No program to execute given.\n");
    usage();
    return 1;
  }

  params = &argv[1];

  if (logfile_name != NULL) {
    logfile_fd = open(logfile_name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (logfile_fd < 0) {
      fprintf(stderr, "Cannot write to logfile %s: %s\n", 
              logfile_name, strerror(errno));
      return 1;
    }
    fprintf(stderr, "Generating output to %s.\n", logfile_name);
  }

  double_fork();

  return 0;
}

