// Filename: partBundle.h
// Created by:  drose (22Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef PARTBUNDLE_H
#define PARTBUNDLE_H

#include "pandabase.h"

#include "partGroup.h"
#include "animControl.h"
#include "partSubset.h"
#include "pointerTo.h"
#include "thread.h"
#include "cycleData.h"
#include "cycleDataLockedReader.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "luse.h"
#include "pvector.h"

class AnimBundle;
class PartBundleNode;
class PartBundleNode;

////////////////////////////////////////////////////////////////////
//       Class : PartBundle
// Description : This is the root of a MovingPart hierarchy.  It
//               defines the hierarchy of moving parts that make up an
//               animatable object.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PartBundle : public PartGroup {
public:

  // This is passed down through the MovingParts during the
  // do_update() call to specify the channels that are in effect.
  typedef pmap<AnimControl *, float> ChannelBlend;

protected:
  // The copy constructor is protected; use make_copy() or copy_subgraph().
  PartBundle(const PartBundle &copy);

public:
  PartBundle(const string &name = "");
  virtual PartGroup *make_copy() const;

PUBLISHED:

  // This is the parameter to set_blend_type() and specifies the kind
  // of blending operation to be performed when multiple controls are
  // in effect simultaneously (see set_control_effect()) or between
  // sequential frames of the animation.
  enum BlendType {
    // BT_linear does a componentwise average of all blended matrices,
    // which is a linear blend.  The result of this is that if a
    // particular vertex would have been at point P in one animation
    // and point Q in another one, it will end up on the line in
    // between them in the resulting blend animation.  However, this
    // tends to stretch and squash limbs in strange and disturbing
    // ways.
    BT_linear,

    // BT_normalized_linear is a compromise on BT_linear.  The matrix
    // is blended linearly without the scale and shear components, and
    // the blended scale and shear components are applied separately.
    // This keeps all of the character's body parts in the correct
    // size and shape.  However, if the hierarchy is disconnected,
    // body parts can fly off.  It's essential the skeleton hierarchy
    // be completely connected to use this blend mode successully.
    BT_normalized_linear,

    // BT_componentwise linearly blends all components separately,
    // including H, P, and R, and recomposes the matrix.
    BT_componentwise,

    // BT_componentwise_quat linearly blends all components
    // separately, except for rotation which is blended as a
    // quaternion.
    BT_componentwise_quat,
  };

  INLINE void set_blend_type(BlendType bt);
  INLINE BlendType get_blend_type() const;

  void set_anim_blend_flag(bool anim_blend_flag);
  INLINE bool get_anim_blend_flag() const;

  INLINE void set_frame_blend_flag(bool frame_blend_flag);
  INLINE bool get_frame_blend_flag() const;

  INLINE void set_root_xform(const LMatrix4f &root_xform);
  INLINE void xform(const LMatrix4f &mat);
  INLINE const LMatrix4f &get_root_xform() const;

  INLINE int get_num_nodes() const;
  INLINE PartBundleNode *get_node(int n) const;

  void clear_control_effects();
  INLINE void set_control_effect(AnimControl *control, float effect);
  INLINE float get_control_effect(AnimControl *control) const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level) const;

  PT(AnimControl) bind_anim(AnimBundle *anim,
                            int hierarchy_match_flags = 0, 
                            const PartSubset &subset = PartSubset());

  bool update();
  bool force_update();

public:
  // The following functions aren't really part of the public
  // interface; they're just public so we don't have to declare a
  // bunch of friends.
  virtual void control_activated(AnimControl *control);

protected:
  virtual void add_node(PartBundleNode *node);
  virtual void remove_node(PartBundleNode *node);

private:
  class CData;

  void do_set_control_effect(AnimControl *control, float effect, CData *cdata);
  float do_get_control_effect(AnimControl *control, const CData *cdata) const;
  void recompute_net_blend(CData *cdata);
  void clear_and_stop_intersecting(AnimControl *control, CData *cdata);

  typedef pvector<PartBundleNode *> Nodes;
  Nodes _nodes;

  // This is the data that must be cycled between pipeline stages.
  class CData : public CycleData {
  public:
    CData();
    CData(const CData &copy);

    virtual CycleData *make_copy() const;
    virtual TypeHandle get_parent_type() const {
      return PartBundle::get_class_type();
    }

    BlendType _blend_type;
    bool _anim_blend_flag;
    bool _frame_blend_flag;
    LMatrix4f _root_xform;
    AnimControl *_last_control_set;
    ChannelBlend _blend;
    float _net_blend;
    bool _anim_changed;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataLockedReader<CData> CDLockedReader;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

public:
  static void register_with_read_factory();
  virtual void finalize(BamReader *manager);

  static TypedWritable *make_PartBundle(const FactoryParams &params);

public:

  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PartGroup::init_type();
    register_type(_type_handle, "PartBundle",
                  PartGroup::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class PartBundleNode;
  friend class MovingPartBase;
  friend class MovingPartMatrix;
  friend class MovingPartScalar;
};

inline ostream &operator <<(ostream &out, const PartBundle &bundle) {
  bundle.output(out);
  return out;
}

ostream &operator <<(ostream &out, PartBundle::BlendType blend_type);
istream &operator >>(istream &in, PartBundle::BlendType &blend_type);

#include "partBundle.I"

#endif
