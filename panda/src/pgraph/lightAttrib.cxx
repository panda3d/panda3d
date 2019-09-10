/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightAttrib.cxx
 * @author drose
 * @date 2002-03-26
 */

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

// This STL Function object is used in sort_on_lights(), below, to sort a list
// of Lights in reverse order by priority.  In the case of two lights with
// equal priority, the class priority is compared.
class CompareLightPriorities {
public:
  bool operator ()(const NodePath &a, const NodePath &b) const {
    nassertr(!a.is_empty() && !b.is_empty(), a < b);
    Light *la = a.node()->as_light();
    Light *lb = b.node()->as_light();
    nassertr(la != nullptr && lb != nullptr, a < b);

    if (la->get_priority() != lb->get_priority()) {
      return la->get_priority() > lb->get_priority();
    }
    return la->get_class_priority() > lb->get_class_priority();
  }
};

/**
 * Use LightAttrib::make() to construct a new LightAttrib object.  The copy
 * constructor is only defined to facilitate methods like add_on_light().
 */
LightAttrib::
LightAttrib(const LightAttrib &copy) :
  _on_lights(copy._on_lights),
  _off_lights(copy._off_lights),
  _off_all_lights(copy._off_all_lights),
  _sort_seq(UpdateSeq::old())
{
  // Increase the attrib_ref of all the lights in this attribute.
  Lights::const_iterator it;
  for (it = _on_lights.begin(); it != _on_lights.end(); ++it) {
    Light *lobj = (*it).node()->as_light();
    nassertd(lobj != nullptr) continue;
    lobj->attrib_ref();
  }
}

/**
 * Destructor.
 */
LightAttrib::
~LightAttrib() {
  // Call attrib_unref() on all on lights.
  Lights::const_iterator it;
  for (it = _on_lights.begin(); it != _on_lights.end(); ++it) {
    const NodePath &np = *it;
    if (!np.is_empty()) {
      Light *lobj = np.node()->as_light();
      if (lobj != nullptr) {
        lobj->attrib_unref();
      }
    }
  }
}

/**
 * Constructs a new LightAttrib object that turns on (or off, according to op)
 * the indicated light(s).
 *
 * @deprecated Use add_on_light() or add_off_light() instead.
 */
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

  nassert_raise("invalid operation");
  return make();
}

/**
 * Constructs a new LightAttrib object that turns on (or off, according to op)
 * the indicate light(s).
 *
 * @deprecated Use add_on_light() or add_off_light() instead.
 */
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

  nassert_raise("invalid operation");
  return make();
}

/**
 * Constructs a new LightAttrib object that turns on (or off, according to op)
 * the indicate light(s).
 *
 * @deprecated Use add_on_light() or add_off_light() instead.
 */
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

  nassert_raise("invalid operation");
  return make();
}

/**
 * Constructs a new LightAttrib object that turns on (or off, according to op)
 * the indicate light(s).
 *
 * @deprecated Use add_on_light() or add_off_light() instead.
 */
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

  nassert_raise("invalid operation");
  return make();
}

/**
 * Returns a RenderAttrib that corresponds to whatever the standard default
 * properties for render attributes of this type ought to be.
 */
CPT(RenderAttrib) LightAttrib::
make_default() {
  return return_new(new LightAttrib);
}

/**
 * Returns the basic operation type of the LightAttrib.  If this is O_set, the
 * lights listed here completely replace any lights that were already on.  If
 * this is O_add, the lights here are added to the set of lights that were
 * already on, and if O_remove, the lights here are removed from the set of
 * lights that were on.
 *
 * @deprecated LightAttribs nowadays have a separate list of on_lights and
 * off_lights, so this method no longer makes sense.  Query the lists
 * independently.
 */
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

/**
 * Returns the number of lights listed in the attribute.
 *
 * @deprecated LightAttribs nowadays have a separate list of on_lights and
 * off_lights, so this method no longer makes sense.  Query the lists
 * independently.
 */
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

/**
 * Returns the nth light listed in the attribute.
 *
 * @deprecated LightAttribs nowadays have a separate list of on_lights and
 * off_lights, so this method no longer makes sense.  Query the lists
 * independently.
 */
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

/**
 * Returns true if the indicated light is listed in the attrib, false
 * otherwise.
 *
 * @deprecated LightAttribs nowadays have a separate list of on_lights and
 * off_lights, so this method no longer makes sense.  Query the lists
 * independently.
 */
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

/**
 * Returns a new LightAttrib, just like this one, but with the indicated light
 * added to the list of lights.
 *
 * @deprecated Use add_on_light() or add_off_light() instead.
 */
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

/**
 * Returns a new LightAttrib, just like this one, but with the indicated light
 * removed from the list of lights.
 *
 * @deprecated Use remove_on_light() or remove_off_light() instead.
 */
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

/**
 * Constructs a new LightAttrib object that does nothing.
 */
CPT(RenderAttrib) LightAttrib::
make() {
  // We make it a special case and store a pointer to the empty attrib forever
  // once we find it the first time, as an optimization.
  if (_empty_attrib == nullptr) {
    _empty_attrib = return_new(new LightAttrib);
  }

  return _empty_attrib;
}

/**
 * Constructs a new LightAttrib object that turns off all lights (and hence
 * disables lighting).
 */
CPT(RenderAttrib) LightAttrib::
make_all_off() {
  // We make it a special case and store a pointer to the off attrib forever
  // once we find it the first time, as an optimization.
  if (_all_off_attrib == nullptr) {
    LightAttrib *attrib = new LightAttrib;
    attrib->_off_all_lights = true;
    _all_off_attrib = return_new(attrib);
  }

  return _all_off_attrib;
}

/**
 * Returns a new LightAttrib, just like this one, but with the indicated light
 * added to the list of lights turned on by this attrib.
 */
CPT(RenderAttrib) LightAttrib::
add_on_light(const NodePath &light) const {
  nassertr(!light.is_empty(), this);
  Light *lobj = light.node()->as_light();
  nassertr(lobj != nullptr, this);

  LightAttrib *attrib = new LightAttrib(*this);

  std::pair<Lights::iterator, bool> insert_result =
    attrib->_on_lights.insert(Lights::value_type(light));
  if (insert_result.second) {
    lobj->attrib_ref();

    // Also ensure it is removed from the off_lights list.
    attrib->_off_lights.erase(light);
  }

  return return_new(attrib);
}

/**
 * Returns a new LightAttrib, just like this one, but with the indicated light
 * removed from the list of lights turned on by this attrib.
 */
CPT(RenderAttrib) LightAttrib::
remove_on_light(const NodePath &light) const {
  nassertr(!light.is_empty(), this);
  Light *lobj = light.node()->as_light();
  nassertr(lobj != nullptr, this);

  LightAttrib *attrib = new LightAttrib(*this);
  if (attrib->_on_lights.erase(light)) {
    lobj->attrib_unref();
  }
  return return_new(attrib);
}

/**
 * Returns a new LightAttrib, just like this one, but with the indicated light
 * added to the list of lights turned off by this attrib.
 */
CPT(RenderAttrib) LightAttrib::
add_off_light(const NodePath &light) const {
  nassertr(!light.is_empty(), this);
  Light *lobj = light.node()->as_light();
  nassertr(lobj != nullptr, this);

  LightAttrib *attrib = new LightAttrib(*this);
  if (!_off_all_lights) {
    attrib->_off_lights.insert(light);
  }
  if (attrib->_on_lights.erase(light)) {
    lobj->attrib_unref();
  }
  return return_new(attrib);
}

/**
 * Returns a new LightAttrib, just like this one, but with the indicated light
 * removed from the list of lights turned off by this attrib.
 */
CPT(RenderAttrib) LightAttrib::
remove_off_light(const NodePath &light) const {
  nassertr(!light.is_empty() && light.node()->as_light() != nullptr, this);
  LightAttrib *attrib = new LightAttrib(*this);
  attrib->_off_lights.erase(light);
  return return_new(attrib);
}

/**
 * Returns the most important light (that is, the light with the highest
 * priority) in the LightAttrib, excluding any ambient lights.  Returns an
 * empty NodePath if no non-ambient lights are found.
 */
NodePath LightAttrib::
get_most_important_light() const {
  check_sorted();

  if (_num_non_ambient_lights > 0) {
    return _sorted_on_lights[0];
  } else {
    return NodePath();
  }
}

/**
 * Returns the total contribution of all the ambient lights.
 */
LColor LightAttrib::
get_ambient_contribution() const {
  check_sorted();

  LVecBase4 total(0);

  Lights::const_iterator li;
  li = _sorted_on_lights.begin() + _num_non_ambient_lights;
  for (; li != _sorted_on_lights.end(); ++li) {
    const NodePath &np = (*li);
    Light *light = np.node()->as_light();
    nassertd(light != nullptr && light->is_ambient_light()) continue;

    total += light->get_color();
  }

  return total;
}

/**
 *
 */
void LightAttrib::
output(std::ostream &out) const {
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

/**
 *
 */
void LightAttrib::
write(std::ostream &out, int indent_level) const {
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

/**
 * Intended to be overridden by derived LightAttrib types to return a unique
 * number indicating whether this LightAttrib is equivalent to the other one.
 *
 * This should return 0 if the two LightAttrib objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two LightAttrib objects whose get_type()
 * functions return the same.
 */
int LightAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const LightAttrib *ta = (const LightAttrib *)other;

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

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t LightAttrib::
get_hash_impl() const {
  size_t hash = 0;

  Lights::const_iterator li;
  for (li = _on_lights.begin(); li != _on_lights.end(); ++li) {
    NodePath light = (*li);
    hash = light.add_hash(hash);
  }

  // This bool value goes here, between the two lists, to differentiate
  // between the two.
  hash = int_hash::add_hash(hash, (int)_off_all_lights);

  for (li = _off_lights.begin(); li != _off_lights.end(); ++li) {
    NodePath light = (*li);
    hash = light.add_hash(hash);
  }

  return hash;
}

/**
 * Intended to be overridden by derived RenderAttrib types to specify how two
 * consecutive RenderAttrib objects of the same type interact.
 *
 * This should return the result of applying the other RenderAttrib to a node
 * in the scene graph below this RenderAttrib, which was already applied.  In
 * most cases, the result is the same as the other RenderAttrib (that is, a
 * subsequent RenderAttrib completely replaces the preceding one).  On the
 * other hand, some kinds of RenderAttrib (for instance, ColorTransformAttrib)
 * might combine in meaningful ways.
 */
CPT(RenderAttrib) LightAttrib::
compose_impl(const RenderAttrib *other) const {
  const LightAttrib *ta = (const LightAttrib *)other;

  if (ta->_off_all_lights) {
    // If the other type turns off all lights, it doesn't matter what we are.
    return ta;
  }

  // This is a three-way merge between ai, bi, and ci, except that bi and ci
  // should have no intersection and therefore needn't be compared to each
  // other.
  Lights::const_iterator ai = _on_lights.begin();
  Lights::const_iterator bi = ta->_on_lights.begin();
  Lights::const_iterator ci = ta->_off_lights.begin();

  // Create a new LightAttrib that will hold the result.
  LightAttrib *new_attrib = new LightAttrib;
  std::back_insert_iterator<Lights> result =
    std::back_inserter(new_attrib->_on_lights);

  while (ai != _on_lights.end() &&
         bi != ta->_on_lights.end() &&
         ci != ta->_off_lights.end()) {
    if ((*ai) < (*bi)) {
      if ((*ai) < (*ci)) {
        // Here is a light that we have in the original, which is not present
        // in the secondary.
        *result = *ai;
        ++ai;
        ++result;

      } else if ((*ci) < (*ai)) {
        // Here is a light that is turned off in the secondary, but was not
        // present in the original.
        ++ci;

      } else { // (*ci) == (*ai)
        // Here is a light that is turned off in the secondary, and was
        // present in the original.
        ++ai;
        ++ci;
      }

    } else if ((*bi) < (*ai)) {
      // Here is a new light we have in the secondary, that was not present in
      // the original.
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
      // Here is a light that we have in the original, which is not present in
      // the secondary.
      *result = *ai;
      ++ai;
      ++result;

    } else if ((*bi) < (*ai)) {
      // Here is a new light we have in the secondary, that was not present in
      // the original.
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
      // Here is a light that we have in the original, which is not present in
      // the secondary.
      *result = *ai;
      ++ai;
      ++result;

    } else if ((*ci) < (*ai)) {
      // Here is a light that is turned off in the secondary, but was not
      // present in the original.
      ++ci;

    } else { // (*ci) == (*ai)
      // Here is a light that is turned off in the secondary, and was present
      // in the original.
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

  // Increase the attrib_ref of all the lights in this new attribute.
  Lights::const_iterator it;
  for (it = new_attrib->_on_lights.begin(); it != new_attrib->_on_lights.end(); ++it) {
    Light *lobj = (*it).node()->as_light();
    nassertd(lobj != nullptr) continue;
    lobj->attrib_ref();
  }

  // This is needed since _sorted_on_lights is not yet populated.
  new_attrib->_sort_seq = UpdateSeq::old();

  return return_new(new_attrib);
}

/**
 * Intended to be overridden by derived RenderAttrib types to specify how two
 * consecutive RenderAttrib objects of the same type interact.
 *
 * See invert_compose() and compose_impl().
 */
CPT(RenderAttrib) LightAttrib::
invert_compose_impl(const RenderAttrib *other) const {
  // I think in this case the other attrib always wins.  Maybe this needs a
  // bit more thought.  It's hard to imagine that it's even important to
  // compute this properly.
  return other;
}

/**
 * Makes sure the lights are sorted in order of priority.  Also counts the
 * number of non-ambient lights.
 */
void LightAttrib::
sort_on_lights() {
  _sort_seq = Light::get_sort_seq();

  // Separate the list of lights into ambient lights and other lights.
  _sorted_on_lights.clear();
  OrderedLights ambient_lights;

  Lights::const_iterator li;
  for (li = _on_lights.begin(); li != _on_lights.end(); ++li) {
    const NodePath &np = (*li);
    nassertd(!np.is_empty() && np.node()->as_light() != nullptr) continue;

    if (!np.node()->is_ambient_light()) {
      _sorted_on_lights.push_back(np);
    } else {
      ambient_lights.push_back(np);
    }
  }

  // Remember how many lights were non-ambient lights, which makes it easier
  // to traverse through the list of non-ambient lights.
  _num_non_ambient_lights = _sorted_on_lights.size();

  // This sort function uses the STL function object defined above.
  sort(_sorted_on_lights.begin(), _sorted_on_lights.end(),
       CompareLightPriorities());

  // Now insert the ambient lights back at the end.  We don't really care
  // about their relative priorities, because their contribution will simply
  // be summed up in the end anyway.
  _sorted_on_lights.insert(_sorted_on_lights.end(),
                           ambient_lights.begin(), ambient_lights.end());
}

/**
 * Tells the BamReader how to create objects of type LightAttrib.
 */
void LightAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void LightAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_bool(_off_all_lights);

  // write the number of off_lights
  dg.add_uint16(get_num_off_lights());

  // write the off lights pointers if any
  Lights::const_iterator fi;
  if (manager->get_file_minor_ver() < 40) {
    for (fi = _off_lights.begin(); fi != _off_lights.end(); ++fi) {
      manager->write_pointer(dg, fi->node());
    }
  } else {
    for (fi = _off_lights.begin(); fi != _off_lights.end(); ++fi) {
      (*fi).write_datagram(manager, dg);
    }
  }

  // write the number of on lights
  dg.add_uint16(get_num_on_lights());
  // write the on lights pointers if any
  Lights::const_iterator nti;
  if (manager->get_file_minor_ver() < 40) {
    for (nti = _on_lights.begin(); nti != _on_lights.end(); ++nti) {
      manager->write_pointer(dg, nti->node());
    }
  } else {
    for (nti = _on_lights.begin(); nti != _on_lights.end(); ++nti) {
      (*nti).write_datagram(manager, dg);
    }
  }
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int LightAttrib::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderAttrib::complete_pointers(p_list, manager);

  if (manager->get_file_minor_ver() >= 40) {
    for (size_t i = 0; i < _off_lights.size(); ++i) {
      pi += _off_lights[i].complete_pointers(p_list + pi, manager);
    }

    for (size_t i = 0; i < _on_lights.size(); ++i) {
      pi += _on_lights[i].complete_pointers(p_list + pi, manager);
    }

  } else {
    BamAuxData *aux = (BamAuxData *)manager->get_aux_data(this, "lights");
    nassertr(aux != nullptr, pi);

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
  }

  return pi;
}

/**
 * Called by the BamReader to perform any final actions needed for setting up
 * the object after all objects have been read and all pointers have been
 * completed.
 */
void LightAttrib::
finalize(BamReader *manager) {
  if (manager->get_file_minor_ver() >= 40) {
    AttribNodeRegistry *areg = AttribNodeRegistry::get_global_ptr();

    // Check if any of the nodes we loaded are mentioned in the
    // AttribNodeRegistry.  If so, replace them.
    for (size_t i = 0; i < _off_lights.size(); ++i) {
      int n = areg->find_node(_off_lights[i]);
      if (n != -1) {
        // If it's in the registry, replace it.
        _off_lights[i] = areg->get_node(n);
      }
    }

    for (size_t i = 0; i < _on_lights.size(); ++i) {
      int n = areg->find_node(_on_lights[i]);
      if (n != -1) {
        // If it's in the registry, replace it.
        _on_lights[i] = areg->get_node(n);
      }

      Light *lobj = _on_lights[i].node()->as_light();
      nassertd(lobj != nullptr) continue;
      lobj->attrib_ref();
    }

  } else {
    // Now it's safe to convert our saved PandaNodes into NodePaths.
    BamAuxData *aux = (BamAuxData *)manager->get_aux_data(this, "lights");
    nassertv(aux != nullptr);
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
        // Otherwise, add any arbitrary NodePath.  Complain if it's ambiguous.
        _off_lights.push_back(NodePath(node));
      }
    }

    _on_lights.reserve(aux->_on_list.size());
    for (ni = aux->_on_list.begin(); ni != aux->_on_list.end(); ++ni) {
      PandaNode *node = (*ni);
      int n = areg->find_node(node->get_type(), node->get_name());
      if (n != -1) {
        // If it's in the registry, add that NodePath.
        _on_lights.push_back(areg->get_node(n));
        node = _on_lights.back().node();
      } else {
        // Otherwise, add any arbitrary NodePath.  Complain if it's ambiguous.
        _on_lights.push_back(NodePath(node));
      }

      Light *lobj = node->as_light();
      nassertd(lobj != nullptr) continue;
      lobj->attrib_ref();
    }
  }

  // Now that the NodePaths have been filled in, we can sort the list.
  _off_lights.sort();
  _on_lights.sort();
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type LightAttrib is encountered in the Bam file.  It should create the
 * LightAttrib and extract its information from the file.
 */
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

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new LightAttrib.
 */
void LightAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _off_all_lights = scan.get_bool();

  if (manager->get_file_minor_ver() >= 40) {
    _off_lights.resize(scan.get_uint16());
    for (size_t i = 0; i < _off_lights.size(); ++i) {
      _off_lights[i].fillin(scan, manager);
    }

    _on_lights.resize(scan.get_uint16());
    for (size_t i = 0; i < _on_lights.size(); ++i) {
      _on_lights[i].fillin(scan, manager);
    }
  } else {
    BamAuxData *aux = new BamAuxData;
    manager->set_aux_data(this, "lights", aux);

    aux->_num_off_lights = scan.get_uint16();
    manager->read_pointers(scan, aux->_num_off_lights);

    aux->_num_on_lights = scan.get_uint16();
    manager->read_pointers(scan, aux->_num_on_lights);
  }

  _sorted_on_lights.clear();
  _sort_seq = UpdateSeq::old();
}
