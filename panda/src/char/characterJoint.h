// Filename: characterJoint.h
// Created by:  drose (23Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef CHARACTERJOINT_H
#define CHARACTERJOINT_H

#include <pandabase.h>

#include <movingPartMatrix.h>
#include <namedNode.h>
#include <nodeRelation.h>
#include <pt_NamedNode.h>

////////////////////////////////////////////////////////////////////
//       Class : CharacterJoint
// Description : This represents one joint of the character's
//               animation, containing an animating transform matrix.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CharacterJoint : public MovingPartMatrix {
protected:
  CharacterJoint();
  CharacterJoint(const CharacterJoint &copy);

public:
  CharacterJoint(PartGroup *parent, const string &name,
		 const LMatrix4f &initial_value);

  virtual PartGroup *make_copy() const;

  virtual void update_internals(PartGroup *parent, bool self_changed,
				bool parent_changed);

PUBLISHED:
  bool add_net_transform(NodeRelation *arc);
  bool remove_net_transform(NodeRelation *arc);
  bool has_net_transform(NodeRelation *arc) const;
  void clear_net_transforms();

  bool add_local_transform(NodeRelation *arc);
  bool remove_local_transform(NodeRelation *arc);
  bool has_local_transform(NodeRelation *arc) const;
  void clear_local_transforms();

private:
  typedef set<PT(NodeRelation)> ArcList;
  ArcList _net_transform_arcs;
  ArcList _local_transform_arcs;

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);  
  virtual int complete_pointers(vector_typedWriteable &plist, 
				BamReader *manager);

  static TypedWriteable *make_CharacterJoint(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

private:
  int _num_net_arcs, _num_local_arcs;

public:
  // The _geom_node member just holds a temporary pointer to a node
  // for the CharacterMaker's convenenience while creating the
  // character.  It does not store any meaningful value after
  // creation is complete.
  PT_NamedNode _geom_node;

  // These are filled in as the joint animates.
  LMatrix4f _net_transform;
  LMatrix4f _initial_net_transform_inverse;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MovingPartMatrix::init_type();
    register_type(_type_handle, "CharacterJoint",
		  MovingPartMatrix::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class Character;
};

#endif


