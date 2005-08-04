
///////////////////////////////////////////////////////////////////////
//
// This simple program merely sets up the python environment
// variables and then runs python:
//
//    PYTHONPATH
//    PATH
//
// Note that 'genpycode' is just a slight variant of 'ppython':
//
// genpycode xyz -->
// ppython direct\\src\\ffi\\jGenPyCode.py xyz
//
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
//
// Windows Version
//
///////////////////////////////////////////////////////////////////////

#ifdef WIN32

#ifdef BUILDING_PPYTHON
#define LINK_SOURCE "\\bin\\ppython.exe"
#define LINK_TARGET "\\python\\python.exe"
#define PPYTHON 1
#endif

#ifdef BUILDING_GENPYCODE
#define LINK_SOURCE "\\bin\\genpycode.exe"
#define LINK_TARGET "\\python\\python.exe"
#define GENPYCODE 1
#endif

#ifdef BUILDING_PACKPANDA
#define LINK_SOURCE "\\bin\\packpanda.exe"
#define LINK_TARGET "\\python\\python.exe"
#define PACKPANDA 1
#endif

#include <windows.h>
#include <winuser.h>
#include <stdlib.h>
#include <process.h>
#include <malloc.h>
#include <stdio.h>
#include <signal.h>
#define PATH_MAX 1024

void pathfail(void)
{
  fprintf(stderr, "Cannot locate the root of the panda tree\n");
  exit(1);
}

int main(int argc, char **argv)
{
  char fnbuf[PATH_MAX],ppbuf[PATH_MAX],pabuf[PATH_MAX],modcmd[PATH_MAX];
  int fnlen;

  // Ask windows for the file name of this executable.

  fnlen = GetModuleFileName(NULL, fnbuf, 1023);  
  if ((fnlen <= 0)||(fnlen >= 1023)) pathfail();
  fnbuf[fnlen] = 0;

  // Make sure that the executable's name ends in LINK_SOURCE
  
  int srclen = strlen(LINK_SOURCE);
  if (fnlen < srclen + 4) pathfail();
  if (stricmp(fnbuf + fnlen - srclen, LINK_SOURCE)) pathfail();
  fnlen -= srclen; fnbuf[fnlen] = 0;
  
  // See if we can find the panda root.  If not, abort.
  
  sprintf(ppbuf,"%s/direct/__init__.py",fnbuf);
  FILE *f = fopen(ppbuf,"r");
  if (f==0) pathfail();
  fclose(f);
  
  // Set the PYTHONPATH and PATH
  
  char *pp = getenv("PYTHONPATH");
  if (pp) sprintf(ppbuf,"PYTHONPATH=%s;%s\\bin;%s\\lib;%s",fnbuf,fnbuf,fnbuf,pp);
  else    sprintf(ppbuf,"PYTHONPATH=%s;%s\\bin;%s\\lib",fnbuf,fnbuf,fnbuf);
  putenv(ppbuf);
  char *path = getenv("PATH");
  if (path) sprintf(pabuf,"PATH=%s\\bin;%s",fnbuf,path);
  else      sprintf(pabuf,"PATH=%s\\bin",fnbuf);
  putenv(pabuf);
  
  // Fetch the command line and trim the first word.

  char *cmdline = GetCommandLine();
  char *args = cmdline;
  bool inquote = false;
  while (*args && ((*args != ' ')||(inquote))) {
    if (*args == '"') inquote = !inquote;
    args++;
  }
  while (*args==' ') args++;

  // Append LINK_TARGET to the file name.
  
  if (fnlen + strlen(LINK_TARGET) > 1023) pathfail();
  strcat(fnbuf, LINK_TARGET);
  
  // Calculate MODCMD
  
#ifdef PPYTHON
  sprintf(modcmd,"python %s",args);
#endif
#ifdef GENPYCODE
  sprintf(modcmd,"python -c \"import direct.ffi.jGenPyCode\" %s",args);
#endif
#ifdef PACKPANDA
  sprintf(modcmd,"python -c \"import direct.directscripts.packpanda\" %s",args);
#endif
  
  // Run it.

  signal(SIGINT, SIG_IGN);
  PROCESS_INFORMATION pinfo;
  STARTUPINFO sinfo;
  GetStartupInfo(&sinfo);
  BOOL ok = CreateProcess(fnbuf,modcmd,NULL,NULL,TRUE,NULL,NULL,NULL,&sinfo,&pinfo);
  if (ok) WaitForSingleObject(pinfo.hProcess,INFINITE);
}

#endif /* WIN32 */

///////////////////////////////////////////////////////////////////////
//
// Linux Version
//
// This would probably work on most unixes, with the possible
// exception of the /proc/self/exe bit.
//
///////////////////////////////////////////////////////////////////////

#ifdef __linux__

#ifdef BUILDING_PPYTHON
#define LINK_SOURCE "/bin/ppython"
#define PPYTHON 1
#endif

#ifdef BUILDING_GENPYCODE
#define LINK_SOURCE "/bin/genpycode"
#define GENPYCODE 1
#endif

#ifdef BUILDING_PACKPANDA
#define LINK_SOURCE "/bin/packpanda"
#define PACKPANDA 1
#endif

#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/param.h>

void errorexit(char *s)
{
  fprintf(stderr,"%s\n");
  exit(1);
}

void pathfail(void)
{
  fprintf(stderr, "Cannot locate the root of the panda tree\n");
  exit(1);
}

int main(int argc, char **argv)
{
  char fnbuf[PATH_MAX],ppbuf[PATH_MAX];
  char *modargv[1024];
  int fnlen,modargc;

  // Ask linux for the file name of this executable.

  int ok = readlink("/proc/self/exe", fnbuf, PATH_MAX-1);
  if (ok<0) errorexit("Cannot read /proc/sys/exe");
  fnbuf[PATH_MAX-1] = 0;
  fnlen = strlen(fnbuf);

  // Make sure that the executable's name ends in LINK_SOURCE
  
  int srclen = strlen(LINK_SOURCE);
  if (fnlen < srclen + 4) pathfail();
  if (strcmp(fnbuf + fnlen - srclen, LINK_SOURCE)) pathfail();
  fnlen -= srclen; fnbuf[fnlen] = 0;

  // See if we can find the 'direct' tree locally.
  // If not, continue anyway.  It may be possible to succeed.
  
  sprintf(ppbuf,"%s/direct/__init__.py",fnbuf);
  FILE *f = fopen(ppbuf,"r");
  if (f) {
    char *pp = getenv("PYTHONPATH");
    if (pp) sprintf(ppbuf,"PYTHONPATH=%s:%s/lib:%s",fnbuf,fnbuf,pp);
    else    sprintf(ppbuf,"PYTHONPATH=%s:%s/lib",fnbuf,fnbuf);
    putenv(ppbuf);
  }
  
  // Calculate MODARGV
  
  modargc=0;
  modargv[modargc++]="python";
#ifdef GENPYCODE
    modargv[modargc++] = "-c";
    modargv[modargc++] = "import direct.ffi.jGenPyCode";
#endif
#ifdef PACKPANDA
    modargv[modargc++] = "-c";
    modargv[modargc++] = "import direct.ffi.packpanda";
#endif
  for (int i=1; i<argc; i++) modargv[modargc++] = argv[i];
  modargv[modargc] = 0;
  
  // Run it.

  execv("/usr/bin/python", modargv);
}

#endif /* LINUX */
