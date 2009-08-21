// Filename: ppToplevelObject.h
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

#ifndef PPTOPLEVELOBJECT_H
#define PPTOPLEVELOBJECT_H

#include "nppanda3d_common.h"

////////////////////////////////////////////////////////////////////
//       Class : PPToplevelObject
// Description : This is a special object fed to Mozilla as the
//               toplevel scripting object for the instance.  It has
//               only one property, "main", which corresponds to the
//               appRunner.main object from Python.
////////////////////////////////////////////////////////////////////
class PPToplevelObject : public NPObject {
public:
  static PPToplevelObject *make_new(PPInstance *inst);

  inline P3D_object *get_main() const;
  void set_main(P3D_object *main);

private:
  void construct(PPInstance *inst);
  void invalidate();

  bool has_property(NPIdentifier name);
  bool get_property(NPIdentifier name,
                    NPVariant *result);

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
  PPInstance *_instance;
  P3D_object *_main;
  NPIdentifier _main_id;

public:
  static NPClass _object_class;
};

#include "ppToplevelObject.I"

#endif

