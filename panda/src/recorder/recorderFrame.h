/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file recorderFrame.h
 * @author drose
 * @date 2004-01-28
 */

#ifndef RECORDERFRAME_H
#define RECORDERFRAME_H

#include "pandabase.h"
#include "recorderBase.h"
#include "typedWritable.h"
#include "recorderTable.h"
#include "datagram.h"

class BamWriter;
class BamReader;
class FactoryParams;

/**
 * This object represents one frame of data in the recorded session file.  One
 * of these is repeatedly created and destructed in recording and playback,
 * respectively.
 */
class EXPCL_PANDA_RECORDER RecorderFrame : public TypedWritable {
public:
  INLINE RecorderFrame();
  INLINE RecorderFrame(double timestamp, int frame,
                       bool table_changed, RecorderTable *table);
  INLINE ~RecorderFrame();

  void play_frame(BamReader *manager);

  double _timestamp;
  int _frame;
  bool _table_changed;
  RecorderTable *_table;

  Datagram _data;
  size_t _data_pos;

private:
  RecorderTable _local_table;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritable::init_type();
    register_type(_type_handle, "RecorderFrame",
                  TypedWritable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "recorderFrame.I"

#endif
