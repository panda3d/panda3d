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
//       Class : NodeAttributeWrapper
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NodeAttributeWrapper {
public:
  typedef NodeTransitionWrapper TransitionWrapper;
  typedef NodeAttributeWrapper AttributeWrapper;

  INLINE_GRAPH NodeAttributeWrapper(TypeHandle handle);
  INLINE_GRAPH NodeAttributeWrapper(const NodeAttributeWrapper &copy);
  INLINE_GRAPH void operator = (const NodeAttributeWrapper &copy);
  static NodeAttributeWrapper init_from(const NodeTransitionWrapper &trans);

  INLINE_GRAPH TypeHandle get_handle() const;
  INLINE_GRAPH NodeAttribute *get_attrib() const;
  INLINE_GRAPH void set_attrib(NodeAttribute *attrib);

  INLINE_GRAPH bool is_initial() const;
  INLINE_GRAPH int compare_to(const NodeAttributeWrapper &other) const;

  INLINE_GRAPH void make_initial();
  void apply_in_place(const NodeTransitionWrapper &trans);

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  TypeHandle _handle;
  PT(NodeAttribute) _attrib;
};

INLINE_GRAPH ostream &operator << (ostream &out, const NodeAttributeWrapper &naw);

#include "nodeAttributeWrapper.T"

#ifndef DONT_INLINE_GRAPH
#include "nodeAttributeWrapper.I"
#endif

#endif
