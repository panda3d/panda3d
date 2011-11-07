// Filename: lightAttrib.cxx
// Created by:  drose (26Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "lightAttrib.h"
#include "pandaNode.h"
#include "nodePath.h"
#include "graphicsStateGuardianBase.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "config_pgraph.h"
#include "attribNodeRegistry.h"
#include "indent.h"
#include <iterator>

CPT(RenderAttrib) LightAttrib::_empty_attrib;
int LightAttrib::_attrib_slot;
CPT(RenderAttrib) LightAttrib::_all_off_attrib;
TypeHandle LightAttrib::_type_handle;

// This STL Function object is used in filter_to_max(), below, to sort
// a list of Lights in reverse order by priority.  In the case of two
// lights with equal priority, the class priority is compared.
class CompareLightPriorities {
public:
  bool operator ()(const NodePath &a, const NodePath &b) const {
    nassertr(!a.is_empty() && !b.is_empty(), a < b);
    Light *la = a.node()->as_light();
    Light *lb = b.node()->as_light();
    nassertr(la != (Light *)NULL && lb != (Light *)NULL, a < b);
             
    if (la->get_priority() != lb->get_priority()) {
      return la->get_priority() > lb->get_priority();
    }
    return la->get_class_priority() > lb->get_class_priority();
  }
};

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::make
//       Access: Published, Static
//  Description: Constructs a new LightAttrib object that turns on (or
//               off, according to op) the indicated light(s).
//
//               This method is now deprecated.  Use add_on_light() or
//               add_off_light() instead.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightAttrib::
make(LightAttrib::Operation op, Light *light) {
  pgraph_cat.warning()
    << "Using deprecated LightAttrib interface.\n";

  CPT(RenderAttrib) attrib;

  switch (op) {
  case O_set:
    attrib = make_all_off();
    attrib = DCAST(LightAttrib, attrib)->add_on_light(NodePath(light->as_node()));
    return attrib;
   
  case O_add:
    attrib = make();
    attrib = DCAST(LightAttrib, attrib)->add_on_light(NodePath(light->as_node()));
    return attrib;

  case O_remove:
    attrib = make();
    attrib = DCAST(LightAttrib, attrib)->add_off_light(NodePath(light->as_node()));
    return attrib;
  }

  nassertr(false, make());
  return make();
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::make
//       Access: Published, Static
//  Description: Constructs a new LightAttrib object that turns on (or
//               off, according to op) the indicate light(s).
//
//               This method is now deprecated.  Use add_on_light() or
//               add_off_light() instead.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightAttrib::
make(LightAttrib::Operation op, Light *light1, Light *light2) {
  pgraph_cat.warning()
    << "Using deprecated LightAttrib interface.\n";

  CPT(RenderAttrib) attrib;

  switch (op) {
  case O_set:
    attrib = make_all_off();
    attrib = DCAST(LightAttrib, attrib)->add_on_light(NodePath(light1->as_node()));
    attrib = DCAST(LightAttrib, attrib)->add_on_light(NodePath(light2->as_node()));
    return attrib;
   
  case O_add:
    attrib = make();
    attrib = DCAST(LightAttrib, attrib)->add_on_light(NodePath(light1->as_node()));
    attrib = DCAST(LightAttrib, attrib)->add_on_light(NodePath(light2->as_node()));
    return attrib;

  case O_remove:
    attrib = make();
    attrib = DCAST(LightAttrib, attrib)->add_off_light(NodePath(light1->as_node()));
    attrib = DCAST(LightAttrib, attrib)->add_off_light(NodePath(light2->as_node()));
    return attrib;
  }

  nassertr(false, make());
  return make();
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::make
//       Access: Published, Static
//  Description: Constructs a new LightAttrib object that turns on (or
//               off, according to op) the indicate light(s).
//
//               This method is now deprecated.  Use add_on_light() or
//               add_off_light() instead.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightAttrib::
make(LightAttrib::Operation op, Light *light1, Light *light2,
     Light *light3) { 
  pgraph_cat.warning()
    << "Using deprecated LightAttrib interface.\n";

  CPT(RenderAttrib) attrib;

  switch (op) {
  case O_set:
    attrib = make_all_off();
    attrib = DCAST(LightAttrib, attrib)->add_on_light(NodePath(light1->as_node()));
    attrib = DCAST(LightAttrib, attrib)->add_on_light(NodePath(light2->as_node()));
    attrib = DCAST(LightAttrib, attrib)->add_on_light(NodePath(light3->as_node()));
    return attrib;
   
  case O_add:
    attrib = make();
    attrib = DCAST(LightAttrib, attrib)->add_on_light(NodePath(light1->as_node()));
    attrib = DCAST(LightAttrib, attrib)->add_on_light(NodePath(light2->as_node()));
    attrib = DCAST(LightAttrib, attrib)->add_on_light(NodePath(light3->as_node()));
    return attrib;

  case O_remove:
    attrib = make();
    attrib = DCAST(LightAttrib, attrib)->add_off_light(NodePath(light1->as_node()));
    attrib = DCAST(LightAttrib, attrib)->add_off_light(NodePath(light2->as_node()));
    attrib = DCAST(LightAttrib, attrib)->add_off_light(NodePath(light3->as_node()));
    return attrib;
  }

  nassertr(false, make());
  return make();
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::make
//       Access: Published, Static
//  Description: Constructs a new LightAttrib object that turns on (or
//               off, according to op) the indicate light(s).
//
//               This method is now deprecated.  Use add_on_light() or
//               add_off_light() instead.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightAttrib::
make(LightAttrib::Operation op, Light *light1, Light *light2,
     Light *light3, Light *light4) {
  pgraph_cat.warning()
    << "Using deprecated LightAttrib interface.\n";

  CPT(RenderAttrib) attrib;

  switch (op) {
  case O_set:
    attrib = make_all_off();
    attrib = DCAST(LightAttrib, attrib)->add_on_light(NodePath(light1->as_node()));
    attrib = DCAST(LightAttrib, attrib)->add_on_light(NodePath(light2->as_node()));
    attrib = DCAST(LightAttrib, attrib)->add_on_light(NodePath(light3->as_node()));
    attrib = DCAST(LightAttrib, attrib)->add_on_light(NodePath(light4->as_node()));
    return attrib;
   
  case O_add:
    attrib = make();
    attrib = DCAST(LightAttrib, attrib)->add_on_light(NodePath(light1->as_node()));
    attrib = DCAST(LightAttrib, attrib)->add_on_light(NodePath(light2->as_node()));
    attrib = DCAST(LightAttrib, attrib)->add_on_light(NodePath(light3->as_node()));
    attrib = DCAST(LightAttrib, attrib)->add_on_light(NodePath(light4->as_node()));
    return attrib;

  case O_remove:
    attrib = make();
    attrib = DCAST(LightAttrib, attrib)->add_off_light(NodePath(light1->as_node()));
    attrib = DCAST(LightAttrib, attrib)->add_off_light(NodePath(light2->as_node()));
    attrib = DCAST(LightAttrib, attrib)->add_off_light(NodePath(light3->as_node()));
    attrib = DCAST(LightAttrib, attrib)->add_off_light(NodePath(light4->as_node()));
    return attrib;
  }

  nassertr(false, make());
  return make();
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::make_default
//       Access: Published, Static
//  Description: Returns a RenderAttrib that corresponds to whatever
//               the standard default properties for render attributes
//               of this type ought to be.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightAttrib::
make_default() {
  return return_new(new LightAttrib);
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::get_operation
//       Access: Published
//  Description: Returns the basic operation type of the LightAttrib.
//               If this is O_set, the lights listed here completely
//               replace any lights that were already on.  If this is
//               O_add, the lights here are added to the set of of
//               lights that were already on, and if O_remove, the
//               lights here are removed from the set of lights that
//               were on.
//
//               This method is now deprecated.  LightAttribs nowadays
//               have a separate list of on_lights and off_lights, so
//               this method doesn't make sense.  Query the lists
//               independently.
////////////////////////////////////////////////////////////////////
LightAttrib::Operation LightAttrib::
get_operation() const {
  pgraph_cat.warning()
    << "Using deprecated LightAttrib interface.\n";

  if (has_all_off()) {
    return O_set;

  } else if (get_num_off_lights() == 0) {
    return O_add;

  } else {
    return O_remove;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::get_num_lights
//       Access: Published
//  Description: Returns the number of lights listed in the attribute.
//
//               This method is now deprecated.  LightAttribs nowadays
//               have a separate list of on_lights and off_lights, so
//               this method doesn't make sense.  Query the lists
//               independently.
////////////////////////////////////////////////////////////////////
int LightAttrib::
get_num_lights() const {
  pgraph_cat.warning()
    << "Using deprecated LightAttrib interface.\n";

  if (get_num_off_lights() == 0) {
    return get_num_on_lights();
  } else {
    return get_num_off_lights();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::get_light
//       Access: Published
//  Description: Returns the nth light listed in the attribute.
//
//               This method is now deprecated.  LightAttribs nowadays
//               have a separate list of on_lights and off_lights, so
//               this method doesn't make sense.  Query the lists
//               independently.
////////////////////////////////////////////////////////////////////
Light *LightAttrib::
get_light(int n) const {
  pgraph_cat.warning()
    << "Using deprecated LightAttrib interface.\n";

  if (get_num_off_lights() == 0) {
    return get_on_light(n).node()->as_light();
  } else {
    return get_off_light(n).node()->as_light();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::has_light
//       Access: Published
//  Description: Returns true if the indicated light is listed in the
//               attrib, false otherwise.
//
//               This method is now deprecated.  LightAttribs nowadays
//               have a separate list of on_lights and off_lights, so
//               this method doesn't make sense.  Query the lists
//               independently.
////////////////////////////////////////////////////////////////////
bool LightAttrib::
has_light(Light *light) const {
  pgraph_cat.warning()
    << "Using deprecated LightAttrib interface.\n";

  if (get_num_off_lights() == 0) {
    return has_on_light(NodePath(light->as_node()));
  } else {
    return has_off_light(NodePath(light->as_node()));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::add_light
//       Access: Published
//  Description: Returns a new LightAttrib, just like this one, but
//               with the indicated light added to the list of lights.
//
//               This method is now deprecated.  Use add_on_light() or
//               add_off_light() instead.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightAttrib::
add_light(Light *light) const {
  pgraph_cat.warning()
    << "Using deprecated LightAttrib interface.\n";

  if (get_num_off_lights() == 0) {
    return add_on_light(NodePath(light->as_node()));
  } else {
    return add_off_light(NodePath(light->as_node()));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::remove_light
//       Access: Published
//  Description: Returns a new LightAttrib, just like this one, but
//               with the indicated light removed from the list of
//               lights.
//
//               This method is now deprecated.  Use remove_on_light()
//               or remove_off_light() instead.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightAttrib::
remove_light(Light *light) const {
  pgraph_cat.warning()
    << "Using deprecated LightAttrib interface.\n";

  if (get_num_off_lights() == 0) {
    return remove_on_light(NodePath(light->as_node()));
  } else {
    return remove_off_light(NodePath(light->as_node()));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::make
//       Access: Published, Static
//  Description: Constructs a new LightAttrib object that does
//               nothing.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightAttrib::
make() {
  // We make it a special case and store a pointer to the empty attrib
  // forever once we find it the first time, as an optimization.
  if (_empty_attrib == (RenderAttrib *)NULL) {
    _empty_attrib = return_new(new LightAttrib);
  }

  return _empty_attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::make_all_off
//       Access: Published, Static
//  Description: Constructs a new LightAttrib object that turns off
//               all lights (and hence disables lighting).
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightAttrib::
make_all_off() {
  // We make it a special case and store a pointer to the off attrib
  // forever once we find it the first time, as an optimization.
  if (_all_off_attrib == (RenderAttrib *)NULL) {
    LightAttrib *attrib = new LightAttrib;
    attrib->_off_all_lights = true;
    _all_off_attrib = return_new(attrib);
  }

  return _all_off_attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::add_on_light
//       Access: Published
//  Description: Returns a new LightAttrib, just like this one, but
//               with the indicated light added to the list of lights
//               turned on by this attrib.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightAttrib::
add_on_light(const NodePath &light) const {
  nassertr(!light.is_empty() && light.node()->as_light() != (Light *)NULL, this);
  LightAttrib *attrib = new LightAttrib(*this);
  attrib->_on_lights.insert(light);
  attrib->_off_lights.erase(light);

  pair<Lights::iterator, bool> insert_result = 
    attrib->_on_lights.insert(Lights::value_type(light));
  if (insert_result.second) {
    // Also ensure it is removed from the off_lights list.
    attrib->_off_lights.erase(light);
  }

  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::remove_on_light
//       Access: Published
//  Description: Returns a new LightAttrib, just like this one, but
//               with the indicated light removed from the list of
//               lights turned on by this attrib.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightAttrib::
remove_on_light(const NodePath &light) const {
  nassertr(!light.is_empty() && light.node()->as_light() != (Light *)NULL, this);
  LightAttrib *attrib = new LightAttrib(*this);
  attrib->_on_lights.erase(light);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::add_off_light
//       Access: Published
//  Description: Returns a new LightAttrib, just like this one, but
//               with the indicated light added to the list of lights
//               turned off by this attrib.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightAttrib::
add_off_light(const NodePath &light) const {
  nassertr(!light.is_empty() && light.node()->as_light() != (Light *)NULL, this);
  LightAttrib *attrib = new LightAttrib(*this);
  if (!_off_all_lights) {
    attrib->_off_lights.insert(light);
  }
  attrib->_on_lights.erase(light);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::remove_off_light
//       Access: Published
//  Description: Returns a new LightAttrib, just like this one, but
//               with the indicated light removed from the list of
//               lights turned off by this attrib.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightAttrib::
remove_off_light(const NodePath &light) const {
  nassertr(!light.is_empty() && light.node()->as_light() != (Light *)NULL, this);
  LightAttrib *attrib = new LightAttrib(*this);
  attrib->_off_lights.erase(light);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::filter_to_max
//       Access: Public
//  Description: Returns a new LightAttrib, very much like this one,
//               but with the number of on_lights reduced to be no
//               more than max_lights.  The number of off_lights in
//               the new LightAttrib is undefined.
//
//               The number of AmbientLights is not included in the
//               count.  All AmbientLights in the original attrib are
//               always included in the result, regardless of the
//               value of max_lights.
////////////////////////////////////////////////////////////////////
CPT(LightAttrib) LightAttrib::
filter_to_max(int max_lights) const {
  if (max_lights < 0 || (int)_on_lights.size() <= max_lights) {
    // Trivial case: this LightAttrib qualifies.
    return this;
  }

  // Since check_filtered() will clear the _filtered list if we are out
  // of date, we should call it first.
  check_filtered();

  Filtered::const_iterator fi;
  fi = _filtered.find(max_lights);
  if (fi != _filtered.end()) {
    // Easy case: we have already computed this for this particular
    // LightAttrib.
    return (*fi).second;
  }

  // Harder case: we have to compute it now.  We must choose the n
  // lights with the highest priority in our list of lights.
  Lights priority_lights, ambient_lights;

  // Separate the list of lights into ambient lights and other lights.
  Lights::const_iterator li;
  for (li = _on_lights.begin(); li != _on_lights.end(); ++li) {
    const NodePath &np = (*li);
    nassertr(!np.is_empty() && np.node()->as_light() != (Light *)NULL, this);
    if (np.node()->is_ambient_light()) {
      ambient_lights.push_back(np);
    } else {
      priority_lights.push_back(np);
    }
  }

  // This sort function uses the STL function object defined above.
  sort(priority_lights.begin(), priority_lights.end(), 
       CompareLightPriorities());

  // Now lop off all of the lights after the first max_lights.
  if ((int)priority_lights.size() > max_lights) { 
    priority_lights.erase(priority_lights.begin() + max_lights,
                          priority_lights.end());
  }

  // Put the ambient lights back into the list.
  for (li = ambient_lights.begin(); li != ambient_lights.end(); ++li) {
    priority_lights.push_back(*li);
  }

  // And re-sort the ov_set into its proper order.
  priority_lights.sort();

  // Now create a new attrib reflecting these lights.
  PT(LightAttrib) attrib = new LightAttrib;
  attrib->_on_lights.swap(priority_lights);

  CPT(RenderAttrib) new_attrib = return_new(attrib);

  // Finally, record this newly-created attrib in the map for next
  // time.
  CPT(LightAttrib) light_attrib = (const LightAttrib *)new_attrib.p();
  ((LightAttrib *)this)->_filtered[max_lights] = light_attrib;
  return light_attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::get_most_important_light
//       Access: Public
//  Description: Returns the most important light (that is, the light
//               with the highest priority) in the LightAttrib,
//               excluding any ambient lights.  Returns an empty
//               NodePath if no non-ambient lights are found.
////////////////////////////////////////////////////////////////////
NodePath LightAttrib::
get_most_important_light() const {
  NodePath best;

  CompareLightPriorities compare;

  Lights::const_iterator li;
  for (li = _on_lights.begin(); li != _on_lights.end(); ++li) {
    const NodePath &np = (*li);
    nassertr(!np.is_empty() && np.node()->as_light() != (Light *)NULL, NodePath());
    if (!np.node()->is_ambient_light()) {
      if (best.is_empty() || compare(np, best)) {
        best = np;
      }
    }
  }

  return best;
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void LightAttrib::
output(ostream &out) const {
  out << get_type() << ":";
  if (_off_lights.empty()) {
    if (_on_lights.empty()) {
      if (_off_all_lights) {
        out << "all off";
      } else {
        out << "identity";
      }
    } else {
      if (_off_all_lights) {
        out << "set";
      } else {
        out << "on";
      }
    }

  } else {
    out << "off";
    Lights::const_iterator fi;
    for (fi = _off_lights.begin(); fi != _off_lights.end(); ++fi) {
      NodePath light = (*fi);
      if (light.is_empty()) {
        out << " " << light;
      } else {
        out << " " << light.get_name();
      }        
    }

    if (!_on_lights.empty()) {
      out << " on";
    }
  }
    
  Lights::const_iterator li;
  for (li = _on_lights.begin(); li != _on_lights.end(); ++li) {
    NodePath light = (*li);
    if (light.is_empty()) {
      out << " " << light;
    } else {
      out << " " << light.get_name();
    }        
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void LightAttrib::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << get_type() << ":";
  if (_off_lights.empty()) {
    if (_on_lights.empty()) {
      if (_off_all_lights) {
        out << "all off\n";
      } else {
        out << "identity\n";
      }
    } else {
      if (_off_all_lights) {
        out << "set\n";
      } else {
        out << "on\n";
      }
    }

  } else {
    out << "off\n";
    Lights::const_iterator fi;
    for (fi = _off_lights.begin(); fi != _off_lights.end(); ++fi) {
      NodePath light = (*fi);
      indent(out, indent_level + 2) << light << "\n";
    }

    if (!_on_lights.empty()) {
      indent(out, indent_level) << "on\n";
    }
  }
    
  Lights::const_iterator li;
  for (li = _on_lights.begin(); li != _on_lights.end(); ++li) {
    NodePath light = (*li);
    indent(out, indent_level + 2) << light << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived LightAttrib
//               types to return a unique number indicating whether
//               this LightAttrib is equivalent to the other one.
//
//               This should return 0 if the two LightAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two LightAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int LightAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const LightAttrib *ta;
  DCAST_INTO_R(ta, other, 0);

  if (_off_all_lights != ta->_off_all_lights) {
    return (int)_off_all_lights - (int)ta->_off_all_lights;
  }

  Lights::const_iterator li = _on_lights.begin();
  Lights::const_iterator oli = ta->_on_lights.begin();

  while (li != _on_lights.end() && oli != ta->_on_lights.end()) {
    NodePath light = (*li);
    NodePath other_light = (*oli);

    int compare = light.compare_to(other_light);
    if (compare != 0) {
      return compare;
    }

    ++li;
    ++oli;
  }

  if (li != _on_lights.end()) {
    return 1;
  }
  if (oli != ta->_on_lights.end()) {
    return -1;
  }

  Lights::const_iterator fi = _off_lights.begin();
  Lights::const_iterator ofi = ta->_off_lights.begin();

  while (fi != _off_lights.end() && ofi != ta->_off_lights.end()) {
    NodePath light = (*fi);
    NodePath other_light = (*ofi);

    int compare = light.compare_to(other_light);
    if (compare != 0) {
      return compare;
    }

    ++fi;
    ++ofi;
  }

  if (fi != _off_lights.end()) {
    return 1;
  }
  if (ofi != ta->_off_lights.end()) {
    return -1;
  }
  
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::get_hash_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to return a unique hash for these particular
//               properties.  RenderAttribs that compare the same with
//               compare_to_impl(), above, should return the same
//               hash; RenderAttribs that compare differently should
//               return a different hash.
////////////////////////////////////////////////////////////////////
size_t LightAttrib::
get_hash_impl() const {
  size_t hash = 0;

  Lights::const_iterator li;
  for (li = _on_lights.begin(); li != _on_lights.end(); ++li) {
    NodePath light = (*li);
    hash = light.add_hash(hash);
  }

  // This bool value goes here, between the two lists, to
  // differentiate between the two.
  hash = int_hash::add_hash(hash, (int)_off_all_lights);

  for (li = _off_lights.begin(); li != _off_lights.end(); ++li) {
    NodePath light = (*li);
    hash = light.add_hash(hash);
  }

  return hash;
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::compose_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to specify how two consecutive RenderAttrib
//               objects of the same type interact.
//
//               This should return the result of applying the other
//               RenderAttrib to a node in the scene graph below this
//               RenderAttrib, which was already applied.  In most
//               cases, the result is the same as the other
//               RenderAttrib (that is, a subsequent RenderAttrib
//               completely replaces the preceding one).  On the other
//               hand, some kinds of RenderAttrib (for instance,
//               ColorTransformAttrib) might combine in meaningful
//               ways.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightAttrib::
compose_impl(const RenderAttrib *other) const {
  const LightAttrib *ta;
  DCAST_INTO_R(ta, other, 0);

  if (ta->_off_all_lights) {
    // If the other type turns off all lights, it doesn't matter what
    // we are.
    return ta;
  }

  // This is a three-way merge between ai, bi, and ci, except that bi
  // and ci should have no intersection and therefore needn't be
  // compared to each other.
  Lights::const_iterator ai = _on_lights.begin();
  Lights::const_iterator bi = ta->_on_lights.begin();
  Lights::const_iterator ci = ta->_off_lights.begin();

  // Create a new LightAttrib that will hold the result.
  LightAttrib *new_attrib = new LightAttrib;
  back_insert_iterator<Lights> result = 
    back_inserter(new_attrib->_on_lights);

  while (ai != _on_lights.end() && 
         bi != ta->_on_lights.end() && 
         ci != ta->_off_lights.end()) {
    if ((*ai) < (*bi)) {
      if ((*ai) < (*ci)) {
        // Here is a light that we have in the original, which is not
        // present in the secondary.
        *result = *ai;
        ++ai;
        ++result;

      } else if ((*ci) < (*ai)) {
        // Here is a light that is turned off in the secondary, but
        // was not present in the original.
        ++ci;

      } else { // (*ci) == (*ai)
        // Here is a light that is turned off in the secondary, and
        // was present in the original.
        ++ai;
        ++ci;
      }

    } else if ((*bi) < (*ai)) {
      // Here is a new light we have in the secondary, that was not
      // present in the original.
      *result = *bi;
      ++bi;
      ++result;

    } else {  // (*bi) == (*ai)
      // Here is a light we have in both.
      *result = *bi;
      ++ai;
      ++bi;
      ++result;
    }
  }

  while (ai != _on_lights.end() && bi != ta->_on_lights.end()) {
    if ((*ai) < (*bi)) {
      // Here is a light that we have in the original, which is not
      // present in the secondary.
      *result = *ai;
      ++ai;
      ++result;

    } else if ((*bi) < (*ai)) {
      // Here is a new light we have in the secondary, that was not
      // present in the original.
      *result = *bi;
      ++bi;
      ++result;

    } else {
      // Here is a light we have in both.
      *result = *bi;
      ++ai;
      ++bi;
      ++result;
    }
  }

  while (ai != _on_lights.end() && ci != ta->_off_lights.end()) {
    if ((*ai) < (*ci)) {
      // Here is a light that we have in the original, which is not
      // present in the secondary.
      *result = *ai;
      ++ai;
      ++result;
      
    } else if ((*ci) < (*ai)) {
      // Here is a light that is turned off in the secondary, but
      // was not present in the original.
      ++ci;
      
    } else { // (*ci) == (*ai)
      // Here is a light that is turned off in the secondary, and
      // was present in the original.
      ++ai;
      ++ci;
    }
  }

  while (ai != _on_lights.end()) {
    *result = *ai;
    ++ai;
    ++result;
  }

  while (bi != ta->_on_lights.end()) {
    *result = *bi;
    ++bi;
    ++result;
  }

  return return_new(new_attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::invert_compose_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to specify how two consecutive RenderAttrib
//               objects of the same type interact.
//
//               See invert_compose() and compose_impl().
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightAttrib::
invert_compose_impl(const RenderAttrib *other) const {
  // I think in this case the other attrib always wins.  Maybe this
  // needs a bit more thought.  It's hard to imagine that it's even
  // important to compute this properly.
  return other;
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::get_auto_shader_attrib_impl
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightAttrib::
get_auto_shader_attrib_impl(const RenderState *state) const {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::sort_on_lights
//       Access: Private
//  Description: This is patterned after
//               TextureAttrib::sort_on_stages(), but since lights
//               don't actually require sorting, this only empties the
//               _filtered map.
////////////////////////////////////////////////////////////////////
void LightAttrib::
sort_on_lights() {
  _sort_seq = Light::get_sort_seq();
  _filtered.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               LightAttrib.
////////////////////////////////////////////////////////////////////
void LightAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void LightAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_bool(_off_all_lights);

  // write the number of off_lights
  dg.add_uint16(get_num_off_lights());
  // write the off lights pointers if any
  Lights::const_iterator fi;
  for (fi = _off_lights.begin(); fi != _off_lights.end(); ++fi) {
    NodePath light = (*fi);

    // Since we can't write out a NodePath, we write out just the
    // plain PandaNode.  The user can use the AttribNodeRegistry on
    // re-read if there is any ambiguity that needs to be resolved.
    manager->write_pointer(dg, light.node());
  }

  // write the number of on lights
  dg.add_uint16(get_num_on_lights());
  // write the on lights pointers if any
  Lights::const_iterator nti;
  for (nti = _on_lights.begin(); nti != _on_lights.end(); ++nti) {
    NodePath light = (*nti);
    manager->write_pointer(dg, light.node());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int LightAttrib::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderAttrib::complete_pointers(p_list, manager);

  BamAuxData *aux = (BamAuxData *)manager->get_aux_data(this, "lights");
  nassertr(aux != NULL, pi);

  int i;
  aux->_off_list.reserve(aux->_num_off_lights);
  for (i = 0; i < aux->_num_off_lights; ++i) {
    PandaNode *node;
    DCAST_INTO_R(node, p_list[pi++], pi);
    aux->_off_list.push_back(node);
  }

  aux->_on_list.reserve(aux->_num_on_lights);
  for (i = 0; i < aux->_num_on_lights; ++i) {
    PandaNode *node;
    DCAST_INTO_R(node, p_list[pi++], pi);
    aux->_on_list.push_back(node);
  }

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::finalize
//       Access: Public, Virtual
//  Description: Called by the BamReader to perform any final actions
//               needed for setting up the object after all objects
//               have been read and all pointers have been completed.
////////////////////////////////////////////////////////////////////
void LightAttrib::
finalize(BamReader *manager) {
  // Now it's safe to convert our saved PandaNodes into NodePaths.
  BamAuxData *aux = (BamAuxData *)manager->get_aux_data(this, "lights");
  nassertv(aux != NULL);
  nassertv(aux->_num_off_lights == (int)aux->_off_list.size());
  nassertv(aux->_num_on_lights == (int)aux->_on_list.size());

  AttribNodeRegistry *areg = AttribNodeRegistry::get_global_ptr();

  _off_lights.reserve(aux->_off_list.size());
  NodeList::iterator ni;
  for (ni = aux->_off_list.begin(); ni != aux->_off_list.end(); ++ni) {
    PandaNode *node = (*ni);
    int n = areg->find_node(node->get_type(), node->get_name());
    if (n != -1) {
      // If it's in the registry, add that NodePath.
      _off_lights.push_back(areg->get_node(n));
    } else {
      // Otherwise, add any arbitrary NodePath.  Complain if it's
      // ambiguous.
      _off_lights.push_back(NodePath(node));
    }
  }
  _off_lights.sort();

  _on_lights.reserve(aux->_on_list.size());
  for (ni = aux->_on_list.begin(); ni != aux->_on_list.end(); ++ni) {
    PandaNode *node = (*ni);
    int n = areg->find_node(node->get_type(), node->get_name());
    if (n != -1) {
      // If it's in the registry, add that NodePath.
      _on_lights.push_back(areg->get_node(n));
    } else {
      // Otherwise, add any arbitrary NodePath.  Complain if it's
      // ambiguous.
      _on_lights.push_back(NodePath(node));
    }
  }
  _on_lights.sort();
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type LightAttrib is encountered
//               in the Bam file.  It should create the LightAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *LightAttrib::
make_from_bam(const FactoryParams &params) {
  LightAttrib *attrib = new LightAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  manager->register_finalize(attrib);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new LightAttrib.
////////////////////////////////////////////////////////////////////
void LightAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _off_all_lights = scan.get_bool();

  BamAuxData *aux = new BamAuxData;
  manager->set_aux_data(this, "lights", aux);

  aux->_num_off_lights = scan.get_uint16();
  manager->read_pointers(scan, aux->_num_off_lights);
    
  aux->_num_on_lights = scan.get_uint16();
  manager->read_pointers(scan, aux->_num_on_lights);
}
