///////////////////////////////////////////////////////////////////////
//
// This is a little wrapper to make it easy to run a python
// program from the command line.
//
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
//
// Windows Version
//
///////////////////////////////////////////////////////////////////////

#ifdef WIN32

#ifdef BUILDING_GENPYCODE
#define LINK_SOURCE "\\bin\\genpycode.exe"
#define LINK_TARGET "\\python\\python.exe"
#endif

#ifdef BUILDING_PACKPANDA
#define LINK_SOURCE "\\bin\\packpanda.exe"
#define LINK_TARGET "\\python\\python.exe"
#endif

#ifdef BUILDING_EGGCACHER
#define LINK_SOURCE "\\bin\\loadalleggs.exe"
#define LINK_TARGET "\\python\\python.exe"
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
  
#ifdef BUILDING_GENPYCODE
  sprintf(modcmd,"python -c \"import direct.ffi.jGenPyCode\" %s",args);
#endif
#ifdef BUILDING_PACKPANDA
  sprintf(modcmd,"python -c \"import direct.directscripts.packpanda\" %s",args);
#endif
#ifdef BUILDING_EGGCACHER
  sprintf(modcmd,"python -c \"import direct.directscripts.eggcacher\" %s",args);
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

#ifdef BUILDING_GENPYCODE
#define LINK_SOURCE "/bin/genpycode"
#endif

#ifdef BUILDING_PACKPANDA
#define LINK_SOURCE "/bin/packpanda"
#endif

#ifdef BUILDING_EGGCACHER
#define LINK_SOURCE "/bin/eggcacher"
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

  // Calculate MODARGV
  
  modargc=0;
  modargv[modargc++]="python";
#ifdef BUILDING_GENPYCODE
    modargv[modargc++] = "-c";
    modargv[modargc++] = "import direct.ffi.jGenPyCode";
#endif
#ifdef BUILDING_PACKPANDA
    modargv[modargc++] = "-c";
    modargv[modargc++] = "import direct.directscripts.packpanda";
#endif
#ifdef BUILDING_EGGCACHER
    modargv[modargc++] = "-c";
    modargv[modargc++] = "import direct.directscripts.eggcacher";
#endif
  for (int i=1; i<argc; i++) modargv[modargc++] = argv[i];
  modargv[modargc] = 0;
  
  // Run it.

  execv("/usr/bin/python", modargv);
}

#endif /* LINUX */
