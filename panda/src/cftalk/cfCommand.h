// Filename: cfCommand.h
// Created by:  drose (19Feb09)
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

#ifndef CFCOMMAND_H
#define CFCOMMAND_H

#include "pandabase.h"

#include "typedWritableReferenceCount.h"
#include "pandaNode.h"

////////////////////////////////////////////////////////////////////
//       Class : CFCommand
// Description : A single command in the Connected-Frame protocol.
//               This can be sent client-to-server or
//               server-to-client.
//
//               This is an abstract base class.  Individual commands
//               will specialize from this.
////////////////////////////////////////////////////////////////////
class EXPCL_CFTALK CFCommand : public TypedWritableReferenceCount {
protected:
  CFCommand();

PUBLISHED:
  virtual ~CFCommand();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "CFCommand",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//       Class : CFDoCullCommand
// Description : Starts the cull process for a particular
//               DisplayRegion.
////////////////////////////////////////////////////////////////////
class EXPCL_CFTALK CFDoCullCommand : public CFCommand {
protected:
  INLINE CFDoCullCommand();
PUBLISHED:
  INLINE CFDoCullCommand(PandaNode *scene);

  INLINE PandaNode *get_scene() const;

private:
  PT(PandaNode) _scene;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual void update_bam_nested(BamWriter *manager);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CFCommand::init_type();
    register_type(_type_handle, "CFDoCullCommand",
                  CFCommand::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cfCommand.I"

#endif
