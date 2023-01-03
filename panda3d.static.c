#include "Python.h"

extern PyMODINIT_FUNC PyInit_core(void);
extern PyMODINIT_FUNC PyInit_direct(void);


static PyObject *
mod_panda3d_import_cython(PyObject *self, PyObject *spec)
{
    //load_submodule_mphase("panda3d.xxx", PyInit_xxx(), spec, "xxx");
    Py_RETURN_NONE;
}

static PyMethodDef
mod_panda3d_static_methods[] = {
    {"import_cython", (PyCFunction)mod_panda3d_import_cython, METH_O, "panda3d.xxx"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef
mod_panda3d_static = {
    PyModuleDef_HEAD_INIT,
   "panda3d_static", NULL, -1,
   mod_panda3d_static_methods
};


void
load_submodule(const char *parent, PyObject *mod, const char *alias)
{
    char fqn[1024];
    snprintf(fqn, sizeof(fqn), "%s.%s", parent, alias);

    PyObject *modules = PyImport_GetModuleDict();

    PyObject *pmod = PyDict_GetItemString(modules, parent);

    if (!mod) {
        snprintf(fqn, sizeof(fqn), "ERROR: %s.%s", parent, alias);
        puts(fqn);
        PyErr_Print();
        PyErr_Clear();
    }
    else {
        PyDict_SetItemString(modules, fqn, mod);
        PyDict_SetItemString(PyModule_GetDict(mod), "__name__",
                             PyUnicode_FromString(fqn));
        PyModule_AddObjectRef(pmod, alias, mod);
        Py_XDECREF(mod);
    }
}


PyMODINIT_FUNC PyInit_static(void) {
    load_submodule("panda3d", PyInit_core(), "core");
    load_submodule("panda3d", PyInit_direct(), "direct");
    return PyModule_Create(&mod_panda3d_static);
}


