// Filename: geomNode.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
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
class AllAttributesWrapper;

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

  virtual void write(ostream &out, int indent_level = 0) const;

PUBLISHED:
  void write_verbose(ostream &out, int indent_level) const;

public:
  void draw(GraphicsStateGuardianBase *gsg);

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

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);  
  virtual int complete_pointers(vector_typedWriteable &plist, 
				BamReader *manager);

  static TypedWriteable *make_GeomNode(const FactoryParams &params);

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
