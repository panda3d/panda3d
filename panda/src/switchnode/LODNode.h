// Filename: LODNode.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef LODNODE_H
#define LODNODE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <namedNode.h>
#include <LOD.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : LODNode
// Description : A node that implements level of detail
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA LODNode : public NamedNode {
public:
  INLINE LODNode(const string &name = "");
  INLINE LODNode(const LODNode &copy);

  virtual Node *make_copy() const;
  virtual void xform(const LMatrix4f &mat);

  // The sense of in vs. out distances is as if the object were coming
  // towards you from far away: it switches "in" at the far distance,
  // and switches "out" at the close distance.  Thus, "in" should be
  // larger than "out".

  INLINE void add_switch(float in, float out);
  INLINE bool set_switch(int index, float in, float out);
  INLINE void clear_switches(void);

  virtual bool sub_render(const AllAttributesWrapper &attrib,
			  AllTransitionsWrapper &trans,
			  GraphicsStateGuardianBase *gsgbase);
  virtual bool has_sub_render() const;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &me);  

  static TypedWriteable *make_LODNode(const FactoryParams &params);

protected:
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  LOD    			_lod;

public:
  static TypeHandle get_class_type( void ) {
      return _type_handle;
  }
  static void init_type( void ) {
    NamedNode::init_type();
    register_type( _type_handle, "LODNode",
                NamedNode::get_class_type() );
  }
  virtual TypeHandle get_type( void ) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle		_type_handle;
};

#include "LODNode.I"

#endif
