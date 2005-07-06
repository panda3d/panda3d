// Filename: dtool_super_base.cxx
// Created by:  drose (04Jul05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "py_panda.h"
  
class EmptyClass
{
};
Define_Module_Class_Private(dtoolconfig,DTOOL_SUPPER_BASE,EmptyClass,DTOOL_SUPPER_BASE111);

static PyObject * GetSupperBase(PyObject * self)
{
    Py_INCREF(&(Dtool_DTOOL_SUPPER_BASE.As_PyTypeObject())); // order is important .. this is used for static functions
    return  (PyObject *)&Dtool_DTOOL_SUPPER_BASE;
};


PyMethodDef Dtool_Methods_DTOOL_SUPPER_BASE[]= {
  { "DtoolGetSupperBase",(PyCFunction ) &GetSupperBase, METH_NOARGS,"Will Return SUPPERbase Class"},
  { NULL, NULL }
};

static long  DTool_HashKey_Methods_DTOOL_SUPPER_BASE(PyObject * self)
{
    void * local_this =DTOOL_Call_GetPointerThis(self);
    if(local_this == NULL)
    {
       return -1;
    };
    return (long)local_this;
};


inline void Dtool_PyModuleClassInit_DTOOL_SUPPER_BASE(PyObject *module)
{
    static bool initdone = false;
    if(!initdone)
    {

        initdone = true;
        Dtool_DTOOL_SUPPER_BASE.As_PyTypeObject().tp_dict = PyDict_New();
        PyDict_SetItemString(Dtool_DTOOL_SUPPER_BASE.As_PyTypeObject().tp_dict,"DtoolClassDict",Dtool_DTOOL_SUPPER_BASE.As_PyTypeObject().tp_dict);

        // __hash__
        Dtool_DTOOL_SUPPER_BASE.As_PyTypeObject().tp_hash = &DTool_HashKey_Methods_DTOOL_SUPPER_BASE;
        Dtool_DTOOL_SUPPER_BASE.As_PyTypeObject().tp_compare = &DTOOL_PyObject_Compare;

        if(PyType_Ready(&Dtool_DTOOL_SUPPER_BASE.As_PyTypeObject()) < 0)
        {
             PyErr_SetString(PyExc_TypeError, "PyType_Ready(Dtool_DTOOL_SUPPER_BASE)");
             return;
        }
        Py_INCREF(&Dtool_DTOOL_SUPPER_BASE.As_PyTypeObject());

        PyDict_SetItemString(Dtool_DTOOL_SUPPER_BASE.As_PyTypeObject().tp_dict,"DtoolGetSupperBase",PyCFunction_New(&Dtool_Methods_DTOOL_SUPPER_BASE[0],&Dtool_DTOOL_SUPPER_BASE.As_PyObject()));

    }

    if(module != NULL)
    {
        Py_INCREF(&Dtool_DTOOL_SUPPER_BASE.As_PyTypeObject());
        PyModule_AddObject(module, "DTOOL_SUPPER_BASE",(PyObject *)&Dtool_DTOOL_SUPPER_BASE.As_PyTypeObject());
    }
}

inline void  * Dtool_DowncastInterface_DTOOL_SUPPER_BASE(void *from_this, Dtool_PyTypedObject *from_type)
{
    return (void *) NULL;
}

inline void  * Dtool_UpcastInterface_DTOOL_SUPPER_BASE(PyObject *self, Dtool_PyTypedObject *requested_type)
{
    return NULL;
}

int  Dtool_Init_DTOOL_SUPPER_BASE(PyObject *self, PyObject *args, PyObject *kwds)
{
       PyErr_SetString(PyExc_TypeError, "Error Can Not Init SUPPER BASE");
       return -1;
}
