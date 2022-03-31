/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file partBundle.h
 * @author drose
 * @date 1999-02-22
 */

#ifndef PARTBUNDLE_H
#define PARTBUNDLE_H

#include "pandabase.h"

#include "partGroup.h"
#include "animControl.h"
#include "partSubset.h"
#include "animPreloadTable.h"
#include "pointerTo.h"
#include "thread.h"
#include "cycleData.h"
#include "cycleDataLockedReader.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "luse.h"
#include "pvector.h"
#include "transformState.h"
#include "weakPointerTo.h"
#include "copyOnWritePointer.h"

class Loader;
class AnimBundle;
class PartBundleNode;
class PartBundleNode;
class TransformState;
class AnimPreloadTable;

/**
 * This is the root of a MovingPart hierarchy.  It defines the hierarchy of
 * moving parts that make up an animatable object.
 */
class EXPCL_PANDA_CHAN PartBundle : public PartGroup {
public:

  // This is passed down through the MovingParts during the do_update() call
  // to specify the channels that are in effect.
  typedef pmap<AnimControl *, PN_stdfloat> ChannelBlend;

protected:
  // The copy constructor is protected; use make_copy() or copy_subgraph().
  PartBundle(const PartBundle &copy);

PUBLISHED:
  explicit PartBundle(const std::string &name = "");
  virtual PartGroup *make_copy() const;

  INLINE CPT(AnimPreloadTable) get_anim_preload() const;
  INLINE PT(AnimPreloadTable) modify_anim_preload();
  INLINE void set_anim_preload(AnimPreloadTable *table);
  INLINE void clear_anim_preload();
  void merge_anim_preloads(const PartBundle *other);

  // This is the parameter to set_blend_type() and specifies the kind of
  // blending operation to be performed when multiple controls are in effect
  // simultaneously (see set_control_effect()) or between sequential frames of
  // the animation.
  enum BlendType {
    // BT_linear does a componentwise average of all blended matrices, which
    // is a linear blend.  The result of this is that if a particular vertex
    // would have been at point P in one animation and point Q in another one,
    // it will end up on the line in between them in the resulting blend
    // animation.  However, this tends to stretch and squash limbs in strange
    // and disturbing ways.
    BT_linear,

    // BT_normalized_linear is a compromise on BT_linear.  The matrix is
    // blended linearly without the scale and shear components, and the
    // blended scale and shear components are applied separately.  This keeps
    // all of the character's body parts in the correct size and shape.
    // However, if the hierarchy is disconnected, body parts can fly off.
    // It's essential the skeleton hierarchy be completely connected to use
    // this blend mode successully.
    BT_normalized_linear,

    // BT_componentwise linearly blends all components separately, including
    // H, P, and R, and recomposes the matrix.
    BT_componentwise,

    // BT_componentwise_quat linearly blends all components separately, except
    // for rotation which is blended as a quaternion.
    BT_componentwise_quat,
  };

  INLINE void set_blend_type(BlendType bt);
  INLINE BlendType get_blend_type() const;

  void set_anim_blend_flag(bool anim_blend_flag);
  INLINE bool get_anim_blend_flag() const;

  INLINE void set_frame_blend_flag(bool frame_blend_flag);
  INLINE bool get_frame_blend_flag() const;

  INLINE void set_root_xform(const LMatrix4 &root_xform);
  INLINE void xform(const LMatrix4 &mat);
  INLINE const LMatrix4 &get_root_xform() const;
  PT(PartBundle) apply_transform(const TransformState *transform);

  INLINE int get_num_nodes() const;
  INLINE PartBundleNode *get_node(int n) const;
  MAKE_SEQ(get_nodes, get_num_nodes, get_node);

  MAKE_PROPERTY(blend_type, get_blend_type, set_blend_type);
  MAKE_PROPERTY(anim_blend_flag, get_anim_blend_flag, set_anim_blend_flag);
  MAKE_PROPERTY(frame_blend_flag, get_frame_blend_flag, set_frame_blend_flag);
  MAKE_PROPERTY(root_xform, get_root_xform, set_root_xform);
  MAKE_SEQ_PROPERTY(nodes, get_num_nodes, get_node);

  void clear_control_effects();
  INLINE void set_control_effect(AnimControl *control, PN_stdfloat effect);
  INLINE PN_stdfloat get_control_effect(AnimControl *control) const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level) const;

  PT(AnimControl) bind_anim(AnimBundle *anim,
                            int hierarchy_match_flags = 0,
                            const PartSubset &subset = PartSubset());
  PT(AnimControl) load_bind_anim(Loader *loader,
                                 const Filename &filename,
                                 int hierarchy_match_flags,
                                 const PartSubset &subset,
                                 bool allow_async);
  void wait_pending();

  bool freeze_joint(const std::string &joint_name, const TransformState *transform);
  bool freeze_joint(const std::string &joint_name, const LVecBase3 &pos, const LVecBase3 &hpr, const LVecBase3 &scale);
  bool freeze_joint(const std::string &joint_name, PN_stdfloat value);
  bool control_joint(const std::string &joint_name, PandaNode *node);
  bool release_joint(const std::string &joint_name);

  bool update();
  bool force_update();

public:
  // The following functions aren't really part of the public interface;
  // they're just public so we don't have to declare a bunch of friends.
  virtual void control_activated(AnimControl *control);
  void control_removed(AnimControl *control);
  INLINE void set_update_delay(double delay);

  bool do_bind_anim(AnimControl *control, AnimBundle *anim,
                    int hierarchy_match_flags, const PartSubset &subset);

protected:
  virtual void add_node(PartBundleNode *node);
  virtual void remove_node(PartBundleNode *node);

private:
  class CData;

  void do_set_control_effect(AnimControl *control, PN_stdfloat effect, CData *cdata);
  PN_stdfloat do_get_control_effect(AnimControl *control, const CData *cdata) const;
  void clear_and_stop_intersecting(AnimControl *control, CData *cdata);

  COWPT(AnimPreloadTable) _anim_preload;

  typedef pvector<PartBundleNode *> Nodes;
  Nodes _nodes;

  typedef pmap<WCPT(TransformState), WPT(PartBundle), std::owner_less<WCPT(TransformState)> > AppliedTransforms;
  AppliedTransforms _applied_transforms;

  double _update_delay;

  // This is the data that must be cycled between pipeline stages.
  class CData : public CycleData {
  public:
    CData();
    CData(const CData &copy);

    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return PartBundle::get_class_type();
    }

    BlendType _blend_type;
    bool _anim_blend_flag;
    bool _frame_blend_flag;
    LMatrix4 _root_xform;
    AnimControl *_last_control_set;
    ChannelBlend _blend;
    bool _anim_changed;
    double _last_update;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataLockedReader<CData> CDLockedReader;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;
  typedef CycleDataStageWriter<CData> CDStageWriter;

public:
  static void register_with_read_factory();
  virtual void finalize(BamReader *manager);
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

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
  friend class Character;
  friend class MovingPartBase;
  friend class MovingPartMatrix;
  friend class MovingPartScalar;
};

inline std::ostream &operator <<(std::ostream &out, const PartBundle &bundle) {
  bundle.output(out);
  return out;
}

EXPCL_PANDA_CHAN std::ostream &operator <<(std::ostream &out, PartBundle::BlendType blend_type);
EXPCL_PANDA_CHAN std::istream &operator >>(std::istream &in, PartBundle::BlendType &blend_type);

#include "partBundle.I"

#endif
