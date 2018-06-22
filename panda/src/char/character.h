/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file character.h
 * @author drose
 * @date 2002-03-06
 */

#ifndef CHARACTER_H
#define CHARACTER_H

#include "pandabase.h"

#include "characterJoint.h"
#include "characterSlider.h"
#include "characterVertexSlider.h"
#include "jointVertexTransform.h"
#include "partBundleNode.h"
#include "vector_PartGroupStar.h"
#include "pointerTo.h"
#include "geom.h"
#include "pStatCollector.h"
#include "transformTable.h"
#include "transformBlendTable.h"
#include "sliderTable.h"

class CharacterJointBundle;

/**
 * An animated character, with skeleton-morph animation and either soft-
 * skinned or hard-skinned vertices.
 */
class EXPCL_PANDA_CHAR Character : public PartBundleNode {
protected:
  Character(const Character &copy, bool copy_bundles);

PUBLISHED:
  explicit Character(const std::string &name);
  virtual ~Character();

public:
  virtual PandaNode *make_copy() const;
  virtual PandaNode *dupe_for_flatten() const;

  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);

  virtual CPT(TransformState)
    calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                      bool &found_any,
                      const TransformState *transform,
                      Thread *current_thread) const;

PUBLISHED:
  virtual PandaNode *combine_with(PandaNode *other);

  INLINE CharacterJointBundle *get_bundle(int i) const;
  void merge_bundles(PartBundle *old_bundle, PartBundle *other_bundle);
  void merge_bundles(PartBundleHandle *old_bundle_handle,
                     PartBundleHandle *other_bundle_handle);

  void set_lod_animation(const LPoint3 &center,
                         PN_stdfloat far_distance, PN_stdfloat near_distance,
                         PN_stdfloat delay_factor);
  void clear_lod_animation();

  CharacterJoint *find_joint(const std::string &name) const;
  CharacterSlider *find_slider(const std::string &name) const;

  void write_parts(std::ostream &out) const;
  void write_part_values(std::ostream &out) const;

  void update_to_now();
  void update();
  void force_update();

protected:
  virtual void r_copy_children(const PandaNode *from, InstanceMap &inst_map,
                               Thread *current_thread);
  virtual void update_bundle(PartBundleHandle *old_bundle_handle,
                             PartBundle *new_bundle);
  CPT(TransformState) get_rel_transform(CullTraverser *trav, CullTraverserData &data);

private:
  void do_update();
  void set_lod_current_delay(double delay);

  typedef pmap<const PandaNode *, PandaNode *> NodeMap;
  typedef pmap<const PartGroup *, PartGroup *> JointMap;
  typedef pmap<const GeomVertexData *, GeomVertexData *> GeomVertexMap;
  typedef pmap<const VertexTransform *, PT(JointVertexTransform) > GeomJointMap;
  typedef pmap<const VertexSlider *, PT(CharacterVertexSlider) > GeomSliderMap;

  void fill_joint_map(JointMap &joint_map, PartGroup *copy, PartGroup *orig);
  void r_merge_bundles(Character::JointMap &joint_map,
                       PartGroup *old_group, PartGroup *new_group);
  void r_copy_char(PandaNode *dest, const PandaNode *source,
                   const Character *from, NodeMap &node_map,
                   const JointMap &joint_map, GeomVertexMap &gvmap,
                   GeomJointMap &gjmap, GeomSliderMap &gsmap);
  void r_update_geom(PandaNode *node,
                     const JointMap &joint_map, GeomVertexMap &gvmap,
                     GeomJointMap &gjmap, GeomSliderMap &gsmap);
  PT(Geom) copy_geom(const Geom *source,
                     const JointMap &joint_map, GeomVertexMap &gvmap,
                     GeomJointMap &gjmap, GeomSliderMap &gsmap);
  void copy_node_pointers(const Character::NodeMap &node_map,
                          PartGroup *dest, const PartGroup *source);

  CPT(TransformTable) redirect_transform_table(const TransformTable *source,
                                               const JointMap &joint_map,
                                               GeomJointMap &gjmap);
  CPT(TransformBlendTable) redirect_transform_blend_table
  (const TransformBlendTable *source, const JointMap &joint_map,
   GeomJointMap &gjmap);
  CPT(SliderTable) redirect_slider_table(const SliderTable *source,
                                         GeomSliderMap &gsmap);

  PT(JointVertexTransform) redirect_joint(const VertexTransform *vt,
                                          const JointMap &joint_map,
                                          GeomJointMap &gjmap);
  PT(CharacterVertexSlider) redirect_slider(const VertexSlider *vs, GeomSliderMap &gsmap);

  void r_clear_joint_characters(PartGroup *part);

  // into our joints and sliders.  typedef vector_PartGroupStar Parts; Parts
  // _parts;

  double _last_auto_update;

  int _view_frame;
  double _view_distance2;

  LPoint3 _lod_center;
  PN_stdfloat _lod_far_distance;
  PN_stdfloat _lod_near_distance;
  PN_stdfloat _lod_delay_factor;
  bool _do_lod_animation;

  // Statistics
  PStatCollector _joints_pcollector;
  PStatCollector _skinning_pcollector;
  static PStatCollector _animation_pcollector;

  // This variable is only used temporarily, while reading from the bam file.
  unsigned int _temp_num_parts;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist,
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
    PartBundleNode::init_type();
    register_type(_type_handle, "Character",
                  PartBundleNode::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "character.I"

#endif
