// Filename: softNodeDesc.h
// Created by:  masad (03Oct03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef SOFTNODEDESC_H
#define SOFTNODEDESC_H

#ifdef _MIN
#undef _MIN
#endif
#ifdef _MAX
#undef _MAX
#endif

#include "pandatoolbase.h"

#include "eggVertex.h"
#include "eggVertexPool.h"
#include "referenceCount.h"
#include "pointerTo.h"
#include "namable.h"

#include <SAA.h>

class EggGroup;
class EggTable;
class EggXfmSAnim;

////////////////////////////////////////////////////////////////////
//       Class : SoftNodeDesc
// Description : Describes a single instance of a node aka element in the Soft
//               scene graph, relating it to the corresponding egg
//               structures (e.g. node, group, or table entry) that
//               will be created.
////////////////////////////////////////////////////////////////////
class SoftNodeDesc : public ReferenceCount, public Namable {
public:
  SoftNodeDesc(SoftNodeDesc *parent=NULL, const string &name = string());
  ~SoftNodeDesc();

  void set_parent(SoftNodeDesc *parent);
  void force_set_parent(SoftNodeDesc *parent);
  void set_model(SAA_Elem *model);
  bool has_model() const;
  SAA_Elem *get_model() const;

  bool is_joint() const;
  bool is_junk() const;
  void set_joint();
  bool is_joint_parent() const;
  bool is_partial(char *search_prefix);

  SoftNodeDesc *_parent;
  SoftNodeDesc *_parentJoint; // keep track of who is your parent joint
  typedef pvector< PT(SoftNodeDesc) > Children;
  Children _children;

private:
  void clear_egg();
  void mark_joint_parent();
  void check_joint_parent();
  void check_junk(bool parent_junk);
  void check_pseudo_joints(bool joint_above);

  void set_parentJoint(SAA_Scene *scene, SoftNodeDesc *lastJoint);

  SAA_ModelType type;

  SAA_Elem *_model;

  EggGroup *_egg_group;
  EggTable *_egg_table;
  EggXfmSAnim *_anim;

  enum JointType {
    JT_none,         // Not a joint.
    JT_joint,        // An actual joint in Soft.
    JT_pseudo_joint, // Not a joint in Soft, but treated just like a
                     // joint for the purposes of the converter.
    JT_joint_parent, // A parent or ancestor of a joint or pseudo joint.
    JT_junk,         // originated from con-/fly-/car_rig/bars etc.
  };
  JointType _joint_type;

public:

  char **texNameArray;
  int *uRepeat, *vRepeat;
  PN_stdfloat matrix[4][4];

  const char *fullname;

  int numTri;
  //  int numShapes;
  int numTexLoc;
  int numTexGlb;
  int *numTexTri;

  // if the node is a MNSRF
  int numNurbTexLoc;
  int numNurbTexGlb;
  int numNurbMats;

  PN_stdfloat *uScale;
  PN_stdfloat *vScale;
  PN_stdfloat *uOffset;
  PN_stdfloat *vOffset;
  
  SAA_Boolean valid;
  SAA_Boolean uv_swap;
  //  SAA_Boolean visible;
  SAA_Elem *textures;
  SAA_Elem *materials;
  SAA_SubElem *triangles;
  SAA_GeomType gtype;

  EggGroup *get_egg_group()const {return _egg_group;}

  void get_transform(SAA_Scene *scene, EggGroup *egg_group, bool global);
  void get_joint_transform(SAA_Scene *scene, EggGroup *egg_group, EggXfmSAnim *anim, bool global);
  void load_poly_model(SAA_Scene *scene, SAA_ModelType type);
  void load_nurbs_model(SAA_Scene *scene, SAA_ModelType type);

  void make_morph_table(PN_stdfloat time);
  void make_linear_morph_table(int numShapes, PN_stdfloat time);
  void make_weighted_morph_table(int numShapes, PN_stdfloat time);
  void make_expression_morph_table(int numShapes, PN_stdfloat time);

  void make_vertex_offsets(int numShapes);
  int find_shape_vert(LPoint3d p3d, SAA_DVector *vertices, int numVert);

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ReferenceCount::init_type();
    Namable::init_type();
    register_type(_type_handle, "SoftNodeDesc",
                  ReferenceCount::get_class_type(),
                  Namable::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class SoftNodeTree;
};

class SoftToEggConverter;
extern SoftToEggConverter stec;

#endif
