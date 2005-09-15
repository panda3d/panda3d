// Filename: py_panda.cxx
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

#ifdef HAVE_PYTHON

PyMemberDef standard_type_members[] = {
	{"this", T_INT, offsetof(Dtool_PyInstDef,_ptr_to_object),READONLY,"C++ This if any"},
	{"this_ownership", T_INT, offsetof(Dtool_PyInstDef, _memory_rules), READONLY,"C++ 'this' ownership rules"},
	{"this_signiture", T_INT, offsetof(Dtool_PyInstDef, _signiture), READONLY,"A type check signiture"},
	{"this_metatype", T_OBJECT, offsetof(Dtool_PyInstDef, _My_Type), READONLY,"The dtool meta object"},
	{NULL}	/* Sentinel */
};


////////////////////////////////////////////////////////////////////////
/// Simple Recognition Functions..
////////////////////////////////////////////////////////////////////////
bool DtoolCanThisBeAPandaInstance(PyObject *self)
{
    // simple sanity check for the class type..size.. will stop basic foobars..
    if(self->ob_type->tp_basicsize >= (int)sizeof(Dtool_PyInstDef))
    {
        Dtool_PyInstDef * pyself = (Dtool_PyInstDef *) self;
        if(pyself->_signiture == PY_PANDA_SIGNITURE)
            return true;
    }
    return false;
}
////////////////////////////////////////////////////////////////////////
//  Function : DTOOL_Call_ExtractThisPointerForType
//
//  These are the rapers that allow for down and upcast from type .. 
//      needed by the Dtool py interface.. Be very carefull if you muck with these
//      as the generated code depends on how this is set up..
////////////////////////////////////////////////////////////////////////
void DTOOL_Call_ExtractThisPointerForType(PyObject *self, Dtool_PyTypedObject * classdef, void ** answer)
{
    if(DtoolCanThisBeAPandaInstance(self))
        *answer = ((Dtool_PyInstDef *)self)->_My_Type->_Dtool_UpcastInterface(self,classdef);
    else
        answer = NULL;
};

void *
DTOOL_Call_GetPointerThisClass(PyObject *self, Dtool_PyTypedObject *classdef,
                               int param, const string &function_name) {
  if (self != NULL) {
    if (DtoolCanThisBeAPandaInstance(self)) {
      Dtool_PyTypedObject *my_type = ((Dtool_PyInstDef *)self)->_My_Type;
      void *result = my_type->_Dtool_UpcastInterface(self, classdef);
      if (result != NULL) {
        return result;
      }

      ostringstream str;
      str << function_name << "() argument " << param << " must be "
          << classdef->_name << ", not " << my_type->_name;
      string msg = str.str();
      PyErr_SetString(PyExc_TypeError, msg.c_str());

    } else {
      ostringstream str;
      str << function_name << "() argument " << param << " must be "
          << classdef->_name;
      PyObject *tname = PyObject_GetAttrString((PyObject *)self->ob_type, "__name__");
      if (tname != (PyObject *)NULL) {
        str << ", not " << PyString_AsString(tname);
        Py_DECREF(tname);
      }

      string msg = str.str();
      PyErr_SetString(PyExc_TypeError, msg.c_str());
    }
  } else {
    PyErr_SetString(PyExc_TypeError, "Self Is Null"); 
  }

  return NULL;
}

void * DTOOL_Call_GetPointerThis(PyObject *self)
{
  if(self != NULL)
  {
      if(DtoolCanThisBeAPandaInstance(self))
      {
        Dtool_PyInstDef * pyself = (Dtool_PyInstDef *) self;
        return pyself->_ptr_to_object;
      }
  }

  return NULL;
};

////////////////////////////////////////////////////////////////////////
//  Function : DTool_CreatePyInstanceTyped
//
// this function relies on the behavior of typed objects in the panda system. 
//
////////////////////////////////////////////////////////////////////////
PyObject * DTool_CreatePyInstanceTyped(void * local_this_in, Dtool_PyTypedObject & known_class_type, bool memory_rules, int RunTimeType)
{     
    if(local_this_in == NULL )
    {
        // Lets don't be stupid..
        PyErr_SetString(PyExc_TypeError, "C Function Return Null 'this'");
        return NULL;
    }
   /////////////////////////////////////////////////////
   // IF the calss is posibly a run time typed object
   /////////////////////////////////////////////////////
    if(RunTimeType > 0)
    {
       /////////////////////////////////////////////////////
       // get best fit class...
       /////////////////////////////////////////////////////
        Dtool_PyTypedObject * target_class = Dtool_RuntimeTypeDtoolType(RunTimeType);
        if(target_class != NULL)
        {
           /////////////////////////////////////////////////////
           // cast to the type...
           //////////////////////////////////////////////////////
            void * new_local_this = target_class->_Dtool_DowncastInterface(local_this_in,&known_class_type);
            if(new_local_this != NULL)
            {
                /////////////////////////////////////////////
                // ask class to allocate a instance..
                /////////////////////////////////////////////
                Dtool_PyInstDef * self = (Dtool_PyInstDef *) target_class->As_PyTypeObject().tp_new(&target_class->As_PyTypeObject(), NULL,NULL);
                if(self != NULL)
                {
                    self->_ptr_to_object = new_local_this;
                    self->_memory_rules = memory_rules;
                    self->_signiture = PY_PANDA_SIGNITURE;
                    self->_My_Type = target_class;    
                    return (PyObject *)self;
                }             
            }
        }
    }

    /////////////////////////////////////////////////////
    // if we get this far .. just wrap the thing in the known type ??
    //    better than aborting...I guess....
    /////////////////////////////////////////////////////
	Dtool_PyInstDef * self = (Dtool_PyInstDef *) known_class_type.As_PyTypeObject().tp_new(&known_class_type.As_PyTypeObject(), NULL,NULL);
    if(self != NULL)
    {
        self->_ptr_to_object = local_this_in;
        self->_memory_rules = memory_rules;
        self->_signiture = PY_PANDA_SIGNITURE;
        self->_My_Type = &known_class_type;    
    }
    return (PyObject *)self;
};

////////////////////////////////////////////////////////////////////////
// DTool_CreatePyInstance .. wrapper function to finalize the existance of a general 
//    dtool py instance..
////////////////////////////////////////////////////////////////////////
PyObject * DTool_CreatePyInstance(void * local_this, Dtool_PyTypedObject & in_classdef, bool memory_rules)
{    
    if(local_this == NULL)
    {
        PyErr_SetString(PyExc_TypeError, "C Function Return Null 'this' ");
        return NULL;
    }

    Dtool_PyTypedObject * classdef = &in_classdef;
    Dtool_PyInstDef * self = (Dtool_PyInstDef *) classdef->As_PyTypeObject().tp_new(&classdef->As_PyTypeObject(), NULL,NULL);
    if(self != NULL)
    {
        self->_ptr_to_object = local_this;
        self->_memory_rules = memory_rules;
        self->_My_Type = classdef;    
    } 
    return (PyObject *)self;
};

///////////////////////////////////////////////////////////////////////////////
/// Th Finalizer for simple instances..
///////////////////////////////////////////////////////////////////////////////
int  DTool_PyInit_Finalize(PyObject * self, void * This, Dtool_PyTypedObject *type, bool memory_rules)
{
    // lets put some code in here that checks to see the memory is properly configured..
    // prior to my call ..

    ((Dtool_PyInstDef *)self)->_My_Type = type;
    ((Dtool_PyInstDef *)self)->_ptr_to_object = This;
    ((Dtool_PyInstDef *)self)->_memory_rules = memory_rules;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// A heler function to glu methed definition together .. that can not be done at 
// code generation time becouse of multiple generation passes in interigate..
//
///////////////////////////////////////////////////////////////////////////////

void Dtool_Accum_MethDefs(PyMethodDef  in[], MethodDefmap &themap)
{
    for(; in->ml_name != NULL; in++)
    {
        if(themap.find(in->ml_name) == themap.end())
        {
            themap[in->ml_name] = in;
        }
    }
}   

///////////////////////////////////////////////////////////////////////////////
//  ** HACK ** allert..
//
//      Need to keep a runtime type dictionary ... that is forward declared of typed object.
//        We rely on the fact that typed objects are uniquly defined by an integer.
//
///////////////////////////////////////////////////////////////////////////////

void
RegisterRuntimeClass(Dtool_PyTypedObject * otype, int class_id) {
  if (class_id == 0) {
    interrogatedb_cat.warning()
      << "Class " << otype->_name 
      << " has a zero TypeHandle value; check that init_type() is called.\n";

  } else if (class_id > 0) {
    RunTimeTypeDictionary &dict = GetRunTimeDictionary();
    pair<RunTimeTypeDictionary::iterator, bool> result =
      dict.insert(RunTimeTypeDictionary::value_type(class_id, otype));
    if (!result.second) {
      // There was already an entry in the dictionary for class_id.
      Dtool_PyTypedObject *other_type = (*result.first).second;
      interrogatedb_cat.warning()
	<< "Classes " << otype->_name << " and " << other_type->_name
	<< " share the same TypeHandle value (" << class_id 
	<< "); check class definitions.\n";

    } else {
      GetRunTimeTypeList().insert(class_id);
      otype->_Dtool_IsRunTimeCapable = true;
    }
  }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
Dtool_PyTypedObject *  Dtool_RuntimeTypeDtoolType(int type)
{
    RunTimeTypeDictionary::iterator di = GetRunTimeDictionary().find(type);
    if(di != GetRunTimeDictionary().end())
        return di->second;
    else
    {
        int  type2 = get_best_parent_from_Set(type,GetRunTimeTypeList());
        di = GetRunTimeDictionary().find(type2);
        if(di != GetRunTimeDictionary().end())
            return di->second;
    }
    return NULL;    
};

///////////////////////////////////////////////////////////////////////////////
void Dtool_PyModuleInitHelper( LibrayDef   *defs[], char *  modulename)
{
    // the module level function inits....
    MethodDefmap  functions;
    for(int xx = 0; defs[xx] != NULL; xx++)
        Dtool_Accum_MethDefs(defs[xx]->_methods,functions);

    PyMethodDef  *newdef = new PyMethodDef[functions.size()+1];
    MethodDefmap::iterator mi;
    int     offset = 0;
    for(mi = functions.begin(); mi != functions.end(); mi++, offset++)
        newdef[offset] = *mi->second;
    newdef[offset].ml_doc = NULL;
    newdef[offset].ml_name = NULL;
    newdef[offset].ml_meth = NULL;
    newdef[offset].ml_flags = 0;

    PyObject * module = Py_InitModule(modulename,newdef);   

    if(module == NULL)
    {
         PyErr_SetString(PyExc_TypeError, "Py_InitModule Returned NULL ???"); 
         return;
    }


    // the constant inits... enums, classes ...
    for(int y = 0; defs[y] != NULL; y++)
        defs[y]->_constants(module);

    PyModule_AddIntConstant(module,"Dtool_PyNavtiveInterface",1);
}
///////////////////////////////////////////////////////////////////////////////
///  HACK.... Be carefull 
//
//  Dtool_BarrowThisRefrence
//      This function can be used to grab the "THIS" pointer from an object and use it
//      Required to support fom historical inharatence in the for of "is this instance of"..
//
///////////////////////////////////////////////////////////////////////////////
PyObject * Dtool_BarrowThisRefrence(PyObject * self, PyObject * args )
{
    PyObject *from_in = NULL;
    PyObject *to_in = NULL;
    if(PyArg_ParseTuple(args, "OO", &to_in, &from_in)) 
    {

        if(DtoolCanThisBeAPandaInstance(from_in) && DtoolCanThisBeAPandaInstance(to_in))
        {
            Dtool_PyInstDef * from = (Dtool_PyInstDef *) from_in;
            Dtool_PyInstDef * to = (Dtool_PyInstDef *) to_in;
            if(from->_My_Type == to->_My_Type)
            {
                to->_memory_rules = false;
                to->_ptr_to_object = from->_ptr_to_object;
                return Py_BuildValue("");
            }
            PyErr_SetString(PyExc_TypeError, "Must Be Same Type??"); 
        }
        else
            PyErr_SetString(PyExc_TypeError, "One of thesee does not appear to be DTOOL Instance ??"); 
    }
    return (PyObject *) NULL;
}
//////////////////////////////////////////////////////////////////////////////////////////////
// We do expose a dictionay for dtool classes .. this should be removed at some point..
//////////////////////////////////////////////////////////////////////////////////////////////
PyObject * Dtool_AddToDictionary(PyObject * self1, PyObject * args )
{

    PyObject  *     self;
    PyObject  *     subject;
    PyObject  *     key;
    if(PyArg_ParseTuple(args, "OSO", &self, &key, &subject))
    {
        PyObject * dict = ((PyTypeObject *)self)->tp_dict;
        if(dict == NULL && !PyDict_Check(dict))
            PyErr_SetString(PyExc_TypeError, "No dictionary On Object");
        else
            PyDict_SetItem(dict,key,subject);

    }   
    if(PyErr_Occurred())
        return (PyObject *)NULL;
    return Py_BuildValue("");

}
///////////////////////////////////////////////////////////////////////////////////
/*
inline long  DTool_HashKey(PyObject * inst)
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

int DTOOL_PyObject_Compare_old(PyObject *v1, PyObject *v2)
{
    // if we are related..
    if(PyType_IsSubtype(v1->ob_type, v2->ob_type)) 
    {
        void * v1_this = DTOOL_Call_GetPointerThis(v1);
        void * v2_this = DTOOL_Call_GetPointerThis(v2);
        if(v1_this != NULL && v2_this != NULL) // both are our types...
        {
            PyObject * func = PyObject_GetAttrString(v1, "compareTo");
            if (func == NULL)
            {
                PyErr_Clear();
            }
            else
            {
                PyObject * res = NULL;
                PyObject * args = Py_BuildValue("(O)", v2);
                if (args != NULL)
                {
                    res = PyObject_Call(func, args, NULL);
                    Py_DECREF(args);
                }
                Py_DECREF(func);
              	PyErr_Clear(); // just in case the function threw an error
                // only use if the cuntion  return an INT... hmm
                if(res != NULL && PyInt_Check(res))
                {
                    int answer = PyInt_AsLong(res);
                    Py_DECREF(res);
                    return  answer;
                }
                if(res != NULL)
                    Py_DECREF(res);

            };
            // CompareTo Failed some how :(
            // do a this compare  .. if Posible...
            if(v1_this < v2_this)
                return -1;

            if(v1_this > v2_this)
                return 1;
            return 0;
        }
        // ok drop to a basic object compare hmmmmmm
    }
    if(v1 < v2)
        return  -1;
    if(v1 > v2)
        return  1;
    return 0;   
}




int DTOOL_PyObject_Compare(PyObject *v1, PyObject *v2)
{
    //  First try compare to function..
    PyObject * func = PyObject_GetAttrString(v1, "compareTo");
    if (func == NULL)
    {
        PyErr_Clear();
    }
    else
    {
        PyObject * res = NULL;
        PyObject * args = Py_BuildValue("(O)", v2);
        if (args != NULL)
        {
            res = PyObject_Call(func, args, NULL);
            Py_DECREF(args);
        }
        Py_DECREF(func);
        PyErr_Clear(); // just in case the function threw an error
        // only use if the cuntion  return an INT... hmm
        if(res != NULL && PyInt_Check(res))
        {
            int answer = PyInt_AsLong(res);
            Py_DECREF(res);
            return  answer;
        }
        if(res != NULL)
            Py_DECREF(res);

    };

    // try this compare
    void * v1_this = DTOOL_Call_GetPointerThis(v1);
    void * v2_this = DTOOL_Call_GetPointerThis(v2);
    if(v1_this != NULL && v2_this != NULL) // both are our types...
    {
        if(v1_this < v2_this)
            return -1;

        if(v1_this > v2_this)
            return 1;
        return 0;
    }

    // ok self compare...
    if(v1 < v2)
        return  -1;
    if(v1 > v2)
        return  1;
    return 0;   
}

#endif  // HAVE_PYTHON
