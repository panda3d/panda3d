// Filename: eggGroupNode.h
// Created by:  drose (16Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGGROUPNODE_H
#define EGGGROUPNODE_H

#include <pandabase.h>

#include "eggNode.h"

#include <coordinateSystem.h>
#include <typeHandle.h>
#include <pointerTo.h>
#include <list>

class EggTextureCollection;
class DSearchPath;

////////////////////////////////////////////////////////////////////
// 	 Class : EggGroupNode
// Description : A base class for nodes in the hierarchy that are not
//               leaf nodes.  (See also EggGroup, which is
//               specifically the "<Group>" node in egg.)
//
//               An EggGroupNode is an STL-style container of pointers
//               to EggNodes, like a vector.  Functions
//               push_back()/pop_back() and insert()/erase() are
//               provided to manipulate the list.  The list may also
//               be operated on (read-only) via iterators and
//               begin()/end().
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggGroupNode : public EggNode {

  // This is a bit of private interface stuff that must be here as a
  // forward reference.  This allows us to define the EggGroupNode as
  // an STL container.

private:
  // We define the list of children as a list and not a vector, so we
  // can avoid the bad iterator-invalidating properties of vectors as
  // we insert/delete elements.
  typedef list<PT(EggNode)> Children;

  // Here begins the actual public interface to EggGroupNode.

public:
  EggGroupNode(const string &name = "") : EggNode(name) { }
  EggGroupNode(const EggGroupNode &copy);
  EggGroupNode &operator = (const EggGroupNode &copy);

  virtual void write(ostream &out, int indent_level) const;

  // The EggGroupNode itself appears to be an STL container of
  // pointers to EggNodes.  The set of children is read-only, however,
  // except through the limited add_child/remove_child or insert/erase
  // interface.  The following implements this.

#ifdef WIN32_VC
  typedef const PT(EggNode) *pointer;
  typedef const PT(EggNode) *const_pointer;
#else
  typedef Children::const_pointer pointer;
  typedef Children::const_pointer const_pointer;
#endif
  typedef Children::const_reference reference;
  typedef Children::const_reference const_reference;
  typedef Children::const_iterator iterator;
  typedef Children::const_iterator const_iterator;
  typedef Children::const_reverse_iterator reverse_iterator;
  typedef Children::const_reverse_iterator const_reverse_iterator;
  typedef Children::size_type size_type;
  typedef Children::difference_type difference_type;

  INLINE iterator begin() const;
  INLINE iterator end() const;
  INLINE reverse_iterator rbegin() const;
  INLINE reverse_iterator rend() const;
  INLINE bool empty() const;
  INLINE size_type size() const;

  INLINE iterator insert(iterator position, PT(EggNode) x);
  INLINE iterator erase(iterator position);
  iterator erase(iterator first, iterator last);
  void replace(iterator position, PT(EggNode) x);
  INLINE void clear();

  PT(EggNode) add_child(PT(EggNode) node);
  PT(EggNode) remove_child(PT(EggNode) node);
  void steal_children(EggGroupNode &other);

  void resolve_filenames(const DSearchPath &searchpath);
  void reverse_vertex_ordering();

protected:
  virtual void update_under(int depth_offset);

  virtual void r_transform(const LMatrix4d &mat, const LMatrix4d &inv,
			   CoordinateSystem to_cs);
  virtual void r_mark_coordsys(CoordinateSystem cs);

  CoordinateSystem find_coordsys_entry();
  int find_textures(EggTextureCollection *collection);
  bool r_resolve_externals(const DSearchPath &searchpath,
			   CoordinateSystem coordsys);

private:
  Children _children;

  // Don't try to use these private functions.  User code should add
  // and remove children via add_child()/remove_child(), or via the
  // STL-like push_back()/pop_back() or insert()/erase(), above.
  void prepare_add_child(EggNode *node);
  void prepare_remove_child(EggNode *node);


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggNode::init_type();
    register_type(_type_handle, "EggGroupNode",
                  EggNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;

  friend class EggTextureCollection;
};

#include "eggGroupNode.I"

#endif

