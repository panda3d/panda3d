///////////////////////////////////////////////////////////////////////
//
// This is a little wrapper to make it easy to run a python
// program from the command line. Basically, it just interfaces
// to the Python API and imports the module that was specified
// by the IMPORT_MODULE preprocessor definition when it was compiled.
//
///////////////////////////////////////////////////////////////////////

#include "dtoolbase.h"

#include <Python.h>
#if PY_MAJOR_VERSION >= 3
#include <wchar.h>
#endif

#ifndef IMPORT_MODULE
#error IMPORT_MODULE must be defined when compiling ppython.cxx !
#endif

#define _STRINGIFY(s) #s
#define STRINGIFY(s) _STRINGIFY(s)
#define IMPORT_MODULE_STR STRINGIFY(IMPORT_MODULE)

#if defined(_WIN32) && PY_MAJOR_VERSION >= 3
// As Py_SetProgramName expects a wchar_t*,
// it's easiest to just use the wmain entry point.
int wmain(int argc, wchar_t *argv[]) {
  Py_SetProgramName(argv[0]);

#elif PY_MAJOR_VERSION >= 3
// Convert from UTF-8 to wchar_t*.
int main(int argc, char *mb_argv[]) {
  wchar_t **argv = new wchar_t*[argc + 1];
  for (int i = 0; i < argc; ++i) {
    size_t len = mbstowcs(NULL, mb_argv[i], 0);
    argv[i] = new wchar_t[len + 1];
    mbstowcs(argv[i], mb_argv[i], len);
    argv[i][len] = NULL;
  }
  // Just for good measure
  argv[argc] = NULL;

  Py_SetProgramName(argv[0]);

#else
// Python 2.
int main(int argc, char *argv[]) {
  Py_SetProgramName(argv[0]);
#endif

  // On Windows, we need to set pythonhome correctly. We'll try to
  // find ppython.exe on the path and set pythonhome to its location.
#ifdef _WIN32
#if PY_MAJOR_VERSION >= 3
  // Py_SetPythonHome expects a wchar_t in Python 3.
  wchar_t *path = _wgetenv(L"PATH");
  wchar_t *result = wcstok(path, L";");
  while (result != NULL) {
    struct _stat st;
    wchar_t *ppython = (wchar_t*) malloc(wcslen(result) * 2 + 26);
    wcscpy(ppython, result);
    wcscat(ppython, L"\\python.exe");
    if (_wstat(ppython, &st) == 0) {
      Py_SetPythonHome(result);
      free(ppython);
      break;
    }
    result = wcstok(NULL, L";");
    free(ppython);
  }
#else
  char *path = getenv("PATH");
  char *result = strtok(path, ";");
  while (result != NULL) {
    struct stat st;
    char *ppython = (char*) malloc(strlen(result) + 13);
    strcpy(ppython, result);
    strcat(ppython, "\\ppython.exe");
    if (stat(ppython, &st) == 0) {
      Py_SetPythonHome(result);
      free(ppython);
      break;
    }
    result = strtok(NULL, ";");
    free(ppython);
  }
#endif
#endif

  Py_Initialize();

  if (Py_VerboseFlag) {
    fprintf(stderr, "Python %s\\n%s\\n", Py_GetVersion(), Py_GetCopyright());
  }

  PySys_SetArgv(argc, argv);

  int sts = 0;
  PyObject* m = PyImport_ImportModule(IMPORT_MODULE_STR);
  if (m <= 0) {
    PyErr_Print();
    sts = 1;
  }

  Py_Finalize();
  return sts;
}
