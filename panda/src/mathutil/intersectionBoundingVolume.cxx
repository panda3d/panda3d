/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file intersectionBoundingVolume.cxx
 * @author drose
 * @date 2012-02-08
 */

#include "intersectionBoundingVolume.h"
#include "unionBoundingVolume.h"
#include "config_mathutil.h"
#include "dcast.h"

TypeHandle IntersectionBoundingVolume::_type_handle;

/**
 *
 */
IntersectionBoundingVolume::
IntersectionBoundingVolume(const IntersectionBoundingVolume &copy) :
  GeometricBoundingVolume(copy),
  _components(copy._components)
{
}

/**
 *
 */
BoundingVolume *IntersectionBoundingVolume::
make_copy() const {
  return new IntersectionBoundingVolume(*this);
}

/**
 *
 */
LPoint3 IntersectionBoundingVolume::
get_approx_center() const {
  nassertr(!is_empty(), LPoint3::zero());
  nassertr(!is_infinite(), LPoint3::zero());

  LPoint3 center = LPoint3::zero();
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    center += (*ci)->get_approx_center();
  }

  return center / (PN_stdfloat)_components.size();
}

/**
 *
 */
void IntersectionBoundingVolume::
xform(const LMatrix4 &mat) {
  nassertv(!mat.is_nan());

  for (Components::iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    PT(GeometricBoundingVolume) copy = DCAST(GeometricBoundingVolume, (*ci)->make_copy());
    copy->xform(mat);
    (*ci) = copy;
  }
}

/**
 *
 */
void IntersectionBoundingVolume::
output(std::ostream &out) const {
  if (is_empty()) {
    out << "intersection, empty";
  } else if (is_infinite()) {
    out << "intersection, infinite";
  } else {
    out << "intersection [";
    for (Components::const_iterator ci = _components.begin();
         ci != _components.end();
         ++ci) {
      out << " " << *(*ci);
    }
    out << " ]";
  }
}

/**
 *
 */
void IntersectionBoundingVolume::
write(std::ostream &out, int indent_level) const {
  if (is_empty()) {
    indent(out, indent_level) << "intersection, empty\n";
  } else if (is_infinite()) {
    indent(out, indent_level) << "intersection, infinite\n";
  } else {
    indent(out, indent_level) << "intersection {\n";
    for (Components::const_iterator ci = _components.begin();
         ci != _components.end();
         ++ci) {
      (*ci)->write(out, indent_level + 2);
    }
    indent(out, indent_level) << "}\n";
  }
}

/**
 * Removes all components from the volume.
 */
void IntersectionBoundingVolume::
clear_components() {
  _components.clear();
  _flags = F_infinite;
}

/**
 * Adds a new component to the volume.  This does not necessarily increase the
 * total number of components by one, and you may or may not be able to find
 * this component in the volume by a subsequent call to get_component();
 * certain optimizations may prevent the component from being added, or have
 * other unexpected effects on the total set of components.
 */
void IntersectionBoundingVolume::
add_component(const GeometricBoundingVolume *component) {
  CPT(GeometricBoundingVolume) gbv;

  if (component->is_exact_type(UnionBoundingVolume::get_class_type())) {
    // Here's a special case.  We'll construct a new union that includes only
    // those components that have some intersection with our existing
    // components.  (No need to include the components that have no
    // intersection.)
    PT(UnionBoundingVolume) unionv = DCAST(UnionBoundingVolume, component->make_copy());
    unionv->filter_intersection(this);

    // Save the modified union in a PT() so it won't be destructed.
    gbv = unionv.p();

    if (unionv->get_num_components() == 1) {
      // If there's only one component left, use just that one.
      gbv = unionv->get_component(0);
    }

    component = gbv;
  }

  if (component->is_empty()) {
    _flags = F_empty;
    _components.clear();

  } else if (component->is_infinite() || is_empty()) {
    // No-op.

  } else if (component->is_exact_type(IntersectionBoundingVolume::get_class_type())) {
    // Another special case.  Just more components.
    const IntersectionBoundingVolume *other = DCAST(IntersectionBoundingVolume, component);
    for (Components::const_iterator ci = other->_components.begin();
         ci != other->_components.end();
         ++ci) {
      add_component(*ci);
    }

  } else {
    // The general case.
    size_t i = 0;
    while (i < _components.size()) {
      const GeometricBoundingVolume *existing = _components[i];
      ++i;

      int result = component->contains(existing);
      if ((result & IF_all) != 0) {
        // The existing component is entirely within this one; no need to do
        // anything with it.
        return;

      } else if (result == 0) {
        // No intersection between these components; we're now empty.
        _flags = F_empty;
        _components.clear();
        return;
      }

      result = existing->contains(component);
      if ((result & IF_all) != 0) {
        // This new component is entirely within an existing component; no
        // need to keep the existing one.
        --i;
        _components.erase(_components.begin() + i);

      } else if (result == 0) {
        // No intersection between these components; we're now empty.
        _flags = F_empty;
        _components.clear();
        return;
      }
    }

    _flags &= ~F_infinite;
    _components.push_back(component);
  }
}

/**
 *
 */
bool IntersectionBoundingVolume::
extend_other(BoundingVolume *other) const {
  return other->extend_by_intersection(this);
}

/**
 *
 */
bool IntersectionBoundingVolume::
around_other(BoundingVolume *other,
             const BoundingVolume **first,
             const BoundingVolume **last) const {
  return other->around_intersections(first, last);
}

/**
 *
 */
int IntersectionBoundingVolume::
contains_other(const BoundingVolume *other) const {
  return other->contains_intersection(this);
}

/**
 *
 */
int IntersectionBoundingVolume::
contains_point(const LPoint3 &point) const {
  nassertr(!point.is_nan(), IF_no_intersection);

  int result = IF_possible | IF_some | IF_all;
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    int this_result = (*ci)->contains(point);
    if ((this_result & IF_dont_understand) != 0) {
      result |= IF_dont_understand;
      break;
    }
    result &= this_result;
    if ((result & IF_possible) == 0) {
      // No point in looking further.
      break;
    }
  }

  return result;
}

/**
 *
 */
int IntersectionBoundingVolume::
contains_lineseg(const LPoint3 &a, const LPoint3 &b) const {
  nassertr(!a.is_nan() && !b.is_nan(), IF_no_intersection);

  int result = IF_possible | IF_some | IF_all;
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    int this_result = (*ci)->contains(a, b);
    if ((this_result & IF_dont_understand) != 0) {
      result |= IF_dont_understand;
      break;
    }
    result &= this_result;
    if ((result & IF_possible) == 0) {
      // No point in looking further.
      break;
    }
  }

  return result;
}

/**
 * Double-dispatch support: called by contains_other() when the type we're
 * testing for intersection is known to be a sphere.
 */
int IntersectionBoundingVolume::
contains_sphere(const BoundingSphere *sphere) const {
  int result = IF_possible | IF_some | IF_all;
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    int this_result = (*ci)->contains_sphere(sphere);
    if ((this_result & IF_dont_understand) != 0) {
      result |= IF_dont_understand;
      break;
    }
    result &= this_result;
    if ((result & IF_possible) == 0) {
      // No point in looking further.
      break;
    }
  }

  return result;
}

/**
 * Double-dispatch support: called by contains_other() when the type we're
 * testing for intersection is known to be a box.
 */
int IntersectionBoundingVolume::
contains_box(const BoundingBox *box) const {
  int result = IF_possible | IF_some | IF_all;
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    int this_result = (*ci)->contains_box(box);
    if ((this_result & IF_dont_understand) != 0) {
      result |= IF_dont_understand;
      break;
    }
    result &= this_result;
    if ((result & IF_possible) == 0) {
      // No point in looking further.
      break;
    }
  }

  return result;
}

/**
 * Double-dispatch support: called by contains_other() when the type we're
 * testing for intersection is known to be a hexahedron.
 */
int IntersectionBoundingVolume::
contains_hexahedron(const BoundingHexahedron *hexahedron) const {
  int result = IF_possible | IF_some | IF_all;
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    int this_result = (*ci)->contains_hexahedron(hexahedron);
    if ((this_result & IF_dont_understand) != 0) {
      result |= IF_dont_understand;
      break;
    }
    result &= this_result;
    if ((result & IF_possible) == 0) {
      // No point in looking further.
      break;
    }
  }

  return result;
}

/**
 * Double-dispatch support: called by contains_other() when the type we're
 * testing for intersection is known to be a line.
 */
int IntersectionBoundingVolume::
contains_line(const BoundingLine *line) const {
  int result = IF_possible | IF_some | IF_all;
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    int this_result = (*ci)->contains_line(line);
    if ((this_result & IF_dont_understand) != 0) {
      result |= IF_dont_understand;
      break;
    }
    result &= this_result;
    if ((result & IF_possible) == 0) {
      // No point in looking further.
      break;
    }
  }

  return result;
}

/**
 * Double-dispatch support: called by contains_other() when the type we're
 * testing for intersection is known to be a plane.
 */
int IntersectionBoundingVolume::
contains_plane(const BoundingPlane *plane) const {
  int result = IF_possible | IF_some | IF_all;
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    int this_result = (*ci)->contains_plane(plane);
    if ((this_result & IF_dont_understand) != 0) {
      result |= IF_dont_understand;
      break;
    }
    result &= this_result;
    if ((result & IF_possible) == 0) {
      // No point in looking further.
      break;
    }
  }

  return result;
}

/**
 * Double-dispatch support: called by contains_other() when the type we're
 * testing for intersection is known to be a union object.
 */
int IntersectionBoundingVolume::
contains_union(const UnionBoundingVolume *unionv) const {
  int result = IF_possible | IF_some | IF_all;
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    int this_result = (*ci)->contains_union(unionv);
    if ((this_result & IF_dont_understand) != 0) {
      result |= IF_dont_understand;
      break;
    }
    result &= this_result;
    if ((result & IF_possible) == 0) {
      // No point in looking further.
      break;
    }
  }

  return result;
}

/**
 * Double-dispatch support: called by contains_other() when the type we're
 * testing for intersection is known to be an intersection object.
 */
int IntersectionBoundingVolume::
contains_intersection(const IntersectionBoundingVolume *intersection) const {
  int result = IF_possible | IF_some | IF_all;
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    int this_result = (*ci)->contains_intersection(intersection);
    if ((this_result & IF_dont_understand) != 0) {
      result |= IF_dont_understand;
      break;
    }
    result &= this_result;
    if ((result & IF_possible) == 0) {
      // No point in looking further.
      break;
    }
  }

  return result;
}

/**
 * Generic handler for a FiniteBoundingVolume.
 */
int IntersectionBoundingVolume::
contains_finite(const FiniteBoundingVolume *volume) const {
  int result = IF_possible | IF_some | IF_all;
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    int this_result = (*ci)->contains_finite(volume);
    if ((this_result & IF_dont_understand) != 0) {
      result |= IF_dont_understand;
      break;
    }
    result &= this_result;
    if ((result & IF_possible) == 0) {
      // No point in looking further.
      break;
    }
  }

  return result;
}

/**
 * Generic handler for a GeometricBoundingVolume.
 */
int IntersectionBoundingVolume::
contains_geometric(const GeometricBoundingVolume *volume) const {
  int result = IF_possible | IF_some | IF_all;
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    int this_result = (*ci)->contains_geometric(volume);
    if ((this_result & IF_dont_understand) != 0) {
      result |= IF_dont_understand;
      break;
    }
    result &= this_result;
    if ((result & IF_possible) == 0) {
      // No point in looking further.
      break;
    }
  }

  return result;
}


/**
 * Generic reverse-direction comparison.  Called by BoundingVolumes that do
 * not implement contains_intersection() explicitly.  This returns the test of
 * whether the other volume contains this volume.
 */
int IntersectionBoundingVolume::
other_contains_intersection(const BoundingVolume *volume) const {
  int result = IF_possible | IF_some | IF_all;
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    int this_result = volume->contains(*ci);
    if ((this_result & IF_dont_understand) != 0) {
      result |= IF_dont_understand;
      break;
    }
    result &= this_result;
    if ((result & IF_possible) == 0) {
      // No point in looking further.
      break;
    }
  }

  return result;
}
