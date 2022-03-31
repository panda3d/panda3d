/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file movieVideoCursor.h
 * @author jyelon
 * @date 2007-07-02
 */

#ifndef MOVIEVIDEOCURSOR_H
#define MOVIEVIDEOCURSOR_H

#include "pandabase.h"
#include "texture.h"
#include "pointerTo.h"
#include "memoryBase.h"
#include "pStatCollector.h"
#include "deletedChain.h"
#include "typedReferenceCount.h"

class MovieVideo;
class FactoryParams;
class BamWriter;
class BamReader;

/**
 * A MovieVideo is actually any source that provides a sequence of video
 * frames.  That could include an AVI file, a digital camera, or an internet
 * TV station.  A MovieVideoCursor is a handle that lets you read data
 * sequentially from a MovieVideo.
 *
 * Thread safety: each individual MovieVideoCursor must be owned and accessed
 * by a single thread.  It is OK for two different threads to open the same
 * file at the same time, as long as they use separate MovieVideoCursor
 * objects.
 */
class EXPCL_PANDA_MOVIES MovieVideoCursor : public TypedWritableReferenceCount {
protected:
  MovieVideoCursor(MovieVideo *src = nullptr);

PUBLISHED:
  virtual ~MovieVideoCursor();
  PT(MovieVideo) get_source() const;
  INLINE int size_x() const;
  INLINE int size_y() const;
  INLINE int get_num_components() const;
  INLINE double length() const;
  INLINE bool can_seek() const;
  INLINE bool can_seek_fast() const;
  INLINE bool aborted() const;

  INLINE bool ready() const;
  INLINE bool streaming() const;
  void setup_texture(Texture *tex) const;

  virtual bool set_time(double timestamp, int loop_count);

  class EXPCL_PANDA_MOVIES Buffer : public TypedReferenceCount {
  public:
    ALLOC_DELETED_CHAIN(Buffer);
    Buffer(size_t block_size);

  PUBLISHED:
    virtual ~Buffer();

    virtual int compare_timestamp(const Buffer *other) const;
    virtual double get_timestamp() const;

  public:
    unsigned char *_block;
    size_t _block_size;

  private:
    DeletedBufferChain *_deleted_chain;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      TypedReferenceCount::init_type();
      register_type(_type_handle, "MovieVideoCursor::Buffer",
                    TypedReferenceCount::get_class_type());
    }
    virtual TypeHandle get_type() const {
      return get_class_type();
    }
    virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  private:
    static TypeHandle _type_handle;
  };
  virtual PT(Buffer) fetch_buffer();

  virtual void apply_to_texture(const Buffer *buffer, Texture *t, int page);
  virtual void apply_to_texture_rgb(const Buffer *buffer, Texture *t, int page);
  virtual void apply_to_texture_alpha(const Buffer *buffer, Texture *t, int page, int alpha_src);

protected:
  Buffer *get_standard_buffer();
  virtual PT(Buffer) make_new_buffer();

protected:
  PT(MovieVideo) _source;
  int _size_x;
  int _size_y;
  int _num_components;
  double _length;
  bool _can_seek;
  bool _can_seek_fast;
  bool _aborted;
  bool _streaming;
  bool _ready;

  PT(Buffer) _standard_buffer;

  static PStatCollector _copy_pcollector;
  static PStatCollector _copy_pcollector_ram;
  static PStatCollector _copy_pcollector_copy;

public:
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);

protected:
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "MovieVideoCursor",
                  TypedWritableReferenceCount::get_class_type());
    Buffer::init_type();
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "movieVideoCursor.I"
#include "movieVideo.h"

#endif
