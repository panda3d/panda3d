#define PY_SSIZE_T_CLEAN
#include <Python.h>

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

/*
 * This pointer is kept internally to this module.  It represents the
 * locally-allocated FrozenModules array.  If the
 * PyImport_FrozenModules is any other value, then it wasn't allocated
 * via this module.
 */
static struct _frozen *frozen_modules = NULL;
static int num_frozen_modules = 0;

/*
 * Call this function to extend the frozen modules array with a new
 * array of frozen modules, provided in a C-style array, at runtime.
 * Returns the total number of frozen modules.
 */
static int 
extend_frozen_modules(const struct _frozen *new_modules, int new_count) {
  int orig_count;
  struct _frozen *realloc_FrozenModules;

  if (PyImport_FrozenModules == frozen_modules) {
    /* If the previous array was allocated through this module, we
       already know the count. */
    orig_count = num_frozen_modules;

  } else {
    /* If the previous array came from anywhere else, we have to count
       up its length. */
    orig_count = 0;
    while (PyImport_FrozenModules[orig_count].name != NULL) {
      ++orig_count;
    }
  }

  if (new_count == 0) {
    /* Trivial no-op. */
    return orig_count;
  }

  /* Reallocate the PyImport_FrozenModules array bigger to make room
     for the additional frozen modules. */
  realloc_FrozenModules = (struct _frozen *)malloc((orig_count + new_count + 1) * sizeof(struct _frozen));

  /* If the previous array was allocated through this module, we can
     free it; otherwise, we have to leak it. */
  if (frozen_modules != NULL) {
    free(frozen_modules);
    frozen_modules = NULL;
  }

  /* The new frozen modules go at the front of the list. */
  memcpy(realloc_FrozenModules, new_modules, new_count * sizeof(struct _frozen));

  /* Then the original set of frozen modules. */
  memcpy(realloc_FrozenModules + new_count, PyImport_FrozenModules, orig_count * sizeof(struct _frozen));

  /* Finally, a single 0-valued entry marks the end of the array. */
  memset(realloc_FrozenModules + orig_count + new_count, 0, sizeof(struct _frozen));

  /* Assign the new pointer. */
  PyImport_FrozenModules = realloc_FrozenModules;
  frozen_modules = realloc_FrozenModules;
  num_frozen_modules = orig_count + new_count;

  return num_frozen_modules;
}

/* 
 * Call this function to extend the frozen modules array with a new
 * list of frozen modules, provided in a Python-style list of (name,
 * code) tuples, at runtime.  This function is designed to be called
 * from Python.
 *
 * Returns the total number of frozen modules.
 */
static PyObject *
py_extend_frozen_modules(PyObject *self, PyObject *args) {
  PyObject *list;
  int num_elements;
  int i;
  struct _frozen *new_modules;
  int total_count;

  if (!PyArg_ParseTuple(args, "O", &list)) {
    return NULL;
  }

  if (!PySequence_Check(list)) {
    Py_DECREF(list);
    PyErr_SetString(PyExc_TypeError, "List required");
    return NULL;
  }

  num_elements = PySequence_Size(list);
  new_modules = (struct _frozen *)malloc(sizeof(struct _frozen) * num_elements);

  for (i = 0; i < num_elements; ++i) {
    PyObject *tuple;
    const char *name;
    const char *code;
    Py_ssize_t size;

    tuple = PySequence_GetItem(list, i);
    if (!PyArg_ParseTuple(tuple, "ss#", &name, &code, &size)) {
      return NULL;
    }

    /* We have to malloc new pointers for the name and code arrays.
       These pointers will never be freed. */
    new_modules[i].name = strdup(name);
    new_modules[i].code = (unsigned char *)malloc(size);
    new_modules[i].size = size;
    memcpy(new_modules[i].code, code, size);

    Py_DECREF(tuple);
  }

  Py_DECREF(list);

  total_count = extend_frozen_modules(new_modules, num_elements);
  free(new_modules);
  
  return Py_BuildValue("i", total_count);
}

/* 
 * Call this function to query whether the named module is already a
 * frozen module or not.
 */
static PyObject *
py_is_frozen_module(PyObject *self, PyObject *args) {
  const char *name;
  int i;

  if (!PyArg_ParseTuple(args, "s", &name)) {
    return NULL;
  }

  i = 0;
  while (PyImport_FrozenModules[i].name != NULL) {
    if (strcmp(PyImport_FrozenModules[i].name, name) == 0) {
      Py_INCREF(Py_True);
      return Py_True;
    }
    ++i;
  }

  Py_INCREF(Py_False);
  return Py_False;
}

/*
 * This returns the tuple (code, isPackage), where code is the code
 * string associated with the named frozen module, and isPackage is
 * true if the module is a package, or false if it is a normal
 * module).  The return value is None if there is no such frozen
 * module.  You must use the marshal module to convert the code string
 * to a code object.
 */ 
static PyObject *
py_get_frozen_module_code(PyObject *self, PyObject *args) {
  const char *name;
  int i;

  if (!PyArg_ParseTuple(args, "s", &name)) {
    return NULL;
  }

  i = 0;
  while (PyImport_FrozenModules[i].name != NULL) {
    if (strcmp(PyImport_FrozenModules[i].name, name) == 0) {
      int is_package = (PyImport_FrozenModules[i].size < 0);
      return Py_BuildValue("(s#i)", PyImport_FrozenModules[i].code,
                           (Py_ssize_t) abs(PyImport_FrozenModules[i].size),
                           is_package);
    }
    ++i;
  }

  return Py_BuildValue("");
}

/* 
 * Call this function to return a list of the existing frozen module
 * names.
 */
static PyObject *
py_get_frozen_module_names(PyObject *self, PyObject *args) {
  int i;
  PyObject *list;

  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  list = PyList_New(0);
  i = 0;
  while (PyImport_FrozenModules[i].name != NULL) {
    PyObject *name = PyString_FromString(PyImport_FrozenModules[i].name);
    PyList_Append(list, name);
    Py_DECREF(name);
    ++i;
  }

  return list;
}

/* Initializes the Python module with our functions. */
DLLEXPORT void initextend_frozen() {
  static PyMethodDef extend_frozen_methods[] = {
    { "extend", py_extend_frozen_modules, METH_VARARGS,
      "Adds new frozen modules at runtime." },
    { "is_frozen_module", py_is_frozen_module, METH_VARARGS,
      "Returns true if the named module is a frozen module." },
    { "get_frozen_module_code", py_get_frozen_module_code, METH_VARARGS,
      "Returns the code string associated with the named module." },
    { "get_frozen_module_names", py_get_frozen_module_names, METH_VARARGS,
      "Returns a list of frozen module names." },
    { NULL, NULL, 0, NULL }        /* Sentinel */
  };

  Py_InitModule("extend_frozen", extend_frozen_methods);
}
