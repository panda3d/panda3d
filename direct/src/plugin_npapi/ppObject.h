// Filename: ppObject.h
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

#ifndef PPOBJECT_H
#define PPOBJECT_H

#include "nppanda3d_common.h"

////////////////////////////////////////////////////////////////////
//       Class : PPObject
// Description : This is the interface layer between an NPObject and a
//               P3D_object.  It maps calls from NPAPI into the
//               P3D_object system.
////////////////////////////////////////////////////////////////////
class PPObject : public NPObject {
public:
  static PPObject *make_new(PPInstance *inst, P3D_object *p3d_object);

  inline P3D_object *get_p3d_object() const;
  void set_p3d_object(P3D_object *p3d_object);

private:
  void construct(P3D_object *p3d_object);
  void invalidate();

  bool has_method(NPIdentifier name);
  bool invoke(NPIdentifier name,
              const NPVariant *args, uint32_t argCount,
              NPVariant *result);
  bool invoke_default(const NPVariant *args, uint32_t argCount,
                      NPVariant *result);
  bool has_property(NPIdentifier name);
  bool get_property(NPIdentifier name,
                    NPVariant *result);
  bool set_property(NPIdentifier name,
                    const NPVariant *value);
  bool remove_property(NPIdentifier name);
  bool enumerate(NPIdentifier **value, uint32_t *count);

private:
  static NPObject *NPAllocate(NPP npp, NPClass *aClass);
  static void NPDeallocate(NPObject *npobj);
  static void NPInvalidate(NPObject *npobj);
  static bool NPHasMethod(NPObject *npobj, NPIdentifier name);
  static bool NPInvoke(NPObject *npobj, NPIdentifier name,
                       const NPVariant *args, uint32_t argCount,
                       NPVariant *result);
  static bool NPInvokeDefault(NPObject *npobj,
                              const NPVariant *args,
                              uint32_t argCount,
                              NPVariant *result);
  static bool NPHasProperty(NPObject *npobj, NPIdentifier name);
  static bool NPGetProperty(NPObject *npobj, NPIdentifier name,
                            NPVariant *result);
  static bool NPSetProperty(NPObject *npobj, NPIdentifier name,
                            const NPVariant *value);
  static bool NPRemoveProperty(NPObject *npobj,
                               NPIdentifier name);
  static bool NPEnumerate(NPObject *npobj, NPIdentifier **value,
                          uint32_t *count);
  static bool NPConstruct(NPObject *npobj,
                          const NPVariant *args,
                          uint32_t argCount,
                          NPVariant *result);

private:
  P3D_object *_p3d_object;

  static NPClass _object_class;
};

#include "ppObject.I"

#endif

