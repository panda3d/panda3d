#ifndef PY_PANDA_H_
#define PY_PANDA_H_
// Filename: py_panda.h
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//O
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//  Too do list ..
//      We need a better dispatcher for the functions..  The behavior today is 
//          try one till it works or you run out of possibilities..  This is anything but optimal 
//          for performance and is treading on thin ice for function python or c++ will 
//          course there types to other types.
//
//	The linking step will produce allot of warnings 
//			warning LNK4049: locally defined symbol..
//
//  Get a second coder to review this file and the generated  code ..
//
//////////////////////////////////////////////////////////////////////////////////////////////
#include <set>
#include <map>
#include <string>

#ifndef NO_RUNTIME_TYPES

#include "dtoolbase.h"
#include "typedObject.h"
#include "typeRegistry.h"

#endif

#ifdef HAVE_PYTHON

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

using namespace std;

///////////////////////////////////////////////////////////////////////////////////
// this is tempory .. untill this is glued better into the panda build system
///////////////////////////////////////////////////////////////////////////////////

#if defined(_WIN32) && !defined(LINK_ALL_STATIC)
#define EXPORT_THIS  __declspec(dllexport) 
#define IMPORT_THIS  extern __declspec(dllimport) 
#else
#define EXPORT_THIS 
#define IMPORT_THIS     extern
#endif
///////////////////////////////////////////////////////////////////////////////////

struct          Dtool_PyTypedObject;
typedef std::map< int , Dtool_PyTypedObject *>   RunTimeTypeDictionary;
typedef std::set<int >                           RunTimeTypeList;

EXPCL_DTOOLCONFIG   RunTimeTypeDictionary & GetRunTimeDictionary();
EXPCL_DTOOLCONFIG   RunTimeTypeList & GetRunTimeTypeList();


//////////////////////////////////////////////////////////
// used to stamp dtool instance.. 
#define PY_PANDA_SIGNITURE 0xdeadbeaf
typedef  void * ( * ConvertFunctionType  )(PyObject *,Dtool_PyTypedObject * );
typedef  void * ( * ConvertFunctionType1  )(void *, Dtool_PyTypedObject *);
typedef  void   ( *FreeFunction  )(PyObject *);
typedef  void   ( *PyModuleClassInit)(PyObject *module);
//inline          Dtool_PyTypedObject *  Dtool_RuntimeTypeDtoolType(int type);
inline void     Dtool_Deallocate_General(PyObject * self);
//inline int      DTOOL_PyObject_Compare(PyObject *v1, PyObject *v2);
//
////////////////////////////////////////////////////////////////////////
// THIS IS THE INSTANCE CONTAINER FOR ALL panda py objects....
////////////////////////////////////////////////////////////////////////
struct Dtool_PyInstDef
{
        PyObject_HEAD
        void                 * _ptr_to_object;
        bool                   _memory_rules;
        unsigned long          _signiture;
        struct Dtool_PyTypedObject * _My_Type;
};

////////////////////////////////////////////////////////////////////////
// A Offset Dictionary Definign How to read the Above Object..
////////////////////////////////////////////////////////////////////////
extern EXPCL_DTOOLCONFIG PyMemberDef standard_type_members[];

////////////////////////////////////////////////////////////////////////
// The Class Definition Structor For a Dtool python type.
////////////////////////////////////////////////////////////////////////
struct Dtool_PyTypedObject  
{
    // Standard Python Fearures..
    PyTypeObject        _PyType;
    //                  My Class Level Features..
    char *                  _name;                                          // cpp name for the object
    bool                    _Dtool_IsRunTimeCapable;                    // derived from TypedObject
    ConvertFunctionType     _Dtool_UpcastInterface;                    // The Upcast Function By Slot
    ConvertFunctionType1    _Dtool_DowncastInterface;                  // The Downcast Function By Slot
    FreeFunction            _Dtool_FreeInstance;
    PyModuleClassInit       _Dtool_ClassInit;                          // The init function pointer
    // some convience functions..
    inline PyTypeObject    & As_PyTypeObject(void) { return _PyType; };
    inline PyObject        & As_PyObject(void) { return (PyObject &)_PyType; };
};


////////////////////////////////////////////////////////////////////////
// Macro's from Hell..  May want to Just Add this to the Code generator..
////////////////////////////////////////////////////////////////////////
#define   Define_Dtool_Class(MODULE_NAME, CLASS_NAME, PUBLIC_NAME) \
static  PyNumberMethods     Dtool_PyNumberMethods_##CLASS_NAME ={\
	0,/*binaryfunc nb_add*/\
	0,/*binaryfunc nb_subtract*/\
	0,/*binaryfunc nb_multiply*/\
	0,/*binaryfunc nb_divide*/\
	0,/*binaryfunc nb_remainder*/\
	0,/*binaryfunc nb_divmod*/\
	0,/*ternaryfunc nb_power*/\
	0,/*unaryfunc nb_negative*/\
	0,/*unaryfunc nb_positive*/\
	0,/*unaryfunc nb_absolute*/\
	0,/*inquiry nb_nonzero*/\
	0,/*unaryfunc nb_invert*/\
	0,/*binaryfunc nb_lshift*/\
	0,/*binaryfunc nb_rshift*/\
	0,/*binaryfunc nb_and*/\
	0,/*binaryfunc nb_xor*/\
	0,/*binaryfunc nb_or*/\
	0,/*coercion nb_coerce*/\
	0,/*unaryfunc nb_int*/\
	0,/*unaryfunc nb_long*/\
	0,/*unaryfunc nb_float*/\
	0,/*unaryfunc nb_oct*/\
	0,/*unaryfunc nb_hex*/\
	0,/*binaryfunc nb_inplace_add*/\
	0,/*binaryfunc nb_inplace_subtract*/\
	0,/*binaryfunc nb_inplace_multiply*/\
	0,/*binaryfunc nb_inplace_divide*/\
	0,/*binaryfunc nb_inplace_remainder*/\
	0,/*ternaryfunc nb_inplace_power*/\
	0,/*binaryfunc nb_inplace_lshift*/\
	0,/*binaryfunc nb_inplace_rshift*/\
	0,/*binaryfunc nb_inplace_and*/\
	0,/*binaryfunc nb_inplace_xor*/\
	0,/*binaryfunc nb_inplace_or*/\
	0,/*binaryfunc nb_floor_divide*/\
	0,/*binaryfunc nb_true_divide*/\
	0,/*binaryfunc nb_inplace_floor_divide*/\
	0,/*binaryfunc nb_inplace_true_divide*/\
    };\
static  PyMappingMethods    Dtool_PyMappingMethods_##CLASS_NAME ={\
	0,/*inquiry mp_length */\
	0,/*binaryfunc mp_subscript */\
	0,/*objobjargproc mp_ass_subscript */\
};\
EXPORT_THIS Dtool_PyTypedObject Dtool_##CLASS_NAME =  {\
{\
    PyObject_HEAD_INIT(NULL)\
    0,\
    #MODULE_NAME "." #PUBLIC_NAME, /*type name with module */ \
    sizeof(Dtool_PyInstDef),     /* tp_basicsize*/ \
    0,     /*tp_itemsize*/ \
    &Dtool_Deallocate_General, /*Dtool_Deallocate_##CLASS_NAME,*/ /*tp_dealloc*/\
    0,              /*tp_print*/\
    0,              /*tp_getattr*/\
    0,              /*tp_setattr*/\
    0,              /*tp_compare*/\
    0,              /*tp_repr*/\
    &Dtool_PyNumberMethods_##CLASS_NAME,              /*tp_as_number*/\
    0,              /*tp_as_sequence*/\
    &Dtool_PyMappingMethods_##CLASS_NAME,              /*tp_as_mapping*/\
    0,     /*tp_hash */\
	0,					/* tp_call */\
	0,					/* tp_str */\
	PyObject_GenericGetAttr,		/* tp_getattro */\
	PyObject_GenericSetAttr,		/* tp_setattro */\
	0,					/* tp_as_buffer */\
	(Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_CHECKTYPES), /* tp_flags */\
	0,					/* tp_doc */\
	0,					/* tp_traverse */\
	0,					/* tp_clear */\
	0,					/* tp_richcompare */\
	0,					/* tp_weaklistoffset */\
	0,					/* tp_iter */\
	0,					/* tp_iternext */\
    Dtool_Methods_##CLASS_NAME,			/* tp_methods */\
	standard_type_members,					/* tp_members */\
	0,		    	    /* tp_getset */\
	0,      		    /* tp_base */\
	0,					/* tp_dict */\
	0,					/* tp_descr_get */\
	0,					/* tp_descr_set */\
	0,					/* tp_dictoffset */\
    Dtool_Init_##CLASS_NAME,		/* tp_init */\
	PyType_GenericAlloc, /* tp_alloc */\
	Dtool_new_##CLASS_NAME,			/* tp_new */\
	_PyObject_Del,					/* tp_free */\
},\
    #CLASS_NAME,   \
    false,\
    Dtool_UpcastInterface_##CLASS_NAME, \
    Dtool_DowncastInterface_##CLASS_NAME, \
    Dtool_FreeInstance_##CLASS_NAME, \
    Dtool_PyModuleClassInit_##CLASS_NAME\
}; 


////////////////////////////////////////////////////////////////////////
// The Fast Deallocator.. for Our instances..
////////////////////////////////////////////////////////////////////////
inline void    Dtool_Deallocate_General(PyObject * self)
{
    ((Dtool_PyInstDef *)self)->_My_Type->_Dtool_FreeInstance(self);
    self->ob_type->tp_free(self);
}
////////////////////////////////////////////////////////////////////////
//  More Macro(s) to Implement class functions.. Usally used if C++ needs type information 
////////////////////////////////////////////////////////////////////////
#define Define_Dtool_new(CLASS_NAME,CNAME)\
PyObject *Dtool_new_##CLASS_NAME(PyTypeObject *type, PyObject *args, PyObject *kwds)\
{\
	PyObject * self = type->tp_alloc(type, 0);\
    ((Dtool_PyInstDef *)self)->_signiture = PY_PANDA_SIGNITURE;\
    ((Dtool_PyInstDef *)self)->_ptr_to_object = NULL;\
    ((Dtool_PyInstDef *)self)->_memory_rules = false;\
    ((Dtool_PyInstDef *)self)->_My_Type = &Dtool_##CLASS_NAME;\
    return self;\
}\
////////////////////////////////////////////////////////////////////////
/// Delete functions..
////////////////////////////////////////////////////////////////////////
#define Define_Dtool_FreeInstance_Private(CLASS_NAME,CNAME)\
static void Dtool_FreeInstance_##CLASS_NAME(PyObject *self)\
{\
}\

#define Define_Dtool_FreeInstance(CLASS_NAME,CNAME)\
static void Dtool_FreeInstance_##CLASS_NAME(PyObject *self)\
{\
    if(((Dtool_PyInstDef *)self)->_ptr_to_object != NULL)\
        if(((Dtool_PyInstDef *)self)->_memory_rules)\
        {\
            delete ((CNAME *)((Dtool_PyInstDef *)self)->_ptr_to_object);\
        }\
}\

#define Define_Dtool_FreeInstanceRef(CLASS_NAME,CNAME)\
static void Dtool_FreeInstance_##CLASS_NAME(PyObject *self)\
{\
    if(((Dtool_PyInstDef *)self)->_ptr_to_object != NULL)\
        if(((Dtool_PyInstDef *)self)->_memory_rules)\
        {\
            unref_delete((CNAME *)((Dtool_PyInstDef *)self)->_ptr_to_object);\
        }\
}\

////////////////////////////////////////////////////////////////////////
/// Simple Recognition Functions..
////////////////////////////////////////////////////////////////////////
EXPCL_DTOOLCONFIG bool DtoolCanThisBeAPandaInstance(PyObject *self);

////////////////////////////////////////////////////////////////////////
//  Function : DTOOL_Call_ExtractThisPointerForType
//
//  These are the rapers that allow for down and upcast from type .. 
//      needed by the Dtool py interface.. Be very carefull if you muck with these
//      as the generated code depends on how this is set up..
////////////////////////////////////////////////////////////////////////
EXPCL_DTOOLCONFIG void DTOOL_Call_ExtractThisPointerForType(PyObject *self, Dtool_PyTypedObject * classdef, void ** answer);


EXPCL_DTOOLCONFIG void * DTOOL_Call_GetPointerThisClass(PyObject *self, Dtool_PyTypedObject  *classdef);

EXPCL_DTOOLCONFIG void * DTOOL_Call_GetPointerThis(PyObject *self);

////////////////////////////////////////////////////////////////////////
//  Function : DTool_CreatePyInstanceTyped
//
// this function relies on the behavior of typed objects in the panda system. 
//
////////////////////////////////////////////////////////////////////////
EXPCL_DTOOLCONFIG  PyObject * DTool_CreatePyInstanceTyped(void * local_this_in, Dtool_PyTypedObject & known_class_type, bool memory_rules, int RunTimeType);

////////////////////////////////////////////////////////////////////////
// DTool_CreatePyInstance .. wrapper function to finalize the existance of a general 
//    dtool py instance..
////////////////////////////////////////////////////////////////////////
EXPCL_DTOOLCONFIG  PyObject * DTool_CreatePyInstance(void * local_this, Dtool_PyTypedObject & in_classdef, bool memory_rules);

///////////////////////////////////////////////////////////////////////////////
//  Macro(s) class definition .. Used to allocate storage and 
//     init some values for a Dtool Py Type object.
/////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//struct Dtool_PyTypedObject Dtool_##CLASS_NAME;

#define Define_Module_Class_Internal(MODULE_NAME,CLASS_NAME,CNAME)\
extern EXPORT_THIS   Dtool_PyTypedObject Dtool_##CLASS_NAME;  \
extern struct        PyMethodDef Dtool_Methods_##CLASS_NAME[];\
int         Dtool_Init_##CLASS_NAME(PyObject *self, PyObject *args, PyObject *kwds);\
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
EXPCL_DTOOLCONFIG int  DTool_PyInit_Finalize(PyObject * self, void * This, Dtool_PyTypedObject *type, bool memory_rules);

///////////////////////////////////////////////////////////////////////////////
/// A heler function to glu methed definition together .. that can not be done at 
// code generation time becouse of multiple generation passes in interigate..
//
///////////////////////////////////////////////////////////////////////////////
typedef std::map<std::string, PyMethodDef *  > MethodDefmap;

EXPCL_DTOOLCONFIG void Dtool_Accum_MethDefs(PyMethodDef  in[], MethodDefmap &themap);

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
EXPCL_DTOOLCONFIG Dtool_PyTypedObject *  Dtool_RuntimeTypeDtoolType(int type);

///////////////////////////////////////////////////////////////////////////////
//// We need a way to runtime merge compile units into a python "Module" .. this is done with the 
/// fallowing structors and code.. along with the support of interigate_module 
///////////////////////////////////////////////////////////////////////////////
struct LibrayDef
{
    typedef  void   ( *ConstantFunction  )(PyObject *);

    PyMethodDef *           _methods;
    ConstantFunction        _constants;
};
///////////////////////////////////////////////////////////////////////////////

EXPCL_DTOOLCONFIG void Dtool_PyModuleInitHelper( LibrayDef   *defs[], char *  modulename);

///////////////////////////////////////////////////////////////////////////////
///  HACK.... Be carefull 
//
//  Dtool_BarrowThisRefrence
//      This function can be used to grab the "THIS" pointer from an object and use it
//      Required to support fom historical inharatence in the for of "is this instance of"..
//
///////////////////////////////////////////////////////////////////////////////
EXPCL_DTOOLCONFIG PyObject * Dtool_BarrowThisRefrence(PyObject * self, PyObject * args );

//////////////////////////////////////////////////////////////////////////////////////////////
// We do expose a dictionay for dtool classes .. this should be removed at some point..
//////////////////////////////////////////////////////////////////////////////////////////////
EXPCL_DTOOLCONFIG PyObject * Dtool_AddToDictionary(PyObject * self1, PyObject * args );

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

EXPCL_DTOOLCONFIG int DTOOL_PyObject_Compare_old(PyObject *v1, PyObject *v2);

EXPCL_DTOOLCONFIG int DTOOL_PyObject_Compare(PyObject *v1, PyObject *v2);

EXPCL_DTOOLCONFIG extern struct   Dtool_PyTypedObject Dtool_DTOOL_SUPPER_BASE;

#endif  // HAVE_PYTHON


#endif // PY_PANDA_H_ 

