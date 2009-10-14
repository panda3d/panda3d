// Filename: ppPandaObject.cxx
// Created by:  drose (03Jul09)
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

#include "ppPandaObject.h"

NPClass PPPandaObject::_object_class = {
  NP_CLASS_STRUCT_VERSION,
  &PPPandaObject::NPAllocate,
  &PPPandaObject::NPDeallocate,
  &PPPandaObject::NPInvalidate,
  &PPPandaObject::NPHasMethod,
  &PPPandaObject::NPInvoke,
  &PPPandaObject::NPInvokeDefault,
  &PPPandaObject::NPHasProperty,
  &PPPandaObject::NPGetProperty,
  &PPPandaObject::NPSetProperty,
  &PPPandaObject::NPRemoveProperty,
#if defined(NP_CLASS_STRUCT_VERSION_ENUM) && NP_CLASS_STRUCT_VERSION >= NP_CLASS_STRUCT_VERSION_ENUM
  &PPPandaObject::NPEnumerate,
#endif
#if defined(NP_CLASS_STRUCT_VERSION_CTOR) && NP_CLASS_STRUCT_VERSION >= NP_CLASS_STRUCT_VERSION_CTOR
  &PPPandaObject::NPConstruct,
#endif
};


////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::make_new
//       Access: Public, Static
//  Description: Use this call to construct a new PPPandaObject.
////////////////////////////////////////////////////////////////////
PPPandaObject *PPPandaObject::
make_new(PPInstance *inst, P3D_object *p3d_object) {
  NPObject *npobj = 
    browser->createobject(inst->get_npp_instance(), &_object_class);
  PPPandaObject *ppobj = (PPPandaObject *)npobj;
  ppobj->construct(inst, p3d_object);
  return ppobj;
}

////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::set_p3d_object
//       Access: Public
//  Description: Changes the p3d_object this PPPandaObject maps to.  The
//               new object's reference count is incremented, and the
//               previous object's is decremented.
////////////////////////////////////////////////////////////////////
void PPPandaObject::
set_p3d_object(P3D_object *p3d_object) {
  if (p3d_object != NULL) {
    P3D_OBJECT_INCREF(p3d_object);
  }
  P3D_OBJECT_XDECREF(_p3d_object);
  _p3d_object = p3d_object;
}
 
////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::construct
//       Access: Private
//  Description: Stands in for the C++ constructor.  We can't have a
//               true constructor because of the C-style interface in
//               NPN_CreateObject().  This must be called explicitly
//               following NPN_CreateObject().
////////////////////////////////////////////////////////////////////
void PPPandaObject::
construct(PPInstance *inst, P3D_object *p3d_object) {
  _instance = inst;
  _p3d_object = NULL;
  set_p3d_object(p3d_object);
}

////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::invalidate
//       Access: Private
//  Description: This "destructor" is called by NPInvalidate().
////////////////////////////////////////////////////////////////////
void PPPandaObject::
invalidate() {
  _instance = NULL;
  set_p3d_object(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::has_method
//       Access: Private
//  Description: Returns true if the object has the named method,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool PPPandaObject::
has_method(NPIdentifier name) {
  string method_name = identifier_to_string(name);
  //nout << this << ".has_method(" << method_name << ")\n";
  if (_p3d_object == NULL) {
    // Not powered up yet.
    return false;
  }

  // Unlike has_property(), below, it turns out that we really do need
  // to honestly answer whether there is a method by this name,
  // because if there is, then Firefox won't query the property in a
  // meaningful fashion.

  // Of course, in Python the distinction between property and method
  // is a little looser than Firefox seems to want to make it, and
  // sometimes you have an object which is both.  This could become
  // problematic in obscure situations.  Too bad, say I.  Mozilla's
  // bug, not mine.

  bool result = P3D_OBJECT_HAS_METHOD(_p3d_object, method_name.c_str());
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::invoke
//       Access: Private
//  Description: Calls the named method on the object, storing the
//               return value into result.  Returns true on success,
//               false on failure.
////////////////////////////////////////////////////////////////////
bool PPPandaObject::
invoke(NPIdentifier name, const NPVariant *args, uint32_t argCount,
       NPVariant *result) {
  string method_name = identifier_to_string(name);
  //nout << this << ".invoke(" << method_name << ")\n";
  if (_p3d_object == NULL) {
    // Not powered up yet.
    return false;
  }

  P3D_object **p3dargs = new P3D_object *[argCount];
  unsigned int i;
  for (i = 0; i < argCount; ++i) {
    p3dargs[i] = _instance->variant_to_p3dobj(&args[i]);
  }

  P3D_object *value = P3D_OBJECT_CALL(_p3d_object, method_name.c_str(), 
                                      true, p3dargs, argCount);
  for (i = 0; i < argCount; ++i) {
    P3D_OBJECT_DECREF(p3dargs[i]);
  }
  delete[] p3dargs;

  if (value == NULL) {
    // No such method, or some problem with the parameters.
    return false;
  }

  // We have the return value, and its value is stored in value.
  _instance->p3dobj_to_variant(result, value);
  P3D_OBJECT_DECREF(value);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::invoke_default
//       Access: Private
//  Description: Calls the default method on the object, storing the
//               return value into result.  Returns true on success,
//               false on failure.
////////////////////////////////////////////////////////////////////
bool PPPandaObject::
invoke_default(const NPVariant *args, uint32_t argCount,
               NPVariant *result) {
  //nout << this << ".invoke_default()\n";
  if (_p3d_object == NULL) {
    // Not powered up yet.
    return false;
  }

  P3D_object **p3dargs = new P3D_object *[argCount];
  unsigned int i;
  for (i = 0; i < argCount; ++i) {
    p3dargs[i] = _instance->variant_to_p3dobj(&args[i]);
  }

  P3D_object *value = P3D_OBJECT_CALL(_p3d_object, "", true,
                                      p3dargs, argCount);
  for (i = 0; i < argCount; ++i) {
    P3D_OBJECT_DECREF(p3dargs[i]);
  }
  delete[] p3dargs;

  if (value == NULL) {
    // No such method, or some problem with the parameters.
    return false;
  }

  // We have the return value, and its value is stored in value.
  _instance->p3dobj_to_variant(result, value);
  P3D_OBJECT_DECREF(value);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::has_property
//       Access: Private
//  Description: Returns true if the object has the named property,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool PPPandaObject::
has_property(NPIdentifier name) {
  string property_name = identifier_to_string(name);
  //nout << this << ".has_property(" << property_name << ")\n";
  if (_p3d_object == NULL) {
    // Not powered up yet.
    return false;
  }

  // If we say we don't have a given property, then set_property()
  // will never be called.  So we always say we *do* have any
  // particular property, whether we currently have it right now or
  // not (since we *could* have it if you call set_property()).

  // On the other hand, Firefox gets confused about methods that are
  // also properties.  So you have to say there's *no* property if
  // there is in fact a callable method by that name, or Firefox will
  // never call the method.
  bool result = P3D_OBJECT_HAS_METHOD(_p3d_object, property_name.c_str());
  return !result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::get_property
//       Access: Private
//  Description: Retrieves the named property value from the object
//               and stores it in result.  Returns true on success,
//               false on failure.
////////////////////////////////////////////////////////////////////
bool PPPandaObject::
get_property(NPIdentifier name, NPVariant *result) {
  // Actually, we never return false.  If the property doesn't exist,
  // we return undefined, to be consistent with JavaScript (and with
  // IE).

  string property_name = identifier_to_string(name);
  //nout << this << ".get_property(" << property_name << ")\n";
  if (_p3d_object == NULL) {
    // Not powered up yet.
    VOID_TO_NPVARIANT(*result);
    return true;
  }

  P3D_object *value = P3D_OBJECT_GET_PROPERTY(_p3d_object, property_name.c_str());
  if (value == NULL) {
    // No such property.
    VOID_TO_NPVARIANT(*result);
    return true;
  }

  // We have the property, and its value is stored in value.
  _instance->p3dobj_to_variant(result, value);
  P3D_OBJECT_DECREF(value);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::set_property
//       Access: Private
//  Description: Replaces the named property value on the object.
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool PPPandaObject::
set_property(NPIdentifier name, const NPVariant *value) {
  string property_name = identifier_to_string(name);
  //nout << this << ".set_property(" << property_name << ")\n";
  if (_p3d_object == NULL) {
    // Not powered up yet.
    return false;
  }

  P3D_object *object = _instance->variant_to_p3dobj(value);
  bool result = P3D_OBJECT_SET_PROPERTY(_p3d_object, property_name.c_str(), 
                                        true, object);
  P3D_OBJECT_DECREF(object);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::remove_property
//       Access: Private
//  Description: Deletes the named property value from the object.
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool PPPandaObject::
remove_property(NPIdentifier name) {
  string property_name = identifier_to_string(name);
  //nout << this << ".remove_property(" << property_name << ")\n";
  if (_p3d_object == NULL) {
    // Not powered up yet.
    return false;
  }

  bool result = P3D_OBJECT_SET_PROPERTY(_p3d_object, property_name.c_str(), 
                                        true, NULL);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::enumerate
//       Access: Private
//  Description: Constructs a list of available properties on this
//               object.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool PPPandaObject::
enumerate(NPIdentifier **value, uint32_t *count) {
  //nout << this << ".enumerate()\n";
  // TODO: Not implemented yet.

  // Note that the array of values must be allocated here with
  // NPN_MemAlloc().
  *value = NULL;
  *count = 0;
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::identifier_to_string
//       Access: Private, Static
//  Description: Gets the string equivalent of the indicated
//               identifier, whether it is an integer identifier or a
//               string identifier.
////////////////////////////////////////////////////////////////////
string PPPandaObject::
identifier_to_string(NPIdentifier ident) {
  if (browser->identifierisstring(ident)) {
    NPUTF8 *result = browser->utf8fromidentifier(ident);
    if (result != NULL) {
      string strval(result);
      browser->memfree(result);
      return strval;
    }
  } else {
    // An integer identifier.  We could treat this as a special case,
    // like Firefox does, but Safari doesn't appear to use integer
    // identifiers and just sends everything as a string identifier.
    // So to make things consistent internally, we also send
    // everything as a string.
    ostringstream strm;
    strm << browser->intfromidentifier(ident);
    return strm.str();
  }

  return string();
}


// The remaining function bodies are the C-style function wrappers
// that are called directly by NPAPI, and which redirect into the
// above C++-style methods.

////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::NPAllocate
//       Access: Private, Static
//  Description: Called by NPN_CreateObject() to allocate space for
//               this object.
////////////////////////////////////////////////////////////////////
NPObject *PPPandaObject::
NPAllocate(NPP npp, NPClass *aClass) {
  assert(aClass == &_object_class);
  return (PPPandaObject *)malloc(sizeof(PPPandaObject));
}

////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::Deallocate
//       Access: Private, Static
//  Description: Called to delete the space allocated by NPAllocate,
//               above.
////////////////////////////////////////////////////////////////////
void PPPandaObject::
NPDeallocate(NPObject *npobj) {
  ((PPPandaObject *)npobj)->invalidate();
  free(npobj);
}

////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::Deallocate
//       Access: Private, Static
//  Description: Called to destruct the object.
////////////////////////////////////////////////////////////////////
void PPPandaObject::
NPInvalidate(NPObject *npobj) {
  // It turns out that this method isn't actually called by Safari's
  // implementation of NPAPI, so we'll move the actual destructor call
  // into NPDeallocate, above.
}

////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::NPHasMethod
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPPandaObject::
NPHasMethod(NPObject *npobj, NPIdentifier name) {
  return ((PPPandaObject *)npobj)->has_method(name);
}

////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::NPInvoke
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPPandaObject::
NPInvoke(NPObject *npobj, NPIdentifier name,
         const NPVariant *args, uint32_t argCount,
         NPVariant *result) {
  return ((PPPandaObject *)npobj)->invoke(name, args, argCount, result);
}

////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::NPInvokeDefault
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPPandaObject::
NPInvokeDefault(NPObject *npobj, const NPVariant *args, uint32_t argCount,
                NPVariant *result) {
  return ((PPPandaObject *)npobj)->invoke_default(args, argCount, result);
}

////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::NPHasProperty
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPPandaObject::
NPHasProperty(NPObject *npobj, NPIdentifier name) {
  return ((PPPandaObject *)npobj)->has_property(name);
}

////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::NPGetProperty
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPPandaObject::
NPGetProperty(NPObject *npobj, NPIdentifier name, NPVariant *result) {
  return ((PPPandaObject *)npobj)->get_property(name, result);
}

////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::NPSetProperty
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPPandaObject::
NPSetProperty(NPObject *npobj, NPIdentifier name, const NPVariant *value) {
  return ((PPPandaObject *)npobj)->set_property(name, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::NPRemoveProperty
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPPandaObject::
NPRemoveProperty(NPObject *npobj, NPIdentifier name) {
  return ((PPPandaObject *)npobj)->remove_property(name);
}

////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::NPEnumerate
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPPandaObject::
NPEnumerate(NPObject *npobj, NPIdentifier **value, uint32_t *count) {
  return ((PPPandaObject *)npobj)->enumerate(value, count);
}

////////////////////////////////////////////////////////////////////
//     Function: PPPandaObject::NPConstruct
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPPandaObject::
NPConstruct(NPObject *npobj, const NPVariant *args,
            uint32_t argCount, NPVariant *result) {
  // Not implemented.  We don't use this constructor mechanism because
  // it wasn't supported on earlier versions of Gecko.  Instead, we
  // use make_new() to construct PPPandaObjects via an explicit call to
  // construct().
  return true;
}
