// Filename: animGroup.h
// Created by:  drose (21Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef ANIMGROUP_H
#define ANIMGROUP_H

#include <pandabase.h>

#include <typedWritableReferenceCount.h>
#include <pointerTo.h>
#include <namable.h>

class AnimBundle;
class BamReader;

////////////////////////////////////////////////////////////////////
//       Class : AnimGroup
// Description : This is the base class for AnimChannel and
//               AnimBundle.  It implements a hierarchy of
//               AnimChannels.  The root of the hierarchy must be an
//               AnimBundle.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AnimGroup : public TypedWritableReferenceCount, public Namable {
protected:
  // The default constructor is protected: don't try to create an
  // AnimGroup without a parent.  To create an AnimChannel hierarchy,
  // you must first create an AnimBundle, and use that to create any
  // subsequent children.
  AnimGroup(const string &name = "") : Namable(name) { }

public:
  // This is the normal AnimGroup constructor.
  AnimGroup(AnimGroup *parent, const string &name);

  int get_num_children() const;
  AnimGroup *get_child(int n) const;
  AnimGroup *find_child(const string &name) const;

  virtual TypeHandle get_value_type() const;

  void sort_descendants();
 
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level) const;

protected:
  void write_descendants(ostream &out, int indent_level) const;

protected:
  typedef vector< PT(AnimGroup) > Children;
  Children _children;
  AnimBundle *_root;

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);  
  virtual int complete_pointers(vector_typedWritable &plist, 
                                BamReader *manager);

  static TypedWritable *make_AnimGroup(const FactoryParams &params);

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
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "AnimGroup",
                  TypedWritableReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

inline ostream &operator << (ostream &out, const AnimGroup &anim) {
  anim.output(out);
  return out;
}

#include "animGroup.I"

#endif


