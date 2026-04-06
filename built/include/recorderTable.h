/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file recorderTable.h
 * @author drose
 * @date 2004-01-27
 */

#ifndef RECORDERTABLE_H
#define RECORDERTABLE_H

#include "pandabase.h"
#include "recorderBase.h"
#include "pointerTo.h"
#include "pmap.h"
#include "typedWritable.h"

class BamWriter;
class BamReader;
class FactoryParams;

/**
 * This object is used by the RecorderController to write (and read) a record
 * of the set of recorders in use to the bam file.  Do not attempt to use it
 * directly.
 */
class EXPCL_PANDA_RECORDER RecorderTable : public TypedWritable {
public:
  INLINE RecorderTable();
  INLINE RecorderTable(const RecorderTable &copy);
  INLINE void operator = (const RecorderTable &copy);
  ~RecorderTable();

  void merge_from(const RecorderTable &other);

  INLINE void add_recorder(const std::string &name, RecorderBase *recorder);
  INLINE RecorderBase *get_recorder(const std::string &name) const;
  INLINE bool remove_recorder(const std::string &name);

  void record_frame(BamWriter *manager, Datagram &dg);
  void play_frame(DatagramIterator &scan, BamReader *manager);
  void set_flags(short flags);
  void clear_flags(short flags);

  void write(std::ostream &out, int indent_level) const;

  // RecorderBase itself doesn't inherit from ReferenceCount, so we can't put
  // a PT() around it.  Instead, we manage the reference count using calls to
  // ref() and unref().
  typedef pmap<std::string, RecorderBase*> Recorders;
  Recorders _recorders;

  bool _error;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritable::init_type();
    register_type(_type_handle, "RecorderTable",
                  TypedWritable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "recorderTable.I"

#endif
