// Filename: allAttributesWrapper.h
// Created by:  drose (21Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef ALLATTRIBUTESWRAPPER_H
#define ALLATTRIBUTESWRAPPER_H

#include <pandabase.h>

#include "nodeAttributes.h"

#include <updateSeq.h>
#include <pointerTo.h>

class AllTransitionsWrapper;

////////////////////////////////////////////////////////////////////
// 	 Class : AllAttributesWrapper
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AllAttributesWrapper {
public:
  typedef AllTransitionsWrapper TransitionWrapper;
  typedef AllAttributesWrapper AttributeWrapper;

  INLINE_GRAPH AllAttributesWrapper();
  INLINE_GRAPH AllAttributesWrapper(const NodeAttributes &attrib);
  INLINE_GRAPH AllAttributesWrapper(const AllAttributesWrapper &copy);
  INLINE_GRAPH void operator = (const AllAttributesWrapper &copy);
  INLINE_GRAPH static AllAttributesWrapper 
  init_from(const AllTransitionsWrapper &trans);
  INLINE_GRAPH ~AllAttributesWrapper();

  INLINE_GRAPH bool is_empty() const;
  INLINE_GRAPH PT(NodeAttribute) set_attribute(TypeHandle handle,
					 NodeAttribute *trans);
  INLINE_GRAPH PT(NodeAttribute) set_attribute(NodeAttribute *trans);
  INLINE_GRAPH PT(NodeAttribute) clear_attribute(TypeHandle handle);
  INLINE_GRAPH bool has_attribute(TypeHandle handle) const;
  INLINE_GRAPH NodeAttribute *get_attribute(TypeHandle handle) const;

  INLINE_GRAPH const NodeAttributes &get_attributes() const;
  INLINE_GRAPH NodeAttributes &get_attributes();

  INLINE_GRAPH bool is_initial() const;
  INLINE_GRAPH int compare_to(const AllAttributesWrapper &other) const;

  INLINE_GRAPH void make_initial();
  void apply_in_place(const AllTransitionsWrapper &trans);
  void apply_from(const AllAttributesWrapper &other, 
		  const AllTransitionsWrapper &trans);
  NodeAttributes *apply(const AllTransitionsWrapper &trans) const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  NodeAttributes _attrib;
};

INLINE_GRAPH ostream &operator << (ostream &out, const AllAttributesWrapper &a);

#include "allAttributesWrapper.T"

#ifndef DONT_INLINE_GRAPH
#include "allAttributesWrapper.I"
#endif

#endif
