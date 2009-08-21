// Filename: ppToplevelObject.cxx
// Created by:  drose (21Aug09)
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

#include "ppToplevelObject.h"

NPClass PPToplevelObject::_object_class = {
  NP_CLASS_STRUCT_VERSION,
  &PPToplevelObject::NPAllocate,
  &PPToplevelObject::NPDeallocate,
  &PPToplevelObject::NPInvalidate,
  &PPToplevelObject::NPHasMethod,
  &PPToplevelObject::NPInvoke,
  &PPToplevelObject::NPInvokeDefault,
  &PPToplevelObject::NPHasProperty,
  &PPToplevelObject::NPGetProperty,
  &PPToplevelObject::NPSetProperty,
  &PPToplevelObject::NPRemoveProperty,
#if defined(NP_CLASS_STRUCT_VERSION_ENUM) && NP_CLASS_STRUCT_VERSION >= NP_CLASS_STRUCT_VERSION_ENUM
  &PPToplevelObject::NPEnumerate,
#endif
#if defined(NP_CLASS_STRUCT_VERSION_CTOR) && NP_CLASS_STRUCT_VERSION >= NP_CLASS_STRUCT_VERSION_CTOR
  &PPToplevelObject::NPConstruct,
#endif
};


////////////////////////////////////////////////////////////////////
//     Function: PPToplevelObject::make_new
//       Access: Public, Static
//  Description: Use this call to construct a new PPToplevelObject.
////////////////////////////////////////////////////////////////////
PPToplevelObject *PPToplevelObject::
make_new(PPInstance *inst) {
  NPObject *npobj = 
    browser->createobject(inst->get_npp_instance(), &_object_class);
  PPToplevelObject *ppobj = (PPToplevelObject *)npobj;
  ppobj->construct(inst);
  return ppobj;
}

////////////////////////////////////////////////////////////////////
//     Function: PPToplevelObject::set_main
//       Access: Public
//  Description: Changes the "main" object this PPToplevelObject maps
//               to.  The new object's reference count is incremented,
//               and the previous object's is decremented.
////////////////////////////////////////////////////////////////////
void PPToplevelObject::
set_main(P3D_object *p3d_object) {
  if (p3d_object != NULL) {
    P3D_OBJECT_INCREF(p3d_object);
  }
  P3D_OBJECT_XDECREF(_main);
  _main = p3d_object;
}
 
////////////////////////////////////////////////////////////////////
//     Function: PPToplevelObject::construct
//       Access: Private
//  Description: Stands in for the C++ constructor.  We can't have a
//               true constructor because of the C-style interface in
//               NPN_CreateObject().  This must be called explicitly
//               following NPN_CreateObject().
////////////////////////////////////////////////////////////////////
void PPToplevelObject::
construct(PPInstance *inst) {
  _instance = inst;
  _main = NULL;

  // Get our one property name as an identifier, so we can look for
  // it.
  _main_id = browser->getstringidentifier("main");
}

////////////////////////////////////////////////////////////////////
//     Function: PPToplevelObject::invalidate
//       Access: Private
//  Description: This "destructor" is called by NPInvalidate().
////////////////////////////////////////////////////////////////////
void PPToplevelObject::
invalidate() {
  _instance = NULL;
  set_main(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PPToplevelObject::has_property
//       Access: Private
//  Description: Returns true if the object has the named property,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool PPToplevelObject::
has_property(NPIdentifier name) {
  if (_main == NULL) {
    // Not powered up yet.
    return false;
  }

  if (name == _main_id) {
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PPToplevelObject::get_property
//       Access: Private
//  Description: Retrieves the named property value from the object
//               and stores it in result.  Returns true on success,
//               false on failure.
////////////////////////////////////////////////////////////////////
bool PPToplevelObject::
get_property(NPIdentifier name, NPVariant *result) {
  if (_main == NULL) {
    // Not powered up yet.
    return false;
  }

  if (name == _main_id) {
    _instance->p3dobj_to_variant(result, _main);
    return true;
  }

  return false;
}


// The remaining function bodies are the C-style function wrappers
// that are called directly by NPAPI, and which redirect into the
// above C++-style methods.

////////////////////////////////////////////////////////////////////
//     Function: PPToplevelObject::NPAllocate
//       Access: Private, Static
//  Description: Called by NPN_CreateObject() to allocate space for
//               this object.
////////////////////////////////////////////////////////////////////
NPObject *PPToplevelObject::
NPAllocate(NPP npp, NPClass *aClass) {
  assert(aClass == &_object_class);
  return (PPToplevelObject *)malloc(sizeof(PPToplevelObject));
}

////////////////////////////////////////////////////////////////////
//     Function: PPToplevelObject::Deallocate
//       Access: Private, Static
//  Description: Called to delete the space allocated by NPAllocate,
//               above.
////////////////////////////////////////////////////////////////////
void PPToplevelObject::
NPDeallocate(NPObject *npobj) {
  ((PPToplevelObject *)npobj)->invalidate();
  free(npobj);
}

////////////////////////////////////////////////////////////////////
//     Function: PPToplevelObject::Deallocate
//       Access: Private, Static
//  Description: Called to destruct the object.
////////////////////////////////////////////////////////////////////
void PPToplevelObject::
NPInvalidate(NPObject *npobj) {
  // It turns out that this method isn't actually called by Safari's
  // implementation of NPAPI, so we'll move the actual destructor call
  // into NPDeallocate, above.
}

////////////////////////////////////////////////////////////////////
//     Function: PPToplevelObject::NPHasMethod
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPToplevelObject::
NPHasMethod(NPObject *npobj, NPIdentifier name) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PPToplevelObject::NPInvoke
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPToplevelObject::
NPInvoke(NPObject *npobj, NPIdentifier name,
         const NPVariant *args, uint32_t argCount,
         NPVariant *result) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PPToplevelObject::NPInvokeDefault
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPToplevelObject::
NPInvokeDefault(NPObject *npobj, const NPVariant *args, uint32_t argCount,
                NPVariant *result) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PPToplevelObject::NPHasProperty
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPToplevelObject::
NPHasProperty(NPObject *npobj, NPIdentifier name) {
  return ((PPToplevelObject *)npobj)->has_property(name);
}

////////////////////////////////////////////////////////////////////
//     Function: PPToplevelObject::NPGetProperty
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPToplevelObject::
NPGetProperty(NPObject *npobj, NPIdentifier name, NPVariant *result) {
  return ((PPToplevelObject *)npobj)->get_property(name, result);
}

////////////////////////////////////////////////////////////////////
//     Function: PPToplevelObject::NPSetProperty
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPToplevelObject::
NPSetProperty(NPObject *npobj, NPIdentifier name, const NPVariant *value) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PPToplevelObject::NPRemoveProperty
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPToplevelObject::
NPRemoveProperty(NPObject *npobj, NPIdentifier name) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PPToplevelObject::NPEnumerate
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPToplevelObject::
NPEnumerate(NPObject *npobj, NPIdentifier **value, uint32_t *count) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PPToplevelObject::NPConstruct
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
bool PPToplevelObject::
NPConstruct(NPObject *npobj, const NPVariant *args,
            uint32_t argCount, NPVariant *result) {
  // Not implemented.  We don't use this constructor mechanism because
  // it wasn't supported on earlier versions of Gecko.  Instead, we
  // use make_new() to construct PPToplevelObjects via an explicit call to
  // construct().
  return true;
}
