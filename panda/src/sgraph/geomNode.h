// Filename: geomNode.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////
#ifndef GEOMNODE_H
#define GEOMNODE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <namedNode.h>
#include <drawable.h>
#include <pointerToArray.h>
#include <luse.h>

class GraphicsStateGuardianBase;
class GeomNodeContext;

////////////////////////////////////////////////////////////////////
//       Class : GeomNode
// Description : Scene graph node that holds drawable geometry
//               elements
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeomNode : public NamedNode {
PUBLISHED:
  GeomNode(const string &name = "");

public:
  GeomNode(const GeomNode &copy);
  void operator = (const GeomNode &copy);
  virtual ~GeomNode();

  virtual Node *make_copy() const;
  virtual void xform(const LMatrix4f &mat);
  virtual Node *combine_with(Node *other); 

  virtual void write(ostream &out, int indent_level = 0) const;

PUBLISHED:
  void write_verbose(ostream &out, int indent_level) const;

public:
  void draw(GraphicsStateGuardianBase *gsg);

  GeomNodeContext *prepare(GraphicsStateGuardianBase *gsg);
  void unprepare();
  void unprepare(GraphicsStateGuardianBase *gsg);
  void clear_gsg(GraphicsStateGuardianBase *gsg);

PUBLISHED:
  int get_num_geoms() const;
  dDrawable *get_geom(int n) const;
  void remove_geom(int n);
  void clear();
  int add_geom(dDrawable *geom);
  void add_geoms_from(const GeomNode *other);

protected:
  virtual void recompute_bound();

private:
  typedef PTA(PT(dDrawable)) Geoms;
  Geoms _geoms;

  // These are essentially similar to the same fields in Geom.  They
  // are used only if the GSG supports a node-level Geom context, as
  // opposed to strictly a Geom-level context.
  GraphicsStateGuardianBase *_prepared_gsg;
  GeomNodeContext *_prepared_context;

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);
  virtual int complete_pointers(vector_typedWritable &plist,
                                BamReader *manager);

  static TypedWritable *make_GeomNode(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

private:
  //This value is only used for the process of re-construction
  //from a binary source. DO NOT ACCESS.  The value is only
  //guaranteed to be accurate during that process
  int _num_geoms;

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    NamedNode::init_type();
    register_type(_type_handle, "GeomNode",
                  NamedNode::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  friend class GeomTransformer;
};

#include "geomNode.I"

#endif
