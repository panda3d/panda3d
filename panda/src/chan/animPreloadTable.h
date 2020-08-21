/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animPreloadTable.h
 * @author drose
 * @date 2008-08-05
 */

#ifndef ANIMPRELOADTABLE_H
#define ANIMPRELOADTABLE_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "ordered_vector.h"
#include "copyOnWriteObject.h"

class BamWriter;
class BamReader;
class Datagram;
class DatagramIterator;
class FactoryParams;

/**
 * This table records data about a list of animations for a particular model,
 * such as number of frames and frame rate.  It's used for implementating
 * asynchronous binding.
 *
 * This table is normally built by an offline tool, such as egg-optchar.
 */
class EXPCL_PANDA_CHAN AnimPreloadTable : public CopyOnWriteObject {
public:
  class AnimRecord {
  public:
    INLINE AnimRecord();
    INLINE bool operator < (const AnimRecord &other) const;

    std::string _basename;
    PN_stdfloat _base_frame_rate;
    int _num_frames;
  };

protected:
  virtual PT(CopyOnWriteObject) make_cow_copy();

PUBLISHED:
  AnimPreloadTable();
  virtual ~AnimPreloadTable();

  int get_num_anims() const;
  int find_anim(const std::string &basename) const;

  INLINE std::string get_basename(int n) const;
  INLINE PN_stdfloat get_base_frame_rate(int n) const;
  INLINE int get_num_frames(int n) const;

  void clear_anims();
  void remove_anim(int n);
  void add_anim(const std::string &basename, PN_stdfloat base_frame_rate, int num_frames);
  void add_anims_from(const AnimPreloadTable *other);

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level) const;

private:
  INLINE void consider_sort() const;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  static TypedWritable *make_from_bam(const FactoryParams &params);

protected:
  void fillin(DatagramIterator &scan, BamReader *manager);

private:
  typedef ov_set<AnimRecord> Anims;
  Anims _anims;
  bool _needs_sort;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CopyOnWriteObject::init_type();
    register_type(_type_handle, "AnimPreloadTable",
                  CopyOnWriteObject::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

inline std::ostream &operator << (std::ostream &out, const AnimPreloadTable &anim) {
  anim.output(out);
  return out;
}

#include "animPreloadTable.I"

#endif
