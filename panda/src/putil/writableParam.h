// Filename: writableParam.h
// Created by:  jason (13Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef WRITABLEPARAM_H
#define WRITABLEPARAM_H

#include "pandabase.h"

#include "factoryParam.h"
#include "datagram.h"

#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : WritableParam
// Description : The specific derivation of FactoryParam that
//               contains the information needed by a TypedWritable
//               object.  Simply contains a Datagram for the object
//               to construct itself from.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA WritableParam : public FactoryParam {
public:
  INLINE const Datagram &get_datagram(void);

private:
  const Datagram &_packet;

public:
  INLINE WritableParam(const Datagram &datagram);
  INLINE WritableParam(const WritableParam &other);
  INLINE ~WritableParam();

private:
  // The assignment operator cannot be used for this class.
  INLINE void operator = (const WritableParam &other);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    FactoryParam::init_type();
    register_type(_type_handle, "WritableParam",
                  FactoryParam::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "writableParam.I"

#endif

