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
#include "animControlCollection.h"

#include "pointerTo.h"
#include "iterator_types.h"

class AnimBundle;
class PartBundleNode;
class PartBundleNode;

////////////////////////////////////////////////////////////////////
//       Class : PartBundle
// Description : This is the root of a MovingPart hierarchy.  It
//               defines the hierarchy of moving parts that make up an
//               animatable object.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PartBundle : public PartGroup, public AnimControlCollection {
public:

  // This is passed down through the MovingParts during the
  // do_update() call to specify the channels that are in effect.
  typedef pmap<AnimControl *, float> ChannelBlend;

  typedef first_of_pair_iterator<ChannelBlend::const_iterator> control_iterator;
  typedef ChannelBlend::size_type control_size_type;

protected:
  // The copy constructor is protected; use make_copy() or copy_subgraph().
  PartBundle(const PartBundle &copy);

public:
  PartBundle(const string &name = "");
  virtual PartGroup *make_copy() const;

PUBLISHED:

  // This is the parameter to set_blend_type() and specifies the kind
  // of blending operation to be performed when multiple controls are
  // in effect simultaneously (see set_control_effect()).
  enum BlendType {

    // BT_single means no blending is performed.  Only one AnimControl
    // is allowed to be set at a time in set_control_effect().  In
    // this mode, and this mode only, activating a particular
    // AnimControl (via play(), loop(), or pose()) will implicitly set
    // the bundle's control_effect to 100% of that particular
    // AnimControl, removing any AnimControls previously set.  In
    // other modes, the control_effect must be set manually.
    BT_single,

    // BT_linear does a componentwise average of all blended matrices,
    // which is a linear blend.  The result of this is that if a
    // particular vertex would have been at point P in one animation
    // and point Q in another one, it will end up on the line in
    // between them in the resulting blend animation.  However, this
    // tends to stretch and squash limbs in strange and disturbing
    // ways.
    BT_linear,

    // BT_normalized_linear is a compromise on BT_linear.  The matrix
    // is blended linearly without the scale component, and the
    // blended scale component is applied separately.  This keeps all
    // of the character's body parts in the correct size and shape.
    // However, if the hierarchy is disconnected, body parts can fly
    // off.  It's essential the skeleton hierarchy be completely
    // connected to use this blend mode successully.
    BT_normalized_linear,
  };

  void set_blend_type(BlendType bt);
  INLINE BlendType get_blend_type() const;

  INLINE PartBundleNode *get_node() const;

  void clear_control_effects();
  void set_control_effect(AnimControl *control, float effect);
  float get_control_effect(AnimControl *control);

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level) const;

  PT(AnimControl) bind_anim(AnimBundle *anim,
                            int hierarchy_match_flags = 0);

  bool bind_anim(AnimBundle *anim, const string &name,
                 int hierarchy_match_flags = 0);

public:
  // The following functions may be used to traverse the set of
  // controls applied to the PartBundle.  Beware!  These are not safe
  // to use outside of PANDA.DLL.
  INLINE control_iterator control_begin() const;
  INLINE control_iterator control_end() const;
  INLINE control_size_type control_size() const;

  INLINE const ChannelBlend &get_blend_map() const;

  // The following functions aren't really part of the public
  // interface; they're just public so we don't have to declare a
  // bunch of friends.

  void advance_time(double time);
  bool update();
  bool force_update();
  virtual void control_activated(AnimControl *control);

protected:
  void recompute_net_blend();
  void clear_and_stop_except(AnimControl *control);

  BlendType _blend_type;
  PartBundleNode *_node;

  AnimControl *_last_control_set;
  ChannelBlend _blend;
  float _net_blend;
  bool _anim_changed;

public:
  static void register_with_read_factory(void);
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
};

inline ostream &operator <<(ostream &out, const PartBundle &bundle) {
  bundle.output(out);
  return out;
}

#include "partBundle.I"

#endif
