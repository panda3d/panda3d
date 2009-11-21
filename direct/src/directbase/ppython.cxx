///////////////////////////////////////////////////////////////////////
//
// This is a little wrapper to make it easy to run a python
// program from the command line. Basically, it just interfaces
// to the Python API and imports the module that was specified
// by the IMPORT_MODULE preprocessor definition when it was compiled.
//
///////////////////////////////////////////////////////////////////////

#include <Python.h>

#ifndef IMPORT_MODULE
#error IMPORT_MODULE must be defined when compiling ppython.cxx !
#endif

#define _STRINGIFY(s) #s
#define STRINGIFY(s) _STRINGIFY(s)
#define IMPORT_MODULE_STR STRINGIFY(IMPORT_MODULE)

int main(int argc, char **argv) {
  int sts = 0;

  Py_SetProgramName(argv[0]);
  
  // On windows, we need to set pythonhome correctly. We'll try to
  // find ppython.exe on the path and set pythonhome to its location.
#ifdef _WIN32
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
  
  Py_Initialize();

  if (Py_VerboseFlag) {
    fprintf(stderr, "Python %s\\n%s\\n", Py_GetVersion(), Py_GetCopyright());
  }

  PySys_SetArgv(argc, argv);

  PyObject* m = PyImport_ImportModule(IMPORT_MODULE_STR);
  if (m <= 0) {
    PyErr_Print();
    sts = 1;
  }

  Py_Finalize();
  return sts;
}
