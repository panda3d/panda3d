/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamCacheIndex.h
 * @author drose
 * @date 2006-06-19
 */

#ifndef BAMCACHEINDEX_H
#define BAMCACHEINDEX_H

#include "pandabase.h"
#include "bamCacheRecord.h"
#include "pointerTo.h"
#include "filename.h"
#include "typedWritable.h"
#include "linkedListNode.h"
#include "pmap.h"
#include "pvector.h"

/**
 * This represents the in-memory index that records the list of files stored
 * in the BamCache.  Since the memory is also flushed to disk from time to
 * time, this class is a TypedWritable object.
 *
 * For the most part, this class is used only by the BamCache class.
 */
class EXPCL_PANDA_PUTIL BamCacheIndex : public TypedWritable, public LinkedListNode {
private:
  INLINE BamCacheIndex();
  ~BamCacheIndex();

public:
  void write(std::ostream &out, int indent_level = 0) const;

private:
  void process_new_records();
  void release_records();
  PT(BamCacheRecord) evict_old_file();

  bool add_record(BamCacheRecord *record);
  bool remove_record(const Filename &source_pathname);

private:
  typedef pmap<Filename, PT(BamCacheRecord) > Records;

  Records _records;
  std::streamsize _cache_size;

  // This structure is a temporary container.  It is only filled in while
  // reading from a bam file.
  typedef pvector< PT(BamCacheRecord) > RecordVector;
  RecordVector _record_vector;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritable::init_type();
    register_type(_type_handle, "BamCacheIndex",
                  TypedWritable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class BamCache;
};

#include "bamCacheIndex.I"

#endif
