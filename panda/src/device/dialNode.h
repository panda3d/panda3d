// Filename: dialNode.h
// Created by:  drose (26Jan01)
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

#ifndef DIALNODE_H
#define DIALNODE_H

#include <pandabase.h>

#include "clientBase.h"
#include "clientDialDevice.h"

#include <dataNode.h>
#include <nodeAttributes.h>


////////////////////////////////////////////////////////////////////
//       Class : DialNode
// Description : This is the primary interface to infinite dial type
//               devices associated with a ClientBase.  This creates a
//               node that connects to the named dial device, if it
//               exists, and provides hooks to the user to read the
//               state of any of the sequentially numbered dial
//               controls associated with that device.
//
//               A dial is a rotating device that does not have
//               stops--it can keep rotating any number of times.
//               Therefore it does not have a specific position at any
//               given time, unlike an AnalogDevice.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DialNode : public DataNode {
PUBLISHED:
  DialNode(ClientBase *client, const string &device_name);
  virtual ~DialNode();

  INLINE bool is_valid() const;

  INLINE int get_num_dials() const;

  INLINE double read_dial(int index);
  INLINE bool is_dial_known(int index) const;

////////////////////////////////////////////////////////////////////
// From parent class DataNode
////////////////////////////////////////////////////////////////////
public:
  virtual void
  transmit_data(NodeAttributes &data);

  NodeAttributes _attrib;

private:
  PT(ClientDialDevice) _dial;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

#include "dialNode.I"

#endif
