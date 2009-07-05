// Filename: ppObject.cxx
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

#include "ppObject.h"

NPClass PPObject::_object_class = {
  NP_CLASS_STRUCT_VERSION,
  &PPObject::NPAllocate,
  &PPObject::NPDeallocate,
  &PPObject::NPInvalidate,
  &PPObject::NPHasMethod,
  &PPObject::NPInvoke,
  &PPObject::NPInvokeDefault,
  &PPObject::NPHasProperty,
  &PPObject::NPGetProperty,
  &PPObject::NPSetProperty,
  &PPObject::NPRemoveProperty,
#if NP_CLASS_STRUCT_VERSION >= NP_CLASS_STRUCT_VERSION_ENUM
  &PPObject::NPEnumerate,
#endif
#if NP_CLASS_STRUCT_VERSION >= NP_CLASS_STRUCT_VERSION_CTOR
  &PPObject::NPConstruct,
#endif
};


////////////////////////////////////////////////////////////////////
//     Function: PPObject::make_new
//       Access: Public, Static
//  Description: Use this call to construct a new PPObject.
////////////////////////////////////////////////////////////////////
PPObject *PPObject::
make_new(PPInstance *inst, P3D_object *p3d_object) {
  NPObject *npobj = 
    browser->createobject(inst->get_npp_instance(), &_object_class);
  PPObject *ppobj = (PPObject *)npobj;
  ppobj->construct(inst, p3d_object);
  return ppobj;
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::set_p3d_object
//       Access: Public
//  Description: Changes the p3d_object this PPObject maps to.  The
//               previous object, if any, is deleted.  Ownership of
//               the new object is passed to the PPObject.
////////////////////////////////////////////////////////////////////
void PPObject::
set_p3d_object(P3D_object *p3d_object) {
  if (_p3d_object != p3d_object) {
    if (_p3d_object != NULL) {
      P3D_OBJECT_FINISH(_p3d_object);
    }
    _p3d_object = p3d_object;
  }
}
 
////////////////////////////////////////////////////////////////////
//     Function: PPObject::construct
//       Access: Private
//  Description: Stands in for the C++ constructor.  We can't have a
//               true constructor because of the C-style interface in
//               NPN_CreateObject().  This must be called explicitly
//               following NPN_CreateObject().
////////////////////////////////////////////////////////////////////
void PPObject::
construct(PPInstance *inst, P3D_object *p3d_object) {
  logfile << "construct: " << this << "\n" << flush;
  _instance = inst;
  _p3d_object = p3d_object;
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::invalidate
//       Access: Private
//  Description: This "destructor" is called by NPInvalidate().
////////////////////////////////////////////////////////////////////
void PPObject::
invalidate() {
  logfile << "invalidate: " << this << "\n" << flush;
  _instance = NULL;
  set_p3d_object(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::has_method
//       Access: Private
//  Description: Returns true if the object has the named method,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool PPObject::
has_method(NPIdentifier name) {
  string property_name = identifier_to_string(name);
  logfile << "has_method: " << this << ", " << property_name << "\n" << flush;
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::invoke
//       Access: Private
//  Description: Calls the named method on the object, storing the
//               return value into result.  Returns true on success,
//               false on failure.
////////////////////////////////////////////////////////////////////
bool PPObject::
invoke(NPIdentifier name, const NPVariant *args, uint32_t argCount,
       NPVariant *result) {
  logfile << "invoke: " << this << "\n" << flush;
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::invoke_default
//       Access: Private
//  Description: Calls the default method on the object, storing the
//               return value into result.  Returns true on success,
//               false on failure.
////////////////////////////////////////////////////////////////////
bool PPObject::
invoke_default(const NPVariant *args, uint32_t argCount,
               NPVariant *result) {
  logfile << "invoke_default: " << this << "\n" << flush;
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::has_property
//       Access: Private
//  Description: Returns true if the object has the named property,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool PPObject::
has_property(NPIdentifier name) {
  string property_name = identifier_to_string(name);
  logfile << "has_property: " << this << ", " << property_name << "\n" << flush;

  // If we say we don't have a given property, then set_property()
  // will never be called.  So we always say we *do* have any
  // particular property, whether we currently have it right now or
  // not (since we *could* have it if you call set_property()).
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::get_property
//       Access: Private
//  Description: Retrieves the named property value from the object
//               and stores it in result.  Returns true on success,
//               false on failure.
////////////////////////////////////////////////////////////////////
bool PPObject::
get_property(NPIdentifier name, NPVariant *result) {
  string property_name = identifier_to_string(name);
  logfile << "get_property: " << this << ", " << property_name << "\n" << flush;
  if (_p3d_object == NULL) {
    // Not powered up yet.
    return false;
  }

  P3D_object *value = P3D_OBJECT_GET_PROPERTY(_p3d_object, property_name.c_str());
  if (value == NULL) {
    // No such property.
    return false;
  }

  // We have the property, and its value is stored in value.
  object_to_variant(result, value);
  P3D_OBJECT_FINISH(value);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::set_property
//       Access: Private
//  Description: Replaces the named property value on the object.
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool PPObject::
set_property(NPIdentifier name, const NPVariant *value) {
  string property_name = identifier_to_string(name);
  logfile << "set_property: " << this << ", " << property_name << "\n" << flush;
  if (_p3d_object == NULL) {
    // Not powered up yet.
    return false;
  }

  P3D_object *object = variant_to_object(value);
  bool result = P3D_OBJECT_SET_PROPERTY(_p3d_object, property_name.c_str(), object);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::remove_property
//       Access: Private
//  Description: Deletes the named property value from the object.
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool PPObject::
remove_property(NPIdentifier name) {
  string property_name = identifier_to_string(name);
  logfile << "remove_property: " << this << ", " << property_name << "\n" << flush;
  if (_p3d_object == NULL) {
    // Not powered up yet.
    return false;
  }

  bool result = P3D_OBJECT_SET_PROPERTY(_p3d_object, property_name.c_str(), NULL);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::enumerate
//       Access: Private
//  Description: Constructs a list of available properties on this
//               object.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool PPObject::
enumerate(NPIdentifier **value, uint32_t *count) {
  logfile << "enumerate: " << this << "\n" << flush;
  // TODO: Not implemented yet.

  // Note that the array of values must be allocated here with
  // NPN_MemAlloc().
  *value = NULL;
  *count = 0;
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::identifier_to_string
//       Access: Private, Static
//  Description: Gets the string equivalent of the indicated
//               identifier, whether it is an integer identifier or a
//               string identifier.
////////////////////////////////////////////////////////////////////
string PPObject::
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

////////////////////////////////////////////////////////////////////
//     Function: PPObject::object_to_variant
//       Access: Private
//  Description: Converts the indicated P3D_object to the equivalent
//               NPVariant, and stores it in result.
////////////////////////////////////////////////////////////////////
void PPObject::
object_to_variant(NPVariant *result, const P3D_object *object) {
  switch (P3D_OBJECT_GET_TYPE(object)) {
  case P3D_OT_none:
    VOID_TO_NPVARIANT(*result);
    break;

  case P3D_OT_bool:
    BOOLEAN_TO_NPVARIANT(P3D_OBJECT_GET_BOOL(object), *result);
    break;

  case P3D_OT_int:
    INT32_TO_NPVARIANT(P3D_OBJECT_GET_INT(object), *result);
    break;

  case P3D_OT_float:
    DOUBLE_TO_NPVARIANT(P3D_OBJECT_GET_FLOAT(object), *result);
    break;

  case P3D_OT_string:
    {
      int size = P3D_OBJECT_GET_STRING(object, NULL, 0);
      char *buffer = (char *)browser->memalloc(size);
      P3D_OBJECT_GET_STRING(object, buffer, size);
      STRINGN_TO_NPVARIANT(buffer, size, *result);
    }
    break;

  case P3D_OT_list:
  case P3D_OT_object:
    {
      PPObject *ppobj = PPObject::make_new(_instance, P3D_OBJECT_COPY(object));
      OBJECT_TO_NPVARIANT(ppobj, *result);
    }
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::variant_to_object
//       Access: Private
//  Description: Converts the indicated NPVariant to the equivalent
//               P3D_object, and returns it (newly-allocated).  The
//               caller is responsible for freeing the returned object
//               later.
////////////////////////////////////////////////////////////////////
P3D_object *PPObject::
variant_to_object(const NPVariant *variant) {
  if (NPVARIANT_IS_VOID(*variant) ||
      NPVARIANT_IS_NULL(*variant)) {
    return P3D_new_none_object();
  } else if (NPVARIANT_IS_BOOLEAN(*variant)) {
    return P3D_new_bool_object(NPVARIANT_TO_BOOLEAN(*variant));
  } else if (NPVARIANT_IS_INT32(*variant)) {
    return P3D_new_int_object(NPVARIANT_TO_INT32(*variant));
  } else if (NPVARIANT_IS_DOUBLE(*variant)) {
    return P3D_new_float_object(NPVARIANT_TO_DOUBLE(*variant));
  } else if (NPVARIANT_IS_STRING(*variant)) {
    NPString str = NPVARIANT_TO_STRING(*variant);
    return P3D_new_string_object(str.utf8characters, str.utf8length);
  } else if (NPVARIANT_IS_OBJECT(*variant)) {
    // TODO.
    return P3D_new_none_object();
  }

  // Hmm, none of the above?
  return P3D_new_none_object();
}


// The remaining function bodies are the C-style function wrappers
// that are called directly by NPAPI, and which redirect into the
// above C++-style methods.

////////////////////////////////////////////////////////////////////
//     Function: PPObject::NPAllocate
//       Access: Private, Static
//  Description: Called by NPN_CreateObject() to allocate space for
//               this object.
////////////////////////////////////////////////////////////////////
NPObject *PPObject::
NPAllocate(NPP npp, NPClass *aClass) {
  logfile << "NPAllocate\n";
  assert(aClass == &_object_class);
  return (PPObject *)malloc(sizeof(PPObject));
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::Deallocate
//       Access: Private, Static
//  Description: Called to delete the space allocated by NPAllocate,
//               above.
////////////////////////////////////////////////////////////////////
void PPObject::
NPDeallocate(NPObject *npobj) {
  logfile << "NPDeallocate: " << npobj << "\n" << flush;
  ((PPObject *)npobj)->invalidate();
  free(npobj);
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::Deallocate
//       Access: Private, Static
//  Description: Called to destruct the object.
////////////////////////////////////////////////////////////////////
void PPObject::
NPInvalidate(NPObject *npobj) {
  logfile << "NPInvalidate: " << npobj << "\n" << flush;

  // It turns out that this method isn't actually called by Safari's
  // implementation of NPAPI, so we'll move the actual destructor call
  // into NPDeallocate, above.
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::NPHasMethod
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPObject::
NPHasMethod(NPObject *npobj, NPIdentifier name) {
  return ((PPObject *)npobj)->has_method(name);
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::NPInvoke
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPObject::
NPInvoke(NPObject *npobj, NPIdentifier name,
         const NPVariant *args, uint32_t argCount,
         NPVariant *result) {
  return ((PPObject *)npobj)->invoke(name, args, argCount, result);
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::NPInvokeDefault
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPObject::
NPInvokeDefault(NPObject *npobj, const NPVariant *args, uint32_t argCount,
                NPVariant *result) {
  return ((PPObject *)npobj)->invoke_default(args, argCount, result);
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::NPHasProperty
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPObject::
NPHasProperty(NPObject *npobj, NPIdentifier name) {
  return ((PPObject *)npobj)->has_property(name);
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::NPGetProperty
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPObject::
NPGetProperty(NPObject *npobj, NPIdentifier name, NPVariant *result) {
  return ((PPObject *)npobj)->get_property(name, result);
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::NPSetProperty
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPObject::
NPSetProperty(NPObject *npobj, NPIdentifier name, const NPVariant *value) {
  return ((PPObject *)npobj)->set_property(name, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::NPRemoveProperty
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPObject::
NPRemoveProperty(NPObject *npobj, NPIdentifier name) {
  return ((PPObject *)npobj)->remove_property(name);
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::NPEnumerate
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPObject::
NPEnumerate(NPObject *npobj, NPIdentifier **value, uint32_t *count) {
  return ((PPObject *)npobj)->enumerate(value, count);
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::NPConstruct
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPObject::
NPConstruct(NPObject *npobj, const NPVariant *args,
            uint32_t argCount, NPVariant *result) {
  // Not implemented.  We don't use this constructor mechanism because
  // it wasn't supported on earlier versions of Gecko.  Instead, we
  // use make_new() to construct PPObjects via an explicit call to
  // construct().
  return true;
}
