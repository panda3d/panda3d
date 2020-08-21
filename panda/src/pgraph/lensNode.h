/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lensNode.h
 * @author drose
 * @date 2002-02-26
 */

#ifndef LENSNODE_H
#define LENSNODE_H

#include "pandabase.h"

#include "pandaNode.h"
#include "lens.h"
#include "perspectiveLens.h"
#include "pointerTo.h"

/**
 * A node that contains a Lens.  The most important example of this kind of
 * node is a Camera, but other kinds of nodes also contain a lens (for
 * instance, a Spotlight).
 */
class EXPCL_PANDA_PGRAPH LensNode : public PandaNode {
PUBLISHED:
  explicit LensNode(const std::string &name, Lens *lens = nullptr);

protected:
  LensNode(const LensNode &copy);
public:
  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level = 0) const;

  virtual void xform(const LMatrix4 &mat);
  virtual PandaNode *make_copy() const;

PUBLISHED:
  INLINE void copy_lens(const Lens &lens);
  INLINE void copy_lens(int index, const Lens &lens);
  INLINE void set_lens(Lens *lens);
  void set_lens(int index, Lens *lens);
  INLINE Lens *get_lens(int index = 0) const;

  bool set_lens_active(int index, bool active);
  INLINE bool get_lens_active(int index) const;

  INLINE bool activate_lens(int index);
  INLINE bool deactivate_lens(int index);

  INLINE bool is_in_view(const LPoint3 &pos);
  bool is_in_view(int index, const LPoint3 &pos);

  void show_frustum();
  void hide_frustum();

protected:
  PT(PandaNode) _shown_frustum;

  class LensSlot {
  public:
    PT(Lens) _lens;
    bool _is_active;
  };

  typedef pvector<LensSlot> Lenses;
  Lenses _lenses;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist,
                                BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "LensNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "lensNode.I"

#endif
