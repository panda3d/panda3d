/* Python interpreter main program for frozen scripts */

#include "Python.h"
#ifdef _WIN32
#include "malloc.h"
#endif

#include <stdio.h>

#if PY_MAJOR_VERSION >= 3
#include <locale.h>

#if PY_MINOR_VERSION < 5
#define Py_DecodeLocale _Py_char2wchar
#endif
#endif

#ifdef MS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

extern void PyWinFreeze_ExeInit(void);
extern void PyWinFreeze_ExeTerm(void);

static struct _inittab extensions[] = {
    {0, 0},
};

#if PY_MAJOR_VERSION >= 3
#define WIN_UNICODE
#endif
#endif

static unsigned char *modblob = NULL;

/* Main program */

#ifdef WIN_UNICODE
int Py_FrozenMain(int argc, wchar_t **argv)
#else
int Py_FrozenMain(int argc, char **argv)
#endif
{
    char *p;
    int n, sts = 1;
    int inspect = 0;
    int unbuffered = 0;

#if PY_MAJOR_VERSION >= 3 && !defined(WIN_UNICODE)
    int i;
    char *oldloc;
    wchar_t **argv_copy = NULL;
    /* We need a second copies, as Python might modify the first one. */
    wchar_t **argv_copy2 = NULL;

    if (argc > 0) {
        argv_copy = (wchar_t **)alloca(sizeof(wchar_t *) * argc);
        argv_copy2 = (wchar_t **)alloca(sizeof(wchar_t *) * argc);
    }
#endif

    Py_FrozenFlag = 1; /* Suppress errors from getpath.c */
    Py_NoSiteFlag = 1;
    Py_NoUserSiteDirectory = 1;

    if ((p = Py_GETENV("PYTHONINSPECT")) && *p != '\0')
        inspect = 1;
    if ((p = Py_GETENV("PYTHONUNBUFFERED")) && *p != '\0')
        unbuffered = 1;

    if (unbuffered) {
        setbuf(stdin, (char *)NULL);
        setbuf(stdout, (char *)NULL);
        setbuf(stderr, (char *)NULL);
    }

#if PY_MAJOR_VERSION >= 3 && !defined(WIN_UNICODE)
    oldloc = setlocale(LC_ALL, NULL);
    setlocale(LC_ALL, "");
    for (i = 0; i < argc; i++) {
        argv_copy[i] = Py_DecodeLocale(argv[i], NULL);
        argv_copy2[i] = argv_copy[i];
        if (!argv_copy[i]) {
            fprintf(stderr, "Unable to decode the command line argument #%i\n",
                            i + 1);
            argc = i;
            goto error;
        }
    }
    setlocale(LC_ALL, oldloc);
#endif

#ifdef MS_WINDOWS
    PyImport_ExtendInittab(extensions);
#endif /* MS_WINDOWS */

    if (argc >= 1) {
#if PY_MAJOR_VERSION >= 3 && !defined(WIN_UNICODE)
        Py_SetProgramName(argv_copy[0]);
#else
        Py_SetProgramName(argv[0]);
#endif
    }

    Py_Initialize();
#ifdef MS_WINDOWS
    PyWinFreeze_ExeInit();
#endif

    if (Py_VerboseFlag)
        fprintf(stderr, "Python %s\n%s\n",
            Py_GetVersion(), Py_GetCopyright());

#if PY_MAJOR_VERSION >= 3 && !defined(WIN_UNICODE)
    PySys_SetArgv(argc, argv_copy);
#else
    PySys_SetArgv(argc, argv);
#endif

    n = PyImport_ImportFrozenModule("__main__");
    if (n == 0)
        Py_FatalError("__main__ not frozen");
    if (n < 0) {
        PyErr_Print();
        sts = 1;
    }
    else
        sts = 0;

    if (inspect && isatty((int)fileno(stdin)))
        sts = PyRun_AnyFile(stdin, "<stdin>") != 0;

#ifdef MS_WINDOWS
    PyWinFreeze_ExeTerm();
#endif
    Py_Finalize();

#if PY_MAJOR_VERSION >= 3 && !defined(WIN_UNICODE)
error:
    if (argv_copy2) {
        for (i = 0; i < argc; i++) {
#if PY_MINOR_VERSION >= 4
            PyMem_RawFree(argv_copy2[i]);
#else
            PyMem_Free(argv_copy2[i]);
#endif
        }
    }
#endif
    return sts;
}


#if defined(_WIN32) && PY_MAJOR_VERSION >= 3
int wmain(int argc, wchar_t *argv[]) {
#else
int main(int argc, char *argv[]) {
#endif
  struct _frozen *_PyImport_FrozenModules;
  unsigned int listoff, modsoff, fsize, modsize, listsize, nummods, modidx;
  int retval;
  FILE *runtime;

#ifdef _WIN32
  wchar_t buffer[2048];
  GetModuleFileNameW(NULL, buffer, 2048);
  runtime = _wfopen(buffer, L"rb");
#else
  runtime = fopen(argv[0], "rb");
#endif

  // Get offsets
  fseek(runtime, -12, SEEK_END);
  fsize = ftell(runtime);
  fread(&listoff, 4, 1, runtime);
  fread(&modsoff, 4, 1, runtime);
  fread(&nummods, 4, 1, runtime);
  modsize = fsize - modsoff;
  listsize = modsoff - listoff;

  // Read module blob
  modblob = malloc(modsize);
  fseek(runtime, modsoff, SEEK_SET);
  fread(modblob, modsize, 1, runtime);

  // Read module list
  _PyImport_FrozenModules = calloc(nummods + 1, sizeof(struct _frozen));
  fseek(runtime, listoff, SEEK_SET);
  for (modidx = 0; modidx < nummods; ++modidx) {
    struct _frozen *moddef = &_PyImport_FrozenModules[modidx];
    unsigned char name_size;
    char *name = NULL, namebuf[256] = {0};
    size_t nsize;
    unsigned int codeptr;
    int codesize;

    // Name
    fread(&name_size, 1, 1, runtime);
    fread(namebuf, 1, name_size, runtime);
    nsize = strlen(namebuf) + 1;
    name = malloc(nsize);
    memcpy(name, namebuf, nsize);
    moddef->name = name;

    // Pointer
    fread(&codeptr, 4, 1, runtime);
    moddef->code = modblob + codeptr;

    // Size
    fread(&codesize, 4, 1, runtime);
    moddef->size = codesize;
  }

  fclose(runtime);

  // Uncomment this to print out the read in module list
  //for (modidx = 0; modidx < nummods; ++modidx) {
  //  struct _frozen *moddef = &_PyImport_FrozenModules[modidx];
  //  printf("MOD: %s %p %d\n", moddef->name, (void*)moddef->code, moddef->size);
  //}

  // Run frozen application
  PyImport_FrozenModules = _PyImport_FrozenModules;
  retval = Py_FrozenMain(argc, argv);

  // Free resources
  free(modblob);
  for (modidx = 0; modidx < nummods; ++modidx) {
    struct _frozen *moddef = &_PyImport_FrozenModules[modidx];
    free((void*)moddef->name);
  }
  free(_PyImport_FrozenModules);
}

#ifdef WIN_UNICODE
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, wchar_t *lpCmdLine, int nCmdShow) {
  return wmain(__argc, __wargv);
}
#elif defined(_WIN32)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char *lpCmdLine, int nCmdShow) {
  return main(__argc, __argv);
}
#endif
