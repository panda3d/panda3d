// Filename: eggJointData.h
// Created by:  drose (23Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGJOINTDATA_H
#define EGGJOINTDATA_H

#include <pandatoolbase.h>

#include "eggComponentData.h"

////////////////////////////////////////////////////////////////////
// 	 Class : EggJointData
// Description : This is one node of a hierarchy of EggJointData
//               nodes, each of which represents a single joint of the
//               character hierarchy across all loaded files: the
//               various models, the LOD's of each model, and the
//               various animation channel files.
////////////////////////////////////////////////////////////////////
class EggJointData : public EggComponentData {
public:
  EggJointData(EggCharacterCollection *collection,
	       EggCharacterData *char_data);

  INLINE int get_num_children() const;
  INLINE EggJointData *get_child(int n) const;
  EggJointData *find_joint(const string &name);

  virtual void add_back_pointer(int model_index, EggObject *egg_object);
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  typedef vector<EggJointData *> Children;
  Children _children;

  friend class EggCharacterCollection;
};

#include "eggJointData.I"

#endif
