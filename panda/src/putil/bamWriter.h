/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamWriter.h
 * @author jason
 * @date 2000-06-08
 */

#ifndef __BAM_WRITER_
#define __BAM_WRITER_

#include "pandabase.h"
#include "pnotify.h"
#include "bamEnums.h"
#include "typedWritable.h"
#include "datagramSink.h"
#include "pdeque.h"
#include "pset.h"
#include "pmap.h"
#include "vector_int.h"
#include "pipelineCyclerBase.h"


// A handy macro for writing PointerToArrays.
#define WRITE_PTA(Manager, dest, Write_func, array)  \
  if (!Manager->register_pta(dest, array.p()))       \
  {                                                  \
    Write_func(Manager, dest, array);                \
  }                                                  \


/**
 * This is the fundamental interface for writing binary objects to a Bam file,
 * to be extracted later by a BamReader.
 *
 * A Bam file can be thought of as a linear collection of objects.  Each
 * object is an instance of a class that inherits, directly or indirectly,
 * from TypedWritable.  The objects may include pointers to other objects; the
 * BamWriter automatically manages these (with help from code within each
 * class) and writes all referenced objects to the file in such a way that the
 * pointers may be correctly restored later.
 *
 * This is the abstract interface and does not specifically deal with disk
 * files, but rather with a DatagramSink of some kind, which simply accepts a
 * linear stream of Datagrams.  It is probably written to a disk file, but it
 * might conceivably be streamed directly to a network or some such nonsense.
 *
 * Bam files are most often used to store scene graphs or subgraphs, and by
 * convention they are given filenames ending in the extension ".bam" when
 * they are used for this purpose.  However, a Bam file may store any
 * arbitrary list of TypedWritable objects; in this more general usage, they
 * are given filenames ending in ".boo" to differentiate them from the more
 * common scene graph files.
 *
 * See also BamFile, which defines a higher-level interface to read and write
 * Bam files on disk.
 */
class EXPCL_PANDA_PUTIL BamWriter : public BamEnums {
PUBLISHED:
  explicit BamWriter(DatagramSink *target = nullptr);
  ~BamWriter();

  void set_target(DatagramSink *target);
  INLINE DatagramSink *get_target();

  bool init();
  INLINE const Filename &get_filename() const;
  bool write_object(const TypedWritable *obj);
  bool has_object(const TypedWritable *obj) const;
  void flush();

  INLINE int get_file_major_ver() const;
  INLINE int get_file_minor_ver() const;
  INLINE void set_file_minor_ver(int minor_ver);

  INLINE BamEndian get_file_endian() const;
  INLINE bool get_file_stdfloat_double() const;

  INLINE BamTextureMode get_file_texture_mode() const;
  INLINE void set_file_texture_mode(BamTextureMode file_texture_mode);

  INLINE TypedWritable *get_root_node() const;
  INLINE void set_root_node(TypedWritable *root_node);

PUBLISHED:
  MAKE_PROPERTY(target, get_target, set_target);
  MAKE_PROPERTY(filename, get_filename);
  MAKE_PROPERTY(file_endian, get_file_endian);
  MAKE_PROPERTY(file_stdfloat_double, get_file_stdfloat_double);
  MAKE_PROPERTY(file_texture_mode, get_file_texture_mode);
  MAKE_PROPERTY(root_node, get_root_node, set_root_node);

public:
  // Functions to support classes that write themselves to the Bam.

  void consider_update(const TypedWritable *obj);

  void write_pointer(Datagram &packet, const TypedWritable *dest);

  void write_file_data(SubfileInfo &result, const Filename &filename);
  void write_file_data(SubfileInfo &result, const SubfileInfo &source);

  void write_cdata(Datagram &packet, const PipelineCyclerBase &cycler);
  void write_cdata(Datagram &packet, const PipelineCyclerBase &cycler,
                   void *extra_data);
  bool register_pta(Datagram &packet, const void *ptr);
  void write_handle(Datagram &packet, TypeHandle type);

  static std::string get_obsolete_type_name(TypeHandle type, int major, int minor);
  static void record_obsolete_type_name(TypeHandle type, std::string name,
                                        int before_major, int before_minor);

private:
  void object_destructs(TypedWritable *object);

  void write_object_id(Datagram &dg, int object_id);
  void write_pta_id(Datagram &dg, int pta_id);
  int enqueue_object(const TypedWritable *object);
  bool flush_queue();

  int _file_major, _file_minor;
  BamEndian _file_endian;
  bool _file_stdfloat_double;
  BamTextureMode _file_texture_mode;

  // Stores the PandaNode representing the root of the node hierarchy we are
  // currently writing, if any, for the purpose of writing NodePaths.  This is
  // a TypedWritable since PandaNode is defined in pgraph.
  TypedWritable *_root_node;

  // This is the set of all TypeHandles already written.
  pset<int, int_hash> _types_written;

  // This keeps track of all of the objects we have written out already (or
  // are about to write out), and associates a unique object ID number to each
  // one.
  class StoreState {
  public:
    int _object_id;
    UpdateSeq _written_seq;
    UpdateSeq _modified;
    const ReferenceCount *_refcount;

    StoreState(int object_id) : _object_id(object_id), _refcount(nullptr) {}
  };
  typedef phash_map<const TypedWritable *, StoreState, pointer_hash> StateMap;
  StateMap _state_map;

  // This seq number is incremented each time we write a new object using the
  // top-level write_object() call.  It indicates the current sequence number
  // we are writing, which is updated in the StoreState, above, and used to
  // keep track of which objects may need to be checked for internal updates.
  UpdateSeq _writing_seq;

  // This is initialized to BOC_push in write_object(), then cleared to
  // BOC_adjunct as each object is written, so that only the first object gets
  // written with BOC_push.
  BamObjectCode _next_boc;

  // This is the next object ID that will be assigned to a new object.
  int _next_object_id;
  bool _long_object_id;

  // This is the queue of objects that need to be written when the current
  // object is finished.
  typedef pdeque<const TypedWritable *> ObjectQueue;
  ObjectQueue _object_queue;

  // This is the set of object_id's that we won't be using any more; we'll
  // encode this set into the bam stream so the BamReader will be able to
  // clean up its internal structures.
  typedef vector_int FreedObjectIds;
  FreedObjectIds _freed_object_ids;

  // These are used by register_pta() to unify multiple references to the same
  // PointerToArray.
  typedef phash_map<const void *, int, pointer_hash> PTAMap;
  PTAMap _pta_map;
  int _next_pta_id;
  bool _long_pta_id;

  // The destination to write all the output to.
  DatagramSink *_target;
  bool _needs_init;

  friend class TypedWritable;
};

#include "bamWriter.I"

#endif
