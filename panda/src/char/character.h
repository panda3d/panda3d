// Filename: character.h
// Created by:  drose (23Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef CHARACTER_H
#define CHARACTER_H

#include <pandabase.h>

#include "computedVertices.h"

#include <partBundleNode.h>
#include <namedNode.h>
#include <vector_PartGroupStar.h>
#include <pStatCollector.h>
#include <pointerTo.h>
#include <geom.h>

class CharacterJointBundle;
class ComputedVertices;

////////////////////////////////////////////////////////////////////
//       Class : Character
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Character : public PartBundleNode {
protected:
  Character(const Character &copy);

public:
  Character(const string &name = "");
  virtual ~Character();

  virtual Node *make_copy() const;

  virtual bool safe_to_transform() const;

PUBLISHED:
  INLINE CharacterJointBundle *get_bundle() const;

  INLINE ComputedVertices *get_computed_vertices() const;
  INLINE int get_num_parts() const;
  INLINE PartGroup *get_part(int n) const;

public:
  virtual void app_traverse();

PUBLISHED:
  void update();

private:
  void copy_joints(PartGroup *copy, PartGroup *orig);

  typedef map<NodeRelation *, NodeRelation *> ArcMap;
  virtual Node *r_copy_subgraph(TypeHandle graph_type, 
				InstanceMap &inst_map) const;
  void r_copy_char(Node *dest, const Node *source, TypeHandle graph_type, 
		   const Character *from, ArcMap &arc_map);
  PT(Geom) copy_geom(Geom *source, const Character *from);
  void copy_arc_pointers(const Character *from, const ArcMap &arc_map);

  // These are the actual dynamic vertex pools for this character's
  // ComputedVertices--the vertices that it will recompute each frame
  // based on the soft-skinning and morphing requirements.  Note that
  // we store this concretely, instead of as a pointer, just because
  // we don't really need to make it a pointer.
  DynamicVertices _cv;

  // And this is the object that animates them.  It *is* a pointer, so
  // it can be shared between multiple instances of this character.
  PT(ComputedVertices) _computed_vertices;

  // This vector is used by the ComputedVertices object to index back
  // into our joints and sliders.
  typedef vector_PartGroupStar Parts;
  Parts _parts;

  // Statistics
  PStatCollector _char_pcollector;
  static PStatCollector _anim_pcollector;

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);  
  virtual int complete_pointers(vector_typedWriteable &plist, 
				BamReader *manager);

  static TypedWriteable *make_Character(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

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

