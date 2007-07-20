// Filename: character.h
// Created by:  drose (06Mar02)
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
class ComputedVertices;

////////////////////////////////////////////////////////////////////
//       Class : Character
// Description : An animated character, with skeleton-morph animation
//               and either soft-skinned or hard-skinned vertices.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_CHAR Character : public PartBundleNode {
protected:
  Character(const Character &copy, bool copy_bundles);

public:
  Character(const string &name);
  virtual ~Character();

  virtual PandaNode *make_copy() const;
  virtual PandaNode *dupe_for_flatten() const;

  virtual PandaNode *combine_with(PandaNode *other); 
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);

  virtual CPT(TransformState)
    calc_tight_bounds(LPoint3f &min_point, LPoint3f &max_point,
                      bool &found_any,
                      const TransformState *transform,
                      Thread *current_thread) const;

PUBLISHED:
  INLINE CharacterJointBundle *get_bundle(int i) const;
  void merge_bundles(PartBundle *old_bundle, PartBundle *other_bundle);

  CharacterJoint *find_joint(const string &name) const;
  CharacterSlider *find_slider(const string &name) const;

  void write_parts(ostream &out) const;
  void write_part_values(ostream &out) const;

  void update_to_now();
  void update();
  void force_update();

private:
  void do_update();

  typedef pmap<const PandaNode *, PandaNode *> NodeMap;
  typedef pmap<const PartGroup *, PartGroup *> JointMap;
  typedef pmap<const GeomVertexData *, GeomVertexData *> GeomVertexMap;
  typedef pmap<const VertexTransform *, PT(JointVertexTransform) > GeomJointMap;
  typedef pmap<const VertexSlider *, PT(CharacterVertexSlider) > GeomSliderMap;

  virtual void r_copy_children(const PandaNode *from, InstanceMap &inst_map,
                               Thread *current_thread);
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

  // into our joints and sliders.
  //typedef vector_PartGroupStar Parts;
  //Parts _parts;

  double _last_auto_update;

  // Statistics
  PStatCollector _joints_pcollector;
  PStatCollector _skinning_pcollector;
  static PStatCollector _animation_pcollector;

  // This variable is only used temporarily, while reading from the
  // bam file.
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

  friend class CharacterMaker;
};

#include "character.I"

#endif

