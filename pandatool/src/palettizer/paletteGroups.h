/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file paletteGroups.h
 * @author drose
 * @date 2000-11-30
 */

#ifndef PALETTEGROUPS_H
#define PALETTEGROUPS_H

#include "pandatoolbase.h"
#include "typedWritable.h"
#include "pset.h"

class PaletteGroup;
class FactoryParams;

/**
 * A set of PaletteGroups.  This presents an interface very like an STL set,
 * with a few additional functions.
 */
class PaletteGroups : public TypedWritable {
private:
  typedef pset<PaletteGroup *> Groups;

public:
#ifndef WIN32_VC
  typedef Groups::const_pointer pointer;
  typedef Groups::const_pointer const_pointer;
#endif
  typedef Groups::const_reference reference;
  typedef Groups::const_reference const_reference;
  typedef Groups::const_iterator iterator;
  typedef Groups::const_iterator const_iterator;
  typedef Groups::const_reverse_iterator reverse_iterator;
  typedef Groups::const_reverse_iterator const_reverse_iterator;
  typedef Groups::size_type size_type;
  typedef Groups::difference_type difference_type;

  PaletteGroups();
  PaletteGroups(const PaletteGroups &copy);
  void operator =(const PaletteGroups &copy);

  void insert(PaletteGroup *group);
  size_type count(PaletteGroup *group) const;
  void make_complete(const PaletteGroups &a);
  void make_union(const PaletteGroups &a, const PaletteGroups &b);
  void make_intersection(const PaletteGroups &a, const PaletteGroups &b);
  void remove_null();
  void clear();

  bool empty() const;
  size_type size() const;
  iterator begin() const;
  iterator end() const;

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;

private:
  void r_make_complete(Groups &result, PaletteGroup *group);

  Groups _groups;

  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *writer, Datagram &datagram);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

protected:
  static TypedWritable *make_PaletteGroups(const FactoryParams &params);

public:
  void fillin(DatagramIterator &scan, BamReader *manager);

private:
  // This value is only filled in while reading from the bam file; don't use
  // it otherwise.
  int _num_groups;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritable::init_type();
    register_type(_type_handle, "PaletteGroups",
                  TypedWritable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const PaletteGroups &groups) {
  groups.output(out);
  return out;
}

#endif
