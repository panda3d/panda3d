#ifndef PY_PANDA_H_
#define PY_PANDA_H_
// Filename: py_panda.h
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//  Too do list ..
//      We need a better dispatcher for the functions..  The behavior today is 
//          try one till it works or you run out of possibilities..  This is anything but optimal 
//          for performance and is treading on thin ice for function python or c++ will 
//          course there types to other types.
//
//      The linking step will produce allot of warnings 
//                      warning LNK4049: locally defined symbol..
//
//  Get a second coder to review this file and the generated  code ..
//
//////////////////////////////////////////////////////////////////////////////////////////////
#include <set>
#include <map>
#include <string>

#ifdef USE_DEBUG_PYTHON
#define  Py_DEBUG
#endif

#ifndef NO_RUNTIME_TYPES

#include "dtoolbase.h"
#include "typedObject.h"
#include "typeRegistry.h"

#endif

#if defined(HAVE_PYTHON) && !defined(CPPPARSER)

#ifdef HAVE_LONG_LONG
#undef HAVE_LONG_LONG
#endif 
#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif

#include "Python.h"
#include "structmember.h"
#ifdef HAVE_LONG_LONG
#undef HAVE_LONG_LONG
#endif 

#if PY_VERSION_HEX < 0x02050000

// Prior to Python 2.5, we didn't have Py_ssize_t.
typedef int Py_ssize_t;
#define PyInt_FromSsize_t PyInt_FromLong
#define PyInt_AsSsize_t PyInt_AsLong

#endif  // PY_VERSION_HEX

// 2.4 macros which aren't available in 2.3
#ifndef Py_RETURN_NONE
inline PyObject* doPy_RETURN_NONE()
{   Py_INCREF(Py_None); return Py_None; }
#define Py_RETURN_NONE return doPy_RETURN_NONE()
#endif

#ifndef Py_RETURN_TRUE
inline PyObject* doPy_RETURN_TRUE()
{Py_INCREF(Py_True); return Py_True;}
#define Py_RETURN_TRUE return doPy_RETURN_TRUE()
#endif

#ifndef Py_RETURN_FALSE
inline PyObject* doPy_RETURN_FALSE()
{Py_INCREF(Py_False); return Py_False;}
#define Py_RETURN_FALSE return doPy_RETURN_FALSE()
#endif

#ifndef PyVarObject_HEAD_INIT
#define PyVarObject_HEAD_INIT(type, size) \
  PyObject_HEAD_INIT(type) size,
#endif

#ifndef Py_TYPE
#define Py_TYPE(ob) (((PyObject*)(ob))->ob_type)
#endif

#ifndef Py_TPFLAGS_CHECKTYPES
// Always on in Python 3
#define Py_TPFLAGS_CHECKTYPES 0
#endif

#if PY_MAJOR_VERSION < 3
// For more portably defining hash functions.
typedef long Py_hash_t;
#endif

#if PY_MAJOR_VERSION >= 3
#define nb_nonzero nb_bool
#define nb_divide nb_true_divide
#define nb_inplace_divide nb_inplace_true_divide
#endif

using namespace std;

#define PY_PANDA_SMALLER_FOOTPRINT 1

///////////////////////////////////////////////////////////////////////////////////
// this is tempory .. untill this is glued better into the panda build system
///////////////////////////////////////////////////////////////////////////////////

#if defined(_WIN32) && !defined(LINK_ALL_STATIC)
#define EXPORT_THIS __declspec(dllexport)
#define IMPORT_THIS extern __declspec(dllimport)
#else
#define EXPORT_THIS
#define IMPORT_THIS extern
#endif
///////////////////////////////////////////////////////////////////////////////////

struct Dtool_PyTypedObject;
typedef std::map<int, Dtool_PyTypedObject *>   RunTimeTypeDictionary;
typedef std::set<int>                           RunTimeTypeList;

EXPCL_DTOOLCONFIG RunTimeTypeDictionary &GetRunTimeDictionary();
EXPCL_DTOOLCONFIG RunTimeTypeList &GetRunTimeTypeList();

//////////////////////////////////////////////////////////
// used to stamp dtool instance.. 
#define PY_PANDA_SIGNATURE 0xbeaf
typedef void * ( * ConvertFunctionType  )(PyObject *,Dtool_PyTypedObject * );
typedef void * ( * ConvertFunctionType1  )(void *, Dtool_PyTypedObject *);
typedef void   ( *FreeFunction  )(PyObject *);
typedef void   ( *PyModuleClassInit)(PyObject *module);
typedef int    ( *InitNoCoerce)(PyObject *self, PyObject *args, PyObject *kwds);

//inline          Dtool_PyTypedObject *  Dtool_RuntimeTypeDtoolType(int type);
inline void     Dtool_Deallocate_General(PyObject * self);
//inline int      DTOOL_PyObject_Compare(PyObject *v1, PyObject *v2);
//
////////////////////////////////////////////////////////////////////////
// THIS IS THE INSTANCE CONTAINER FOR ALL panda py objects....
////////////////////////////////////////////////////////////////////////
#ifdef  PY_PANDA_SMALLER_FOOTPRINT
// this should save   8 bytes per object ....
struct Dtool_PyInstDef {
  PyObject_HEAD
  void *_ptr_to_object;
  struct Dtool_PyTypedObject *_My_Type;
  unsigned short _signature ; 
  int _memory_rules : 1;   // true if we own the pointer and should delete it or unref it
  int _is_const     : 1;       // true if this is a "const" pointer.
};

#else
struct Dtool_PyInstDef {
  PyObject_HEAD
  void *_ptr_to_object;
  int _memory_rules;   // true if we own the pointer and should delete it or unref it
  int _is_const;       // true if this is a "const" pointer.
  unsigned long _signature;
  struct Dtool_PyTypedObject *_My_Type;
};
#endif

////////////////////////////////////////////////////////////////////////
// A Offset Dictionary Defining How to read the Above Object..
////////////////////////////////////////////////////////////////////////
extern EXPCL_DTOOLCONFIG PyMemberDef standard_type_members[];

////////////////////////////////////////////////////////////////////////
// The Class Definition Structor For a Dtool python type.
////////////////////////////////////////////////////////////////////////
struct Dtool_PyTypedObject {
  // Standard Python Features..
  PyTypeObject _PyType;

  // My Class Level Features..
  const char *_name;                             // cpp name for the object
  bool _Dtool_IsRunTimeCapable;                  // derived from TypedObject
  ConvertFunctionType _Dtool_UpcastInterface;    // The Upcast Function By Slot
  ConvertFunctionType1 _Dtool_DowncastInterface; // The Downcast Function By Slot
  FreeFunction _Dtool_FreeInstance;
  PyModuleClassInit _Dtool_ClassInit;            // The module init function pointer
  InitNoCoerce _Dtool_InitNoCoerce;              // A variant of the constructor that does not attempt to perform coercion of its arguments.

  // some convenience functions..
  inline PyTypeObject &As_PyTypeObject() { return _PyType; };
  inline PyObject &As_PyObject() { return (PyObject &)_PyType; };
};

////////////////////////////////////////////////////////////////////////
// Macros from Hell..  May want to just add this to the code generator..
////////////////////////////////////////////////////////////////////////
#define Define_Dtool_PyTypedObject(MODULE_NAME, CLASS_NAME, PUBLIC_NAME) \
  EXPORT_THIS Dtool_PyTypedObject Dtool_##CLASS_NAME =                  \
    {                                                                   \
      {                                                                 \
        PyVarObject_HEAD_INIT(NULL, 0)                                  \
        #MODULE_NAME "." #PUBLIC_NAME,       /*type name with module */ \
        sizeof(Dtool_PyInstDef),                /* tp_basicsize */      \
        0,                                      /* tp_itemsize */       \
        &Dtool_Deallocate_General,              /* tp_dealloc */        \
        0,                                      /* tp_print */          \
        0,                                      /* tp_getattr */        \
        0,                                      /* tp_setattr */        \
        0,                                      /* tp_compare */        \
        0,                                      /* tp_repr */           \
        &Dtool_PyNumberMethods_##CLASS_NAME,    /* tp_as_number */      \
        &Dtool_PySequenceMethods_##CLASS_NAME,  /* tp_as_sequence */    \
        &Dtool_PyMappingMethods_##CLASS_NAME,   /* tp_as_mapping */     \
        0,                                      /* tp_hash */           \
        0,                                      /* tp_call */           \
        0,                                      /* tp_str */            \
        PyObject_GenericGetAttr,                /* tp_getattro */       \
        PyObject_GenericSetAttr,                /* tp_setattro */       \
        &Dtool_PyBufferProcs_##CLASS_NAME,      /* tp_as_buffer */      \
        (Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_CHECKTYPES), /* tp_flags */ \
        0,                                      /* tp_doc */            \
        0,                                      /* tp_traverse */       \
        0,                                      /* tp_clear */          \
        0,                                      /* tp_richcompare */    \
        0,                                      /* tp_weaklistoffset */ \
        0,                                      /* tp_iter */           \
        0,                                      /* tp_iternext */       \
        Dtool_Methods_##CLASS_NAME,             /* tp_methods */        \
        standard_type_members,                  /* tp_members */        \
        0,                                      /* tp_getset */         \
        0,                                      /* tp_base */           \
        0,                                      /* tp_dict */           \
        0,                                      /* tp_descr_get */      \
        0,                                      /* tp_descr_set */      \
        0,                                      /* tp_dictoffset */     \
        Dtool_Init_##CLASS_NAME,                /* tp_init */           \
        PyType_GenericAlloc,                    /* tp_alloc */          \
        Dtool_new_##CLASS_NAME,                 /* tp_new */            \
        PyObject_Del,                           /* tp_free */           \
      },                                                                \
      #CLASS_NAME,                                                      \
      false,                                                            \
      Dtool_UpcastInterface_##CLASS_NAME,                               \
      Dtool_DowncastInterface_##CLASS_NAME,                             \
      Dtool_FreeInstance_##CLASS_NAME,                                  \
      Dtool_PyModuleClassInit_##CLASS_NAME,                             \
      Dtool_InitNoCoerce_##CLASS_NAME                                   \
    };

#define Define_Dtool_Class(MODULE_NAME, CLASS_NAME, PUBLIC_NAME)        \
  static PyNumberMethods Dtool_PyNumberMethods_##CLASS_NAME = {0};      \
  static PySequenceMethods Dtool_PySequenceMethods_##CLASS_NAME = {0};  \
  static PyMappingMethods Dtool_PyMappingMethods_##CLASS_NAME = {0};    \
  static PyBufferProcs Dtool_PyBufferProcs_##CLASS_NAME = {0};          \
  Define_Dtool_PyTypedObject(MODULE_NAME, CLASS_NAME, PUBLIC_NAME)

////////////////////////////////////////////////////////////////////////
// The Fast Deallocator.. for Our instances..
////////////////////////////////////////////////////////////////////////
inline void Dtool_Deallocate_General(PyObject * self) {
  ((Dtool_PyInstDef *)self)->_My_Type->_Dtool_FreeInstance(self);
  Py_TYPE(self)->tp_free(self);
}

////////////////////////////////////////////////////////////////////////
//  More Macro(s) to Implement class functions.. Usually used if C++ needs type information 
////////////////////////////////////////////////////////////////////////
#define Define_Dtool_new(CLASS_NAME,CNAME)\
PyObject *Dtool_new_##CLASS_NAME(PyTypeObject *type, PyObject *args, PyObject *kwds)\
{\
    PyObject *self = type->tp_alloc(type, 0);\
    ((Dtool_PyInstDef *)self)->_signature = PY_PANDA_SIGNATURE;\
    ((Dtool_PyInstDef *)self)->_ptr_to_object = NULL;\
    ((Dtool_PyInstDef *)self)->_memory_rules = false;\
    ((Dtool_PyInstDef *)self)->_is_const = false;\
    ((Dtool_PyInstDef *)self)->_My_Type = &Dtool_##CLASS_NAME;\
    return self;\
}

////////////////////////////////////////////////////////////////////////
/// Delete functions..
////////////////////////////////////////////////////////////////////////
#ifdef NDEBUG
#define Define_Dtool_FreeInstance_Private(CLASS_NAME,CNAME)\
static void Dtool_FreeInstance_##CLASS_NAME(PyObject *self)\
{\
}
#else // NDEBUG
#define Define_Dtool_FreeInstance_Private(CLASS_NAME,CNAME)\
static void Dtool_FreeInstance_##CLASS_NAME(PyObject *self)\
{\
    if(((Dtool_PyInstDef *)self)->_ptr_to_object != NULL)\
        if(((Dtool_PyInstDef *)self)->_memory_rules)\
        {\
          cerr << "Detected leak for " << #CLASS_NAME \
               << " which interrogate cannot delete.\n"; \
        }\
}
#endif  // NDEBUG

#define Define_Dtool_FreeInstance(CLASS_NAME,CNAME)\
static void Dtool_FreeInstance_##CLASS_NAME(PyObject *self)\
{\
    if(((Dtool_PyInstDef *)self)->_ptr_to_object != NULL)\
        if(((Dtool_PyInstDef *)self)->_memory_rules)\
        {\
            delete ((CNAME *)((Dtool_PyInstDef *)self)->_ptr_to_object);\
        }\
}

#define Define_Dtool_FreeInstanceRef(CLASS_NAME,CNAME)\
static void Dtool_FreeInstance_##CLASS_NAME(PyObject *self)\
{\
    if(((Dtool_PyInstDef *)self)->_ptr_to_object != NULL)\
        if(((Dtool_PyInstDef *)self)->_memory_rules)\
        {\
            unref_delete((CNAME *)((Dtool_PyInstDef *)self)->_ptr_to_object);\
        }\
}

////////////////////////////////////////////////////////////////////////
/// Simple Recognition Functions..
////////////////////////////////////////////////////////////////////////
EXPCL_DTOOLCONFIG bool DtoolCanThisBeAPandaInstance(PyObject *self);

////////////////////////////////////////////////////////////////////////
//  Function : DTOOL_Call_ExtractThisPointerForType
//
//  These are the wrappers that allow for down and upcast from type .. 
//      needed by the Dtool py interface.. Be very careful if you muck
//      with these as the generated code depends on how this is set
//      up..
////////////////////////////////////////////////////////////////////////
EXPCL_DTOOLCONFIG void DTOOL_Call_ExtractThisPointerForType(PyObject *self, Dtool_PyTypedObject * classdef, void ** answer);

EXPCL_DTOOLCONFIG void *DTOOL_Call_GetPointerThisClass(PyObject *self, Dtool_PyTypedObject *classdef, int param, const string &function_name, bool const_ok, PyObject **coerced, bool report_errors);

EXPCL_DTOOLCONFIG void *DTOOL_Call_GetPointerThisClass(PyObject *self, Dtool_PyTypedObject *classdef, int param, const string &function_name, bool const_ok, PyObject **coerced);

EXPCL_DTOOLCONFIG void *DTOOL_Call_GetPointerThis(PyObject *self);

////////////////////////////////////////////////////////////////////////
//  Function : DTool_CreatePyInstanceTyped
//
// this function relies on the behavior of typed objects in the panda system. 
//
////////////////////////////////////////////////////////////////////////
EXPCL_DTOOLCONFIG PyObject *DTool_CreatePyInstanceTyped(void *local_this_in, Dtool_PyTypedObject &known_class_type, bool memory_rules, bool is_const, int RunTimeType);

////////////////////////////////////////////////////////////////////////
// DTool_CreatePyInstance .. wrapper function to finalize the existance of a general 
//    dtool py instance..
////////////////////////////////////////////////////////////////////////
EXPCL_DTOOLCONFIG PyObject *DTool_CreatePyInstance(void *local_this, Dtool_PyTypedObject &in_classdef, bool memory_rules, bool is_const);

///////////////////////////////////////////////////////////////////////////////
//  Macro(s) class definition .. Used to allocate storage and 
//     init some values for a Dtool Py Type object.
/////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//struct Dtool_PyTypedObject Dtool_##CLASS_NAME;

#define Define_Module_Class_Internal(MODULE_NAME,CLASS_NAME,CNAME)\
extern EXPORT_THIS   Dtool_PyTypedObject Dtool_##CLASS_NAME;\
extern struct        PyMethodDef Dtool_Methods_##CLASS_NAME[];\
int         Dtool_Init_##CLASS_NAME(PyObject *self, PyObject *args, PyObject *kwds);\
int         Dtool_InitNoCoerce_##CLASS_NAME(PyObject *self, PyObject *args, PyObject *kwds);\
PyObject *  Dtool_new_##CLASS_NAME(PyTypeObject *type, PyObject *args, PyObject *kwds);\
void  *     Dtool_UpcastInterface_##CLASS_NAME(PyObject *self, Dtool_PyTypedObject *requested_type);\
void  *     Dtool_DowncastInterface_##CLASS_NAME(void *self, Dtool_PyTypedObject *requested_type);\
void        Dtool_PyModuleClassInit_##CLASS_NAME(PyObject *module);

///////////////////////////////////////////////////////////////////////////////
#define Define_Module_Class(MODULE_NAME,CLASS_NAME,CNAME,PUBLIC_NAME)\
Define_Module_Class_Internal(MODULE_NAME,CLASS_NAME,CNAME)\
Define_Dtool_new(CLASS_NAME,CNAME)\
Define_Dtool_FreeInstance(CLASS_NAME,CNAME)\
Define_Dtool_Class(MODULE_NAME,CLASS_NAME,PUBLIC_NAME)

///////////////////////////////////////////////////////////////////////////////
#define Define_Module_Class_Private(MODULE_NAME,CLASS_NAME,CNAME,PUBLIC_NAME)\
Define_Module_Class_Internal(MODULE_NAME,CLASS_NAME,CNAME)\
Define_Dtool_new(CLASS_NAME,CNAME)\
Define_Dtool_FreeInstance_Private(CLASS_NAME,CNAME)\
Define_Dtool_Class(MODULE_NAME,CLASS_NAME,PUBLIC_NAME)

///////////////////////////////////////////////////////////////////////////////
#define Define_Module_ClassRef_Private(MODULE_NAME,CLASS_NAME,CNAME,PUBLIC_NAME)\
Define_Module_Class_Internal(MODULE_NAME,CLASS_NAME,CNAME)\
Define_Dtool_new(CLASS_NAME,CNAME)\
Define_Dtool_FreeInstance_Private(CLASS_NAME,CNAME)\
Define_Dtool_Class(MODULE_NAME,CLASS_NAME,PUBLIC_NAME)

///////////////////////////////////////////////////////////////////////////////
#define Define_Module_ClassRef(MODULE_NAME,CLASS_NAME,CNAME,PUBLIC_NAME)\
Define_Module_Class_Internal(MODULE_NAME,CLASS_NAME,CNAME)\
Define_Dtool_new(CLASS_NAME,CNAME)\
Define_Dtool_FreeInstanceRef(CLASS_NAME,CNAME)\
Define_Dtool_Class(MODULE_NAME,CLASS_NAME,PUBLIC_NAME)


///////////////////////////////////////////////////////////////////////////////
/// Th Finalizer for simple instances..
///////////////////////////////////////////////////////////////////////////////
EXPCL_DTOOLCONFIG int DTool_PyInit_Finalize(PyObject *self, void *This, Dtool_PyTypedObject *type, bool memory_rules, bool is_const);

///////////////////////////////////////////////////////////////////////////////
/// A heler function to glu methed definition together .. that can not be done at 
// code generation time becouse of multiple generation passes in interigate..
//
///////////////////////////////////////////////////////////////////////////////
typedef std::map<std::string, PyMethodDef *> MethodDefmap;

EXPCL_DTOOLCONFIG void Dtool_Accum_MethDefs(PyMethodDef in[], MethodDefmap &themap);

///////////////////////////////////////////////////////////////////////////////
//  ** HACK ** allert..
//
//      Need to keep a runtime type dictionary ... that is forward declared of typed object.
//        We rely on the fact that typed objects are uniquly defined by an integer.
//
///////////////////////////////////////////////////////////////////////////////

EXPCL_DTOOLCONFIG void RegisterRuntimeClass(Dtool_PyTypedObject * otype, int class_id);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
EXPCL_DTOOLCONFIG Dtool_PyTypedObject *Dtool_RuntimeTypeDtoolType(int type);

///////////////////////////////////////////////////////////////////////////////
//// We need a way to runtime merge compile units into a python "Module" .. this is done with the 
/// fallowing structors and code.. along with the support of interigate_module 
///////////////////////////////////////////////////////////////////////////////
struct LibraryDef {
  typedef void (*ConstantFunction)(PyObject *);

  PyMethodDef *_methods;
  ConstantFunction _constants;
};
///////////////////////////////////////////////////////////////////////////////

#if PY_MAJOR_VERSION >= 3
EXPCL_DTOOLCONFIG PyObject *Dtool_PyModuleInitHelper(LibraryDef *defs[], PyModuleDef *module_def);
#else
EXPCL_DTOOLCONFIG PyObject *Dtool_PyModuleInitHelper(LibraryDef *defs[], const char *modulename);
#endif

///////////////////////////////////////////////////////////////////////////////
///  HACK.... Be carefull 
//
//  Dtool_BorrowThisReference
//      This function can be used to grab the "THIS" pointer from an object and use it
//      Required to support fom historical inharatence in the for of "is this instance of"..
//
///////////////////////////////////////////////////////////////////////////////
EXPCL_DTOOLCONFIG PyObject *Dtool_BorrowThisReference(PyObject *self, PyObject *args);

//////////////////////////////////////////////////////////////////////////////////////////////
// We do expose a dictionay for dtool classes .. this should be removed at some point..
//////////////////////////////////////////////////////////////////////////////////////////////
EXPCL_DTOOLCONFIG PyObject *Dtool_AddToDictionary(PyObject *self1, PyObject *args);

///////////////////////////////////////////////////////////////////////////////////
/*
EXPCL_DTOOLCONFIG long  DTool_HashKey(PyObject * inst)
{
    long   outcome = (long)inst;
    PyObject * func = PyObject_GetAttrString(inst, "__hash__");
    if (func == NULL) 
    {
        if(DtoolCanThisBeAPandaInstance(inst))
            if(((Dtool_PyInstDef *)inst)->_ptr_to_object != NULL)
                outcome =  (long)((Dtool_PyInstDef *)inst)->_ptr_to_object;
    }
    else
    {
        PyObject *res = PyEval_CallObject(func, (PyObject *)NULL);
        Py_DECREF(func);
        if (res == NULL)
            return -1;
        if (PyInt_Check(res)) 
        {
            outcome = PyInt_AsLong(res);
            if (outcome == -1)
                outcome = -2;
        }
        else 
        {
            PyErr_SetString(PyExc_TypeError,
                "__hash__() should return an int");
            outcome = -1;
        }
        Py_DECREF(res);
    }
    return outcome;
}
*/

/* Compare v to w.  Return
   -1 if v <  w or exception (PyErr_Occurred() true in latter case).
    0 if v == w.
    1 if v > w.
   XXX The docs (C API manual) say the return value is undefined in case
   XXX of error.
*/

EXPCL_DTOOLCONFIG int DTOOL_PyObject_Compare(PyObject *v1, PyObject *v2);

EXPCL_DTOOLCONFIG PyObject *DTOOL_PyObject_RichCompare(PyObject *v1, PyObject *v2, int op);

EXPCL_DTOOLCONFIG PyObject *
make_list_for_item(PyObject *self, const char *num_name,
                   const char *element_name);

EXPCL_DTOOLCONFIG PyObject *
copy_from_make_copy(PyObject *self);

EXPCL_DTOOLCONFIG PyObject *
copy_from_copy_constructor(PyObject *self);

EXPCL_DTOOLCONFIG PyObject *
map_deepcopy_to_copy(PyObject *self, PyObject *args);

EXPCL_DTOOLCONFIG PyObject *
PyLongOrInt_FromUnsignedLong(unsigned long value);

EXPCL_DTOOLCONFIG extern struct Dtool_PyTypedObject Dtool_DTOOL_SUPER_BASE;

#endif  // HAVE_PYTHON && !CPPPARSER

#endif // PY_PANDA_H_ 
