/* Python interpreter main program for frozen scripts */

#include "Python.h"
#ifdef _WIN32
#include "malloc.h"
#endif

#include <stdio.h>

#if PY_MAJOR_VERSION >= 3
#include <locale.h>
#endif

#ifdef MS_WINDOWS
extern void PyWinFreeze_ExeInit(void);
extern void PyWinFreeze_ExeTerm(void);

extern DL_IMPORT(int) PyImport_ExtendInittab(struct _inittab *newtab);
#endif

static unsigned char *modblob = NULL;

/* Main program */

int
Py_FrozenMain(int argc, char **argv)
{
    char *p;
    int n, sts = 1;
    int inspect = 0;
    int unbuffered = 0;

#if PY_MAJOR_VERSION >= 3
    int i;
    char *oldloc = NULL;
    wchar_t **argv_copy = NULL;
    /* We need a second copies, as Python might modify the first one. */
    wchar_t **argv_copy2 = NULL;

    if (argc > 0) {
        argv_copy = PyMem_RawMalloc(sizeof(wchar_t*) * argc);
        argv_copy2 = PyMem_RawMalloc(sizeof(wchar_t*) * argc);
        if (!argv_copy || !argv_copy2) {
            fprintf(stderr, "out of memory\n");
            goto error;
        }
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

#if PY_MAJOR_VERSION >= 3
    oldloc = _PyMem_RawStrdup(setlocale(LC_ALL, NULL));
    if (!oldloc) {
        fprintf(stderr, "out of memory\n");
        goto error;
    }

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
    PyMem_RawFree(oldloc);
    oldloc = NULL;
#endif

#ifdef MS_WINDOWS
    PyImport_ExtendInittab(extensions);
#endif /* MS_WINDOWS */

    if (argc >= 1) {
#if PY_MAJOR_VERSION >= 3
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

#if PY_MAJOR_VERSION >= 3
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

#if PY_MAJOR_VERSION >= 3
    n = PyImport_ImportFrozenModule("_frozen_importlib");
    if (n == 0)
        Py_FatalError("_frozen_importlib not frozen");
    if (n < 0)
        PyErr_Print();
#endif

    if (inspect && isatty((int)fileno(stdin)))
        sts = PyRun_AnyFile(stdin, "<stdin>") != 0;

#ifdef MS_WINDOWS
    PyWinFreeze_ExeTerm();
#endif
    Py_Finalize();

#if PY_MAJOR_VERSION >= 3
error:
    PyMem_RawFree(argv_copy);
    if (argv_copy2) {
        for (i = 0; i < argc; i++)
            PyMem_RawFree(argv_copy2[i]);
        PyMem_RawFree(argv_copy2);
    }
    PyMem_RawFree(oldloc);
#endif
    return sts;
}


int
main(int argc, char *argv[]) {
  struct _frozen *_PyImport_FrozenModules;
  unsigned int listoff, modsoff, fsize, modsize, listsize, nummods, modidx;
  FILE *runtime = fopen(argv[0], "rb");

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
    char *name = NULL, namebuf[256] = {0};
    unsigned int nsize, codeptr;
    int codesize;

    // Name
    fread(namebuf, 1, 256, runtime);
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

  // Uncomment this to print out the read in module list
  //for (unsigned int modidx = 0; modidx < nummods + 1; ++modidx) {
  //  struct _frozen *moddef = &_PyImport_FrozenModules[modidx];
  //  printf("MOD: %s %p %d\n", moddef->name, (void*)moddef->code, moddef->size);
  //}
  fclose(runtime);

  PyImport_FrozenModules = _PyImport_FrozenModules;
  return Py_FrozenMain(argc, argv);
}
