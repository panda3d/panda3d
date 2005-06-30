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
class EXPCL_PANDA Character : public PartBundleNode {
protected:
  Character(const Character &copy);

public:
  Character(const string &name);
  virtual ~Character();

  virtual PandaNode *make_copy() const;

  virtual bool safe_to_transform() const;
  virtual bool safe_to_flatten_below() const;
  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);

PUBLISHED:
  INLINE CharacterJointBundle *get_bundle() const;

  INLINE int get_num_parts() const;
  INLINE PartGroup *get_part(int n) const;

  INLINE CharacterJoint *find_joint(const string &name) const;
  INLINE CharacterSlider *find_slider(const string &name) const;

  INLINE void write_parts(ostream &out) const;
  INLINE void write_part_values(ostream &out) const;

  void update_to_now();
  void update();
  void force_update();

private:
  void do_update();
  void copy_joints(PartGroup *copy, PartGroup *orig);

  typedef pmap<const PandaNode *, PandaNode *> NodeMap;
  typedef pmap<const VertexTransform *, PT(JointVertexTransform) > JointMap;
  typedef pmap<const VertexSlider *, PT(CharacterVertexSlider) > SliderMap;

  virtual void r_copy_children(const PandaNode *from, InstanceMap &inst_map);
  void r_copy_char(PandaNode *dest, const PandaNode *source,
                   const Character *from, NodeMap &node_map,
                   JointMap &joint_map, SliderMap &slider_map);
  PT(Geom) copy_geom(const Geom *source, const Character *from,
                     JointMap &joint_map, SliderMap &slider_map);
  void copy_node_pointers(const Character *from, const NodeMap &node_map);
  CPT(TransformTable) redirect_transform_table(const TransformTable *source,
                                               JointMap &joint_map);
  CPT(TransformBlendTable) redirect_transform_blend_table(const TransformBlendTable *source,
                                                          JointMap &joint_map);
  CPT(SliderTable) redirect_slider_table(const SliderTable *source,
                                         SliderMap &slider_map);

  PT(JointVertexTransform) redirect_joint(const VertexTransform *vt, JointMap &joint_map);
  PT(CharacterVertexSlider) redirect_slider(const VertexSlider *vs, SliderMap &slider_map);

  // This vector is used by the ComputedVertices object to index back
  // into our joints and sliders.
  typedef vector_PartGroupStar Parts;
  Parts _parts;

  // Statistics
  PStatCollector _joints_pcollector;
  PStatCollector _skinning_pcollector;
  static PStatCollector _animation_pcollector;

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
  friend class ComputedVerticesMaker;
  friend class ComputedVertices;
};

#include "character.I"

#endif

