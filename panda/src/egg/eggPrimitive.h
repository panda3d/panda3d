// Filename: eggPrimitive.h
// Created by:  drose (16Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGPRIMITIVE_H
#define EGGPRIMITIVE_H

#include <pandabase.h>

#include "eggNode.h"
#include "eggAttributes.h"
#include "eggVertex.h"
#include "eggTexture.h"
#include "eggMaterial.h"
#include "eggAlphaMode.h"
#include <pointerTo.h>
#include <vector>

class EggVertexPool;

////////////////////////////////////////////////////////////////////
// 	 Class : EggPrimitive
// Description : A base class for any of a number of kinds of geometry
//               primitives: polygons, point lights, nurbs patches,
//               parametrics curves, etc.  Things with a set of
//               vertices and some rendering properties like color.
//
//               An EggPrimitive is an STL-style container of pointers
//               to EggVertex's.  In fact, it IS a vector, and can be
//               manipulated in all the ways that vectors can.
//               However, it is necessary that all vertices belong to
//               the same vertex pool.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggPrimitive : public EggNode, public EggAttributes,
		     public EggAlphaMode
{

  // This is a bit of private interface stuff that must be here as a
  // forward reference.  This allows us to define the EggPrimitive as
  // an STL container.

private:
  typedef vector<PT(EggVertex)> Vertices;

  // Here begins the actual public interface to EggPrimitive.

public:
  INLINE EggPrimitive(const string &name = "");
  INLINE EggPrimitive(const EggPrimitive &copy);
  INLINE EggPrimitive &operator = (const EggPrimitive &copy);
  INLINE ~EggPrimitive();

  INLINE void set_texture(PT(EggTexture) texture);
  INLINE void clear_texture();
  INLINE PT(EggTexture) get_texture() const;
  INLINE bool has_texture() const;

  INLINE void set_material(PT(EggMaterial) material);
  INLINE void clear_material();
  INLINE PT(EggMaterial) get_material() const;
  INLINE bool has_material() const;

  INLINE void set_bface_flag(bool flag);
  INLINE bool get_bface_flag() const;

  virtual void reverse_vertex_ordering();
  virtual void cleanup();

  void remove_doubled_verts(bool closed);
  void remove_nonunique_verts();


  // The EggPrimitive itself appears to be an STL container of
  // pointers to EggVertex objects.  The set of vertices is read-only,
  // however, except through the limited add_vertex/remove_vertex or
  // insert/erase interface.  The following implements this.

#ifdef WIN32_VC
  typedef PT(EggVertex) *pointer;
  typedef PT(EggVertex) *const_pointer;
#else
  typedef Vertices::const_pointer pointer;
  typedef Vertices::const_pointer const_pointer;
#endif
  typedef Vertices::const_reference reference;
  typedef Vertices::const_reference const_reference;
  typedef Vertices::const_iterator iterator;
  typedef Vertices::const_iterator const_iterator;
  typedef Vertices::const_reverse_iterator reverse_iterator;
  typedef Vertices::const_reverse_iterator const_reverse_iterator;
  typedef Vertices::size_type size_type;
  typedef Vertices::difference_type difference_type;

  INLINE iterator begin() const;
  INLINE iterator end() const;
  INLINE reverse_iterator rbegin() const;
  INLINE reverse_iterator rend() const;
  INLINE bool empty() const;
  INLINE size_type size() const;

  INLINE PT(EggVertex) operator [] (int index) const;

  INLINE iterator insert(iterator position, PT(EggVertex) x);
  INLINE iterator erase(iterator position);
  iterator erase(iterator first, iterator last);
  INLINE void replace(iterator position, PT(EggVertex) vertex);
  INLINE void clear();

  PT(EggVertex) add_vertex(PT(EggVertex) vertex);
  PT(EggVertex) remove_vertex(PT(EggVertex) vertex);
  void copy_vertices(const EggPrimitive &other);

  // These are shorthands if you don't want to use the iterators.
  INLINE void set_vertex(int index, PT(EggVertex) vertex);
  INLINE PT(EggVertex) get_vertex(int index) const;

  INLINE EggVertexPool *get_pool() const;

#ifndef NDEBUG
  void test_vref_integrity() const;
#else
  void test_vref_integrity() const { }
#endif  // NDEBUG

private:
  Vertices _vertices;

  // Don't try to use these private functions.  User code should add
  // and remove vertices via add_vertex()/remove_vertex(), or via the
  // STL-like push_back()/pop_back() or insert()/erase(), above.
  void prepare_add_vertex(EggVertex *vertex);
  void prepare_remove_vertex(EggVertex *vertex);

 
protected:
  void write_body(ostream &out, int indent_level) const;


private:
  PT(EggTexture) _texture;
  PT(EggMaterial) _material;
  bool _bface;

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggNode::init_type();
    EggAttributes::init_type();
    EggAlphaMode::get_class_type();
    register_type(_type_handle, "EggPrimitive",
                  EggNode::get_class_type(),
                  EggAttributes::get_class_type(),
		  EggAlphaMode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#include "eggPrimitive.I"

#endif
