// Filename: recorderBase.h
// Created by:  drose (25Jan04)
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

#ifndef RECORDERBASE_H
#define RECORDERBASE_H

#include "pandabase.h"
#include "referenceCount.h"

class BamReader;
class BamWriter;
class Datagram;
class DatagramIterator;
class TypedWritable;

////////////////////////////////////////////////////////////////////
//       Class : RecorderBase
// Description : This is the base class to a number of objects that
//               record particular kinds of user input (like a
//               MouseRecorder) to use in conjunction with a
//               RecorderController to record the user's inputs
//               for a session.
//
//               Note that RecorderBase does not actually inherit from
//               TypedObject, even though it defines get_type().  The
//               assumption is that the classes that derive from
//               RecorderBase might also inherit independently from
//               TypedObject.
//
//               It also does not inherit from TypedWritable, but it
//               defines a method called write_recorder() which is
//               very similar to a TypedWritable's write_datagram().
//               Classes that derive from RecorderBase and also
//               inherit from TypedWritable may choose to remap
//               write_recorder() to do exactly the same thing as
//               write_datagram(), or they may choose to write
//               something slightly different.
//
//               Most types of recorders should derive from Recorder,
//               as it derives from ReferenceCount, except for
//               MouseRecorder, which would otherwise doubly inherit
//               from ReferenceCount.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_RECORDER RecorderBase {
protected:
  RecorderBase();

PUBLISHED:
  virtual ~RecorderBase();

  INLINE bool is_recording() const;
  INLINE bool is_playing() const;

public:
  virtual void record_frame(BamWriter *manager, Datagram &dg);
  virtual void play_frame(DatagramIterator &scan, BamReader *manager);

  virtual void write_recorder(BamWriter *manager, Datagram &dg);

  // We can't let RecorderBase inherit from ReferenceCount, so we
  // define these so we can still manage the reference count.
  virtual void ref() const=0;
  virtual bool unref() const=0;

protected:
  void fillin_recorder(DatagramIterator &scan, BamReader *manager);

private:
  enum Flags {
    F_recording = 0x0001,
    F_playing   = 0x0002,
  };
  short _flags;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ReferenceCount::init_type();
    register_type(_type_handle, "RecorderBase",
                  ReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

  friend class RecorderController;
  friend class RecorderTable;
};

#include "recorderBase.I"

#endif
