// Filename: eggJointData.h
// Created by:  drose (23Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGJOINTDATA_H
#define EGGJOINTDATA_H

#include <pandatoolbase.h>

#include <eggNode.h>
#include <pointerTo.h>
#include <namable.h>

class EggGroup;

////////////////////////////////////////////////////////////////////
// 	 Class : EggJointData
// Description : This is one node of a hierarchy of EggJointData
//               nodes, each of which represents a single joint of the
//               character hierarchy across all loaded files: the
//               various models, the LOD's of each model, and the
//               various animation channel files.
////////////////////////////////////////////////////////////////////
class EggJointData : public Namable {
public:
  EggJointData();
  virtual ~EggJointData();

  INLINE int get_num_children() const;
  INLINE EggJointData *get_child(int n) const;

  bool matches_name(const string &name) const;
  void add_egg_node(int egg_index, int model_index, EggNode *egg_node);

  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  typedef vector<PT(EggNode)> JointNodes;
  typedef vector<JointNodes> Joints;

  Joints _tables;
  // _tables[i][anim] is the table for the animth bundle of the ith
  // file.

  Joints _joints;
  // _joints[i][model] is the joint for the modelth lod of the ith
  // file.

  LMatrix4d _rest_transform;
  LMatrix4d _net_rest_transform;

  typedef vector<EggJointData *> Children;
  Children _children;

  friend class EggCharacterData;
};

#include "eggJointData.I"

#endif


