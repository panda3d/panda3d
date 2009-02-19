// Filename: py_panda.cxx
// Created by:  drose (04Jul05)
//
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

#include "py_panda.h"
#include "config_interrogatedb.h"

#ifdef HAVE_PYTHON

PyMemberDef standard_type_members[] = {
  {(char *)"this", T_INT, offsetof(Dtool_PyInstDef,_ptr_to_object),READONLY, (char *)"C++ This if any"},
//  {(char *)"this_ownership", T_INT, offsetof(Dtool_PyInstDef, _memory_rules), READONLY, (char *)"C++ 'this' ownership rules"},
//  {(char *)"this_const", T_INT, offsetof(Dtool_PyInstDef, _is_const), READONLY, (char *)"C++ 'this' const flag"},
//  {(char *)"this_signature", T_INT, offsetof(Dtool_PyInstDef, _signature), READONLY, (char *)"A type check signature"},
  {(char *)"this_metatype", T_OBJECT, offsetof(Dtool_PyInstDef, _My_Type), READONLY, (char *)"The dtool meta object"},
  {NULL}  /* Sentinel */
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
        if(pyself->_signature == PY_PANDA_SIGNATURE)
            return true;
    }
    return false;
}
////////////////////////////////////////////////////////////////////////
//  Function : DTOOL_Call_ExtractThisPointerForType
//
//  These are the wrappers that allow for down and upcast from type .. 
//      needed by the Dtool py interface.. Be very careful if you muck with these
//      as the generated code depends on how this is set up..
////////////////////////////////////////////////////////////////////////
void DTOOL_Call_ExtractThisPointerForType(PyObject *self, Dtool_PyTypedObject * classdef, void ** answer)
{
    if(DtoolCanThisBeAPandaInstance(self))
        *answer = ((Dtool_PyInstDef *)self)->_My_Type->_Dtool_UpcastInterface(self,classdef);
    else
        answer = NULL;
};

////////////////////////////////////////////////////////////////////
//     Function: attempt_coercion
//  Description: A helper function for DTOOL_Call_GetPointerThisClass,
//               below.  This attempts to coerce the given object to
//               the indicated Panda object, by creating a temporary
//               instance of the required Panda object.  If
//               successful, returns the "this" pointer of the
//               temporary object; otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
static void *
attempt_coercion(PyObject *self, Dtool_PyTypedObject *classdef,
                 PyObject **coerced) {
  // The supplied parameter is not the required type.
  if (coerced != NULL) {
    // Attempt coercion: try to create a temporary instance of the
    // required class using the supplied parameter.
    PyObject *obj = PyObject_Call((PyObject *)classdef, self, NULL);
    if (obj != NULL) {
      // Well, whaddaya know?  The supplied parameter(s) suited
      // the object's constructor.  Now we have a temporary object
      // that we can pass to the function.
      Dtool_PyTypedObject *my_type = ((Dtool_PyInstDef *)obj)->_My_Type;
      void *result = my_type->_Dtool_UpcastInterface(obj, classdef);
      if (result != NULL) {
        // Successfully coerced.  Store the newly-allocated
        // pointer, so the caller can release the coerced object
        // at his leisure.  We store it in a list, so that other
        // parameters can accumulate there too.
        if ((*coerced) == NULL) {
          (*coerced) = PyList_New(0);
        }
        PyList_Append(*coerced, obj);
        return result;
      }
      // Some problem getting the C++ pointer from our created
      // temporary object.  Weird.
      Py_DECREF(obj);
    }
    
    // Clear the error returned by the coercion constructor.  It's not
    // the error message we want to report.
    PyErr_Clear();
  }
  return NULL;
}

// Temporary function to preserve backward compatibility.
void *
DTOOL_Call_GetPointerThisClass(PyObject *self, Dtool_PyTypedObject *classdef,
                               int param, const string &function_name, bool const_ok,
                               PyObject **coerced) {
  return DTOOL_Call_GetPointerThisClass(self, classdef,
                                        param, function_name, const_ok,
                                        coerced, true);
}

////////////////////////////////////////////////////////////////////
//     Function: DTOOL_Call_GetPointerThisClass
//  Description: Extracts the C++ pointer for an object, given its
//               Python wrapper object, for passing as the parameter
//               to a C++ function.
//
//               self is the Python wrapper object in question.
//
//               classdef is the Python class wrapper for the C++
//               class in which the this pointer should be returned.
//               (This may require an upcast operation, if self is not
//               already an instance of classdef.)
//
//               param and function_name are used for error reporting
//               only, and describe the particular function and
//               parameter index for this parameter.
//
//               const_ok is true if the function is declared const
//               and can therefore be called with either a const or
//               non-const "this" pointer, or false if the function is
//               declared non-const, and can therefore be called with
//               only a non-const "this" pointer.
//
//               If coerced is non-NULL, parameter coercion will be
//               attempted.  This means the supplied parameter may not
//               exactly match the required type, but will satisfy the
//               require type's constructor; and we will create
//               temporary object(s) of the required type instead.  In
//               this case, coerced is a pointer to a PyList that will
//               be filled with these temporary objects.  If coerced
//               is a pointer to a NULL PyObject, a new PyList will be
//               created on the first successful coercion.  If coerced
//               itself is NULL, parameter coercion will not be
//               attempted.
//
//               The return value is the C++ pointer that was
//               extracted, or NULL if there was a problem (in which
//               case the Python exception state will have been set).
////////////////////////////////////////////////////////////////////
void *
DTOOL_Call_GetPointerThisClass(PyObject *self, Dtool_PyTypedObject *classdef,
                               int param, const string &function_name, bool const_ok,
                               PyObject **coerced, bool report_errors) {
  if (PyErr_Occurred()) {
    return NULL;
  }
  if (self != NULL) {
    if (DtoolCanThisBeAPandaInstance(self)) {
      Dtool_PyTypedObject *my_type = ((Dtool_PyInstDef *)self)->_My_Type;
      void *result = my_type->_Dtool_UpcastInterface(self, classdef);
      if (result != NULL) {
        if (const_ok || !((Dtool_PyInstDef *)self)->_is_const) {
          return result;
        }

        if (report_errors) {
          ostringstream str;
          str << function_name << "() argument " << param << " may not be const";
          string msg = str.str();
          PyErr_SetString(PyExc_TypeError, msg.c_str());
        }

      } else {
        if (report_errors) {
          ostringstream str;
          str << function_name << "() argument " << param << " must be ";
          
          
          PyObject *fname = PyObject_GetAttrString((PyObject *)classdef, "__name__");
          if (fname != (PyObject *)NULL) {
            str << PyString_AsString(fname);
            Py_DECREF(fname);
          } else {
            str << classdef->_name;
          }
          
          PyObject *tname = PyObject_GetAttrString((PyObject *)self->ob_type, "__name__");
          if (tname != (PyObject *)NULL) {
            str << ", not " << PyString_AsString(tname);
            Py_DECREF(tname);
          } else {
            str << ", not " << my_type->_name;
          }
          
          string msg = str.str();
          PyErr_SetString(PyExc_TypeError, msg.c_str());
        }
      }

    } else {
      // The parameter was not a Panda type.  Can we coerce it to the
      // appropriate type, by creating a temporary object?
      void *result = attempt_coercion(self, classdef, coerced);
      if (result != NULL) {
        return result;
      }

      // Coercion failed.
      if (report_errors) {
        ostringstream str;
        str << function_name << "() argument " << param << " must be ";
        
        PyObject *fname = PyObject_GetAttrString((PyObject *)classdef, "__name__");
        if (fname != (PyObject *)NULL) {
          str << PyString_AsString(fname);
          Py_DECREF(fname);
        } else {
          str << classdef->_name;
        }
        
        PyObject *tname = PyObject_GetAttrString((PyObject *)self->ob_type, "__name__");
        if (tname != (PyObject *)NULL) {
          str << ", not " << PyString_AsString(tname);
          Py_DECREF(tname);
        }
        
        string msg = str.str();
        PyErr_SetString(PyExc_TypeError, msg.c_str());
      }
    }
  } else {
    if (report_errors) {
      PyErr_SetString(PyExc_TypeError, "Self Is Null"); 
    }
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
PyObject * DTool_CreatePyInstanceTyped(void * local_this_in, Dtool_PyTypedObject & known_class_type, bool memory_rules, bool is_const, int RunTimeType)
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
                    self->_is_const = is_const;
                    self->_signature = PY_PANDA_SIGNATURE;
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
        self->_is_const = is_const;
        self->_signature = PY_PANDA_SIGNATURE;
        self->_My_Type = &known_class_type;    
    }
    return (PyObject *)self;
};

////////////////////////////////////////////////////////////////////////
// DTool_CreatePyInstance .. wrapper function to finalize the existance of a general 
//    dtool py instance..
////////////////////////////////////////////////////////////////////////
PyObject * DTool_CreatePyInstance(void * local_this, Dtool_PyTypedObject & in_classdef, bool memory_rules, bool is_const)
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
        self->_is_const = is_const;
        self->_My_Type = classdef;    
    } 
    return (PyObject *)self;
};

///////////////////////////////////////////////////////////////////////////////
/// Th Finalizer for simple instances..
///////////////////////////////////////////////////////////////////////////////
int  DTool_PyInit_Finalize(PyObject * self, void * This, Dtool_PyTypedObject *type, bool memory_rules, bool is_const)
{
    // lets put some code in here that checks to see the memory is properly configured..
    // prior to my call ..

    ((Dtool_PyInstDef *)self)->_My_Type = type;
    ((Dtool_PyInstDef *)self)->_ptr_to_object = This;
    ((Dtool_PyInstDef *)self)->_memory_rules = memory_rules;
    ((Dtool_PyInstDef *)self)->_is_const = is_const;
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
void Dtool_PyModuleInitHelper( LibrayDef   *defs[], const char *  modulename)
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

    PyObject * module = Py_InitModule((char *)modulename,newdef);   

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
//  Dtool_BorrowThisReference
//      This function can be used to grab the "THIS" pointer from an object and use it
//      Required to support historical inheritance in the form of "is this instance of"..
//
///////////////////////////////////////////////////////////////////////////////
PyObject * Dtool_BorrowThisReference(PyObject * self, PyObject * args )
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
                to->_is_const = from->_is_const;
                to->_ptr_to_object = from->_ptr_to_object;
                return Py_BuildValue("");
            }
            PyErr_SetString(PyExc_TypeError, "Must Be Same Type??"); 
        }
        else
            PyErr_SetString(PyExc_TypeError, "One of these does not appear to be DTOOL Instance ??"); 
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
            // do a this compare  .. if Possible...
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

            // Python really wants us to return strictly -1, 0, or 1.
            if (answer < 0) {
              return -1;
            } else if (answer > 0) {
              return 1;
            } else {
              return 0;
            }
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


PyObject *make_list_for_item(PyObject *self, const char *num_name,
                             const char *element_name) {
  PyObject *num_result = PyObject_CallMethod(self, (char *)num_name, (char *)"()");
  if (num_result == NULL) {
    return NULL;
  }
  Py_ssize_t num_elements = PyInt_AsSsize_t(num_result);
  Py_DECREF(num_result);
  
  PyObject *list = PyList_New(num_elements);
  for (int i = 0; i < num_elements; ++i) {
    PyObject *element = PyObject_CallMethod(self, (char *)element_name, (char *)"(i)", i);
    if (element == NULL) {
      Py_DECREF(list);
      return NULL;
    }
    PyList_SetItem(list, i, element);
  }
  return list;
}

////////////////////////////////////////////////////////////////////
//     Function: PyLongOrInt_FromUnsignedLong
//  Description: Similar to PyLong_FromUnsignedLong(), but returns
//               either a regular integer or a long integer, according
//               to whether the indicated value will fit.
////////////////////////////////////////////////////////////////////
EXPCL_DTOOLCONFIG PyObject *
PyLongOrInt_FromUnsignedLong(unsigned long value) {
  if ((long)value < 0) {
    return PyLong_FromUnsignedLong(value);
  } else {
    return PyInt_FromLong((long)value);
  }
}


#endif  // HAVE_PYTHON
