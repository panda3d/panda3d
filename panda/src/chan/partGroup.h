// Filename: partGroup.h
// Created by:  drose (22Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef PARTGROUP_H
#define PARTGROUP_H

#include <pandabase.h>

#include <typedWritableReferenceCount.h>
#include <pointerTo.h>
#include <namable.h>
#include <typedef.h>

#include <list>

class AnimControl;
class AnimGroup;
class PartBundle;
class BamReader;

////////////////////////////////////////////////////////////////////
//       Class : PartGroup
// Description : This is the base class for PartRoot and
//               MovingPart.  It defines a hierarchy of MovingParts.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PartGroup : public TypedWritableReferenceCount, public Namable {
public:
  // This enum defines bits which may be passed into check_hierarchy()
  // and PartBundle::bind_anim() to allow an inexact match of channel
  // hierarchies.  This specifies conditions that we don't care about
  // enforcing.
  enum HierarchyMatchFlags {
    HMF_ok_part_extra          = 0x01,
    HMF_ok_anim_extra          = 0x02,
    HMF_ok_wrong_root_name     = 0x04,
  };

protected:
  // The default constructor is protected: don't try to create a
  // PartGroup without a parent.  To create a PartGroup hierarchy, you
  // must first create a PartBundle, and use that to create any
  // subsequent children.
  INLINE PartGroup(const string &name = "");
  INLINE PartGroup(const PartGroup &copy);

public:
  // This is the normal PartGroup constructor.
  PartGroup(PartGroup *parent, const string &name);
  virtual ~PartGroup();

  virtual PartGroup *make_copy() const;
  PartGroup *copy_subgraph() const;

PUBLISHED:
  int get_num_children() const;
  PartGroup *get_child(int n) const;
  PartGroup *find_child(const string &name) const;

  virtual void write(ostream &out, int indent_level) const;
  virtual void write_with_value(ostream &out, int indent_level) const;

public:
  virtual TypeHandle get_value_type() const;

  void sort_descendants();
  bool check_hierarchy(const AnimGroup *anim,
		       const PartGroup *parent,
		       int hierarchy_match_flags = 0) const;

  virtual void do_update(PartBundle *root, PartGroup *parent,
			 bool parent_changed, bool anim_changed);

protected:
  void write_descendants(ostream &out, int indent_level) const;
  void write_descendants_with_value(ostream &out, int indent_level) const;

  virtual void pick_channel_index(list<int> &holes, int &next) const;
  virtual void bind_hierarchy(AnimGroup *anim, int channel_index);

  typedef vector< PT(PartGroup) > Children;
  Children _children;

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);  
  virtual int complete_pointers(vector_typedWritable &plist, 
				BamReader *manager);

  static TypedWritable *make_PartGroup(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

private:
  int _num_children;

public:

  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "PartGroup",
		  TypedWritableReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class Character;
};

#include "partGroup.I"

#endif


