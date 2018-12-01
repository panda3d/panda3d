/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file writableParam.h
 * @author jason
 * @date 2000-06-13
 */

#ifndef WRITABLEPARAM_H
#define WRITABLEPARAM_H

#include "pandabase.h"

#include "factoryParam.h"
#include "datagram.h"

#include "pvector.h"

/**
 * The specific derivation of FactoryParam that contains the information
 * needed by a TypedWritable object.  Simply contains a Datagram for the
 * object to construct itself from.
 */
class EXPCL_PANDA_PUTIL WritableParam : public FactoryParam {
public:
  INLINE const Datagram &get_datagram();

private:
  const Datagram &_packet;

public:
  INLINE WritableParam(const Datagram &datagram);
  INLINE WritableParam(const WritableParam &other);

private:
  // The assignment operator cannot be used for this class.
  WritableParam &operator = (const WritableParam &other) = delete;

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
