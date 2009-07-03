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
  ppobj->construct(p3d_object);
  return ppobj;
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::set_p3d_object
//       Access: Public
//  Description: Changes the p3d_object this PPObject maps to.  The
//               previous object, if any, is deleted.
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
construct(P3D_object *p3d_object) {
  logfile << "construct: " << this << "\n" << flush;
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
  logfile << "has_method: " << this << "\n" << flush;
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
  logfile << "has_property: " << this << "\n" << flush;
  return false;
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
  logfile << "get_property: " << this << "\n" << flush;
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::set_property
//       Access: Private
//  Description: Replaces the named property value on the object.
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool PPObject::
set_property(NPIdentifier name, const NPVariant *value) {
  logfile << "set_property: " << this << "\n" << flush;
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PPObject::remove_property
//       Access: Private
//  Description: Deletes the named property value from the object.
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool PPObject::
remove_property(NPIdentifier name) {
  logfile << "remove_property: " << this << "\n" << flush;
  return false;
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
  ((PPObject *)npobj)->invalidate();
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
