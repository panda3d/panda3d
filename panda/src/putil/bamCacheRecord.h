/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamCacheRecord.h
 * @author drose
 * @date 2006-06-08
 */

#ifndef BAMCACHERECORD_H
#define BAMCACHERECORD_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "pointerTo.h"
#include "linkedListNode.h"

class BamWriter;
class BamReader;
class Datagram;
class DatagramIterator;
class FactoryParams;
class BamCacheRecord;
class VirtualFile;

/**
 * An instance of this class is written to the front of a Bam or Txo file to
 * make the file a cached instance of some other loadable resource.  This
 * record contains information needed to test the validity of the cache.
 */
class EXPCL_PANDA_PUTIL BamCacheRecord : public TypedWritableReferenceCount,
                                   public LinkedListNode {
private:
  BamCacheRecord();
  BamCacheRecord(const Filename &source_pathname,
                 const Filename &cache_filename);
  BamCacheRecord(const BamCacheRecord &copy);

PUBLISHED:
  virtual ~BamCacheRecord();

  INLINE PT(BamCacheRecord) make_copy() const;

  INLINE bool operator == (const BamCacheRecord &other) const;

  INLINE const Filename &get_source_pathname() const;
  INLINE const Filename &get_cache_filename() const;
  INLINE time_t get_source_timestamp() const;
  INLINE time_t get_recorded_time() const;

  MAKE_PROPERTY(source_pathname, get_source_pathname);
  MAKE_PROPERTY(cache_filename, get_cache_filename);
  MAKE_PROPERTY(source_timestamp, get_source_timestamp);
  MAKE_PROPERTY(recorded_time, get_recorded_time);

  INLINE int get_num_dependent_files() const;
  INLINE const Filename &get_dependent_pathname(int n) const;

  bool dependents_unchanged() const;
  void clear_dependent_files();
  void add_dependent_file(const Filename &pathname);
  void add_dependent_file(const VirtualFile *file);

  INLINE bool has_data() const;
  INLINE void clear_data();
  INLINE TypedWritable *get_data() const;
  INLINE bool extract_data(TypedWritable *&ptr, ReferenceCount *&ref_ptr);
  INLINE void set_data(TypedWritable *ptr, ReferenceCount *ref_ptr);
  INLINE void set_data(TypedWritable *ptr);
  INLINE void set_data(TypedWritableReferenceCount *ptr);
  INLINE void set_data(TypedWritable *ptr, int dummy);

  MAKE_PROPERTY2(data, has_data, get_data, set_data, clear_data);

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;

private:
  // This class is used to sort BamCacheRecords by access time.
  class SortByAccessTime {
  public:
    INLINE bool operator () (const BamCacheRecord *a, const BamCacheRecord *b) const;
  };

  static std::string format_timestamp(time_t timestamp);

  Filename _source_pathname;
  Filename _cache_filename;
  time_t _recorded_time;
  std::streamsize _record_size;  // this is accurate only in the index file.
  time_t _source_timestamp;  // Not record to the cache file.

  class DependentFile {
  public:
    Filename _pathname;
    time_t _timestamp;
    std::streamsize _size;
  };

  typedef pvector<DependentFile> DependentFiles;
  DependentFiles _files;

  // The following are not recorded to disk; they are preserved in-memory only
  // for the current session.
  Filename _cache_pathname;
  TypedWritable *_ptr;
  ReferenceCount *_ref_ptr;

  // The following are not recorded to disk, nor even returned by the BamCache
  // interface.  They are strictly meaningful to the BamCacheRecords stored
  // internally within the BamCache object.
  time_t _record_access_time;

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
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "BamCacheRecord",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class BamCache;
  friend class BamCacheIndex;
  friend class BamCacheRecord::SortByAccessTime;
};

INLINE std::ostream &operator << (std::ostream &out, const BamCacheRecord &record) {
  record.output(out);
  return out;
}

#include "bamCacheRecord.I"

#endif
