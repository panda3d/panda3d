// Filename: eggNode.h
// Created by:  drose (16Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGNODE_H
#define EGGNODE_H

#include <pandabase.h>

#include "eggNamedObject.h"

#include <typeHandle.h>
#include <lmatrix.h>
#include <pointerTo.h>
#include <referenceCount.h>

class EggGroupNode;
class EggAlphaMode;

////////////////////////////////////////////////////////////////////
// 	 Class : EggNode
// Description : A base class for things that may be directly added
//               into the egg hierarchy.  This includes groups,
//               joints, polygons, vertex pools, etc., but does not
//               include things like vertices.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggNode : public EggNamedObject {
public:
  INLINE EggNode(const string &name = "");
  INLINE EggNode(const EggNode &copy);
  INLINE EggNode &operator = (const EggNode &copy);

  INLINE EggGroupNode *get_parent() const;
  INLINE int get_depth() const;
  INLINE bool is_under_instance() const;
  INLINE bool is_under_transform() const;
  INLINE bool is_local_coord() const;

  virtual EggAlphaMode *determine_alpha_mode();
  virtual EggAlphaMode *determine_draw_order();
  virtual EggAlphaMode *determine_bin();

  INLINE const LMatrix4d &get_vertex_frame() const;
  INLINE const LMatrix4d &get_node_frame() const;
  INLINE const LMatrix4d &get_vertex_frame_inv() const;
  INLINE const LMatrix4d &get_node_frame_inv() const;
  INLINE const LMatrix4d &get_vertex_to_node() const;

  INLINE void transform(const LMatrix4d &mat);

  virtual void write(ostream &out, int indent_level) const=0;

#ifndef NDEBUG
  void test_under_integrity() const;
#else
  void test_under_integrity() const { }
#endif  // NDEBUG


protected:
  enum UnderFlags {
    UF_under_instance  = 0x001,
    UF_under_transform = 0x002,
    UF_local_coord     = 0x004,
  };

  virtual void update_under(int depth_offset);
  virtual void adjust_under();

  virtual void r_transform(const LMatrix4d &mat, const LMatrix4d &inv,
			   CoordinateSystem to_cs);
  virtual void r_mark_coordsys(CoordinateSystem cs);

  // These members are updated automatically by prepare_add_child(),
  // prepare_remove_child(), and update_under().  Other functions
  // shouldn't be fiddling with them.

  EggGroupNode *_parent;
  int _depth;
  int _under_flags;

  typedef RefCountObj<LMatrix4d> MatrixFrame;

  PT(MatrixFrame) _vertex_frame;
  PT(MatrixFrame) _node_frame;
  PT(MatrixFrame) _vertex_frame_inv;
  PT(MatrixFrame) _node_frame_inv;
  PT(MatrixFrame) _vertex_to_node;


public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggNamedObject::init_type();
    register_type(_type_handle, "EggNode",
		  EggNamedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;

friend class EggGroupNode;
};

#include "eggNode.I"

#endif
