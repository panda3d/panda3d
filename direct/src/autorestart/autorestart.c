/* Filename: autorestart.c
 * Created by:  drose (05Sep02)
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef WITHIN_PANDA
#include "dtoolbase.h"
#endif

#include <getopt.h>
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
#include <pwd.h>
#include <grp.h>

#ifdef HAVE_LIBCURL
#include <curl/curl.h>
#endif

/* The maximum number of seconds to wait for a process to go away
   after issuing SIGTERM.  This is only used in watchdog mode, when -W
   is provided on the command line. */
#define MAX_WAITTERM_SEC 10

char **params = NULL;
char *logfile_name = NULL;
char *pidfile_name = NULL;
int dont_fork = 0;
char *watchdog_url = NULL;
int watchdog_start_sec = 0;
int watchdog_cycle_sec = 0;
int watchdog_timeout_sec = 0;
char *startup_username = NULL;
char *startup_groupname = NULL;
char *startup_chdir = NULL;
int logfile_fd = -1;
int stop_on_terminate = 0;
int stop_always = 0;
char *respawn_script = NULL;
int respawn_count_time = 0;

/* If requested, delay these many seconds between restart attempts */
int respawn_delay_time = 5;


/* We shouldn't respawn more than (spam_respawn_count - 1) times over
   spam_respawn_time seconds. */
int spam_respawn_count = 5;
int spam_respawn_time = 60;
int spam_restart_delay_time = 600;  /* Optionally, do not exit if we spam too much; simply sleep for this many seconds*/



pid_t child_pid = 0;
pid_t watchdog_pid = 0;

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

/* A callback function passed to libcurl that simply discards the data
   retrieved from the server.  We only care about the HTTP status. */
size_t 
watchdog_bitbucket(void *ptr, size_t size, size_t nmemb, void *userdata) {
  return size * nmemb;
}

/* Waits up to timeout_ms for a particular child to terminate.
   Returns 0 if the timeout expires. */
pid_t 
waitpid_timeout(pid_t child_pid, int *status_ptr, int timeout_ms) {
  pid_t result;
  struct timeval now, tv;
  int now_ms, start_ms, elapsed_ms;
  
  gettimeofday(&now, NULL);
  start_ms = now.tv_sec * 1000 + now.tv_usec / 1000;
    
  result = waitpid(child_pid, status_ptr, WNOHANG);
  while (result == 0) {
    gettimeofday(&now, NULL);
    now_ms = now.tv_sec * 1000 + now.tv_usec / 1000;
    elapsed_ms = now_ms - start_ms;
    
    if (elapsed_ms > timeout_ms) {
      /* Tired of waiting. */
      return 0;
    }
    
    /* Yield the timeslice and wait some more. */
    tv.tv_sec = 0;
    tv.tv_usec = 1;
    select(0, NULL, NULL, NULL, &tv);
    result = waitpid(child_pid, status_ptr, WNOHANG);
  }
  if (result == -1) {
    perror("waitpid");
  }

  return result;
}


/* Poll the requested URL until a failure or timeout occurs, or until
   the child terminates on its own.  Returns 1 on HTTP failure or
   timeout, 0 on self-termination.  In either case, *status_ptr is
   filled in with the status value returned by waitpid().*/
int 
do_watchdog(int *status_ptr) {
#ifndef HAVE_LIBCURL
  fprintf(stderr, "Cannot watchdog; no libcurl available.\n");
  return 0;
#else  /* HAVE_LIBCURL */

  CURL *curl;
  CURLcode res;
  char error_buffer[CURL_ERROR_SIZE];
  pid_t wresult;

  // Before we start polling the URL, wait at least start milliseconds.
  wresult = waitpid_timeout(child_pid, status_ptr, watchdog_start_sec * 1000);
  if (wresult == child_pid) {
    // The child terminated on its own before we got started.
    return 0;
  }

  curl = curl_easy_init();
  if (!curl) {
    fprintf(stderr, "Cannot watchdog; curl failed to init.\n");
    return 0;
  }

  curl_easy_setopt(curl, CURLOPT_URL, watchdog_url);
  /*curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);*/
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, watchdog_timeout_sec * 1000);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, watchdog_bitbucket);
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "autorestart");
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1);
  curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);

  res = curl_easy_perform(curl);
  while (res == 0) {
    /* 0: The HTTP request finished successfully (but might or might
       not have returned an error code like a 404). */
    long http_response = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_response);
    if ((http_response / 100) != 2) {
      /* Anything in the 200 range is deemed success.  Anything else
         is deemed failure. */
      fprintf(stderr, "%s returned %ld\n", watchdog_url, http_response);
      break;
    }

    wresult = waitpid_timeout(child_pid, status_ptr, watchdog_cycle_sec * 1000);
    if (wresult == child_pid) {
      /* The process terminated on its own.  Return 0 to indicate this. */
      return 0;
    }

    res = curl_easy_perform(curl);
  }

  curl_easy_cleanup(curl);

  /* Failed to retrieve the watchdog URL. */
  if (res != 0) {
    fprintf(stderr, "Failed to contact %s: %s\n", watchdog_url, error_buffer);
  }
  
  /* Kill the child process and wait for it to go away. */
  kill(child_pid, SIGTERM);

  pid_t result = waitpid_timeout(child_pid, status_ptr, MAX_WAITTERM_SEC * 1000);
  if (result != child_pid) {
    if (result == -1) {
      perror("waitpid");
    } else {
      /* SIGTERM didn't make the process die.  Try SIGKILL. */
      fprintf(stderr, "Force-killing child process\n");
      kill(child_pid, SIGKILL);
      result = waitpid_timeout(child_pid, status_ptr, MAX_WAITTERM_SEC * 1000);
      if (result == -1) {
        perror("waitpid");
      }
    }
  }

  /* Return 1 to indicate we killed the child due to an HTTP error. */
  return 1;
#endif  /* HAVE_LIBCURL */
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
  int error_exit;

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

  /* Parent. */

  error_exit = 0;

  if (watchdog_url != NULL) {
    /* If we're watchdogging, then go check the URL.  This function
       won't return until the URL fails or the child exits. */
    error_exit = do_watchdog(&status);

  } else {
    /* If we're not watchdogging, then just wait for the child to
       terminate, and diagnose the reason. */
    wresult = waitpid(child_pid, &status, 0);
    if (wresult < 0) {
      perror("waitpid");
      return 0;
    }
  }

  /* Now that we've returned from waitpid, clear the child pid number
     so our signal handler doesn't get too confused. */
  child_pid = 0;

  if (error_exit) {
    /* An HTTP error exit is a reason to respawn. */
    return 1;

  } else if (WIFSIGNALED(status)) {
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
sighup_handler() {
  time_t now;
  char time_buffer[TIME_BUFFER_SIZE];

  now = time(NULL);
  strftime(time_buffer, TIME_BUFFER_SIZE, "%T on %A, %d %b %Y", localtime(&now));

  fprintf(stderr, "\nsighup caught at %s.\n", time_buffer);
  if (child_pid == 0) {
    fprintf(stderr, "no child process.\n\n");

  } else {
    kill(child_pid, SIGHUP);
  }
}

void 
sigalarm_handler() {
  fprintf(stderr, "sleep epoch was complete.\n");
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

  /* Set up a signal handler to trap SIGHUP.  We pass this into the
     child. */
  sa.sa_handler = sighup_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  if (sigaction(SIGHUP, &sa, NULL) < 0) {
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

  if (pidfile_name != NULL) {
    unlink(pidfile_name);
    FILE *pidfile = fopen(pidfile_name, "w");
    if (pidfile == NULL) {
      fprintf(stderr, "Could not write pidfile %s\n", pidfile_name);
    } else {
      fprintf(pidfile, "%d\n", getpid());
      fclose(pidfile);
    }
  }

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
    
    if (respawn_delay_time) {
      sleep(respawn_delay_time);
    }

    /* Make sure we're not respawning too fast. */
    if (spam_respawn_count > 1) {
      sri = (sri + 1) % spam_respawn_count;
      spam_respawn[sri] = now;
      if (num_sri < spam_respawn_count) {
        num_sri++;
      } else {
        time_t last = spam_respawn[(sri + 1) % spam_respawn_count];
        if (now - last < spam_respawn_time) 
        {
          if(!spam_restart_delay_time) 
          {
            fprintf(stderr, "respawning too fast, giving up.\n");
            break;
          } 
          else 
          {
            num_sri = 1; /* reset num_sri */
            fprintf(stderr, "respawning too fast, will sleep for %d seconds.\n", spam_restart_delay_time);
            signal (SIGALRM, sigalarm_handler);
            alarm(spam_restart_delay_time);
            pause();
            signal (SIGALRM, SIG_IGN);
          }
        }
      }
    }
    
    if (stop_always) {
      fprintf(stderr, "instructed to not autorestart, exiting.\n");
      break;
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

          "  -p pidfilename\n"
          "     Write the pid of the monitoring process to the indicated pidfile.\n\n"
          "  -f\n"
          "     Don't fork autorestart itself; run it as a foreground process. \n"
          "     (Normally, autorestart forks itself to run as a background process.)\n"
          "     In this case, the file named by -p is not used.\n\n"
          
          "  -n\n"
          "     Do not attempt to restart the process under any circumstance.\n"
          "     The program can still be used to execute a script on abnormal\n"
          "     process termination.\n\n"

          "  -t\n"
          "     Stop on terminate: don't restart if the child process exits\n"
          "     normally or is killed with a SIGTERM.  With this flag, the\n"
          "     child process will be restarted only if it exits with a\n"
          "     non-zero exit status, or if it is killed with a signal other\n"
          "     than SIGTERM.  Without this flag, the default behavior is to\n"
          "     restart the child process if it exits for any reason.\n\n"

          "  -r count,secs,sleep\n"
          "     Sleep 'sleep' seconds if the process respawns 'count' times\n"
          "     within 'secs' seconds.  This is designed to prevent respawning\n"
          "     from using too many system resources if something is wrong with\n"
          "     the child process.  The default value is %d,%d,%d. Use -r 0,0,0\n"
          "     to disable this feature.\n\n"

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

          "  -d secs\n"
          "     Specifies the number of seconds to delay for between restarts.\n"
          "     The default is %d.\n\n"

#ifdef HAVE_LIBCURL
          "  -W watchdog_url,start,cycle,timeout\n"
          "     Specifies an optional URL to watch while waiting for the process\n"
          "     to terminate.  If this is specified, autorestart will start the process,\n"
          "     wait start seconds, and then repeatedly poll the indicated URL\n"
          "     every cycle seconds.  If a HTTP failure code is detected,\n"
          "     or no response is received within timeout seconds, then the\n"
          "     child is terminated and restarted.  The start, cycle, and timeout\n"
          "     parameters are all required.\n\n"
#endif  /* HAVE_LIBCURL */

          "  -U username\n"
          "     Change to the indicated user upon startup.  The logfile is still\n"
          "     created as the initial user.\n\n"

          "  -G groupname\n"
          "     Change to the indicated group upon startup.\n\n"

          "  -D dirname\n"
          "     Change to the indicated working directory upon startup.  The logfile\n"
          "     is still created relative to the initial startup directory.\n\n"

          "  -h\n"
          "     Output this help information.\n\n",
          spam_respawn_count, spam_respawn_time, spam_restart_delay_time, respawn_delay_time);
}

void
parse_int_triplet(char *param, int *a, int *b, int *c) {
  char *comma;
  char *comma2;
  
  comma = strchr(param, ',');
  if (comma == NULL) {
    fprintf(stderr, "Comma required: %s\n", param);
    exit(1);
  }

  comma2 = strchr(comma+1, ',');
  if (comma2 == NULL) {
    fprintf(stderr, "Second comma required: %s\n", param);
    exit(1);
  }

  *comma = '\0';
  *comma2 = '\0';
  
  *a = atoi(param);
  *b = atoi(comma + 1);
  *c = atoi(comma2 + 1);
}

void 
parse_watchdog(char *param) {
  char *comma;
  char *comma2;
  char *comma3;

#ifndef HAVE_LIBCURL
  fprintf(stderr, "-W requires autorestart to have been compiled with libcurl support.\n");
  exit(1);
#endif  /* HAVE_LIBCURL */

  comma = strrchr(param, ',');
  if (comma == NULL) {
    fprintf(stderr, "Comma required: %s\n", param);
    exit(1);
  }
  *comma = '\0';

  comma2 = strrchr(param, ',');
  if (comma2 == NULL) {
    *comma = ',';
    fprintf(stderr, "Second comma required: %s\n", param);
    exit(1);
  }
  *comma2 = '\0';

  comma3 = strrchr(param, ',');
  if (comma3 == NULL) {
    *comma = ',';
    *comma2 = ',';
    fprintf(stderr, "Third comma required: %s\n", param);
    exit(1);
  }
  *comma3 = '\0';

  watchdog_url = param;
  watchdog_start_sec = atoi(comma3 + 1);
  watchdog_cycle_sec = atoi(comma2 + 1);
  watchdog_timeout_sec = atoi(comma + 1);
}


int 
main(int argc, char *argv[]) {
  extern char *optarg;
  extern int optind;
  /* The initial '+' instructs GNU getopt not to reorder switches. */
  static const char *optflags = "+l:p:fntr:s:c:d:W:U:G:D:h";
  int flag;

  flag = getopt(argc, argv, optflags);
  while (flag != EOF) {
    switch (flag) {
    case 'l':
      logfile_name = optarg;
      break;

    case 'p':
      pidfile_name = optarg;
      break;

    case 'f':
      dont_fork = 1;
      break;

    case 'n':
      stop_always = 1;
      break;

    case 't':
      stop_on_terminate = 1;
      break;

    case 'r':
      parse_int_triplet(optarg, &spam_respawn_count, &spam_respawn_time, &spam_restart_delay_time);
      break;

    case 's':
      respawn_script = optarg;
      break;

    case 'c':
      respawn_count_time = atoi(optarg);
      break;

    case 'd':
      respawn_delay_time = atoi(optarg);
      break;

    case 'W':
      parse_watchdog(optarg);
      break;

    case 'U':
      startup_username = optarg;
      break;

    case 'G':
      startup_groupname = optarg;
      break;

    case 'D':
      startup_chdir = optarg;
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

  if (startup_chdir != NULL) {
    if (chdir(startup_chdir) != 0) {
      perror(startup_chdir);
      return 1;
    }
  }

  if (startup_groupname != NULL) {
    struct group *grp;
    grp = getgrnam(startup_groupname);
    if (grp == NULL) {
      perror(startup_groupname);
      return 1;
    }

    if (setgid(grp->gr_gid) != 0) {
      perror(startup_groupname);
      return 1;
    }
  }

  if (startup_username != NULL) {
    struct passwd *pwd;
    pwd = getpwnam(startup_username);
    if (pwd == NULL) {
      perror(startup_username);
      return 1;
    }

    if (setuid(pwd->pw_uid) != 0) {
      perror(startup_username);
      return 1;
    }
  }

  if (dont_fork) {
    do_autorestart();
  } else {
    double_fork();
  }

  return 0;
}

