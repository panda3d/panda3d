// Filename: nodeAttributeWrapper.h
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef NODEATTRIBUTEWRAPPER_H
#define NODEATTRIBUTEWRAPPER_H

#include <pandabase.h>

#include "nodeAttribute.h"

#include <updateSeq.h>
#include <pointerTo.h>

class NodeTransitionWrapper;

////////////////////////////////////////////////////////////////////
// 	 Class : NodeAttributeWrapper
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NodeAttributeWrapper {
public:
  typedef NodeTransitionWrapper TransitionWrapper;
  typedef NodeAttributeWrapper AttributeWrapper;

  INLINE NodeAttributeWrapper(TypeHandle handle);
  INLINE NodeAttributeWrapper(const NodeAttributeWrapper &copy);
  INLINE void operator = (const NodeAttributeWrapper &copy);
  static NodeAttributeWrapper init_from(const NodeTransitionWrapper &trans);

  INLINE TypeHandle get_handle() const;
  INLINE NodeAttribute *get_attrib() const;
  INLINE void set_attrib(NodeAttribute *attrib);

  INLINE bool is_initial() const;
  INLINE int compare_to(const NodeAttributeWrapper &other) const;

  INLINE void make_initial();
  void apply_in_place(const NodeTransitionWrapper &trans);

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  TypeHandle _handle;
  PT(NodeAttribute) _attrib;
};

INLINE ostream &operator << (ostream &out, const NodeAttributeWrapper &naw) {
  naw.output(out);
  return out;
}

#include "nodeAttributeWrapper.I"

template<class Attribute>
INLINE bool 
get_attribute_into(Attribute *&ptr, const NodeAttributeWrapper &trans);

#endif
