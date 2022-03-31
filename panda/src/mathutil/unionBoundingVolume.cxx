/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file unionBoundingVolume.cxx
 * @author drose
 * @date 2012-02-08
 */

#include "unionBoundingVolume.h"
#include "config_mathutil.h"
#include "dcast.h"
#include "indent.h"

TypeHandle UnionBoundingVolume::_type_handle;

/**
 *
 */
UnionBoundingVolume::
UnionBoundingVolume(const UnionBoundingVolume &copy) :
  GeometricBoundingVolume(copy),
  _components(copy._components)
{
}

/**
 *
 */
BoundingVolume *UnionBoundingVolume::
make_copy() const {
  return new UnionBoundingVolume(*this);
}

/**
 *
 */
LPoint3 UnionBoundingVolume::
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
void UnionBoundingVolume::
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
void UnionBoundingVolume::
output(std::ostream &out) const {
  if (is_empty()) {
    out << "union, empty";
  } else if (is_infinite()) {
    out << "union, infinite";
  } else {
    out << "union [";
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
void UnionBoundingVolume::
write(std::ostream &out, int indent_level) const {
  if (is_empty()) {
    indent(out, indent_level) << "union, empty\n";
  } else if (is_infinite()) {
    indent(out, indent_level) << "union, infinite\n";
  } else {
    indent(out, indent_level) << "union {\n";
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
void UnionBoundingVolume::
clear_components() {
  _components.clear();
  _flags = F_empty;
}

/**
 * Adds a new component to the volume.  This does not necessarily increase the
 * total number of components by one, and you may or may not be able to find
 * this component in the volume by a subsequent call to get_component();
 * certain optimizations may prevent the component from being added, or have
 * other unexpected effects on the total set of components.
 */
void UnionBoundingVolume::
add_component(const GeometricBoundingVolume *component) {
  if (component->is_infinite()) {
    _flags = F_infinite;
    _components.clear();

  } else if (component->is_empty() || is_infinite()) {
    // No-op.

  } else {
    size_t i = 0;
    while (i < _components.size()) {
      const GeometricBoundingVolume *existing = _components[i];
      ++i;

      int result = existing->contains(component);
      if ((result & IF_all) != 0) {
        // This new component is entirely within an existing component; no
        // need to do anything with it.
        return;
      }

      result = component->contains(existing);
      if ((result & IF_all) != 0) {
        // The existing component is entirely within this one; no need to keep
        // the existing one.
        --i;
        _components.erase(_components.begin() + i);
      }
    }

    _flags &= ~F_empty;
    _components.push_back(component);
  }
}

/**
 * Removes from the union any components that have no intersection with the
 * indicated volume.
 */
void UnionBoundingVolume::
filter_intersection(const BoundingVolume *volume) {
  size_t i = 0;
  while (i < _components.size()) {
    const GeometricBoundingVolume *existing = _components[i];
    ++i;

    int result = volume->contains(existing);
    if ((result & IF_possible) == 0) {
      // There is no intersection.  Remove this component.
      --i;
      _components.erase(_components.begin() + i);
    }
  }

  if (_components.empty()) {
    _flags |= F_empty;
  }
}

/**
 *
 */
bool UnionBoundingVolume::
extend_other(BoundingVolume *other) const {
  return other->extend_by_union(this);
}

/**
 *
 */
bool UnionBoundingVolume::
around_other(BoundingVolume *other,
             const BoundingVolume **first,
             const BoundingVolume **last) const {
  return other->around_unions(first, last);
}

/**
 *
 */
int UnionBoundingVolume::
contains_other(const BoundingVolume *other) const {
  return other->contains_union(this);
}

/**
 *
 */
bool UnionBoundingVolume::
extend_by_geometric(const GeometricBoundingVolume *volume) {
  add_component(volume);
  return true;
}

/**
 *
 */
bool UnionBoundingVolume::
around_geometric(const BoundingVolume **first,
                 const BoundingVolume **last) {
  nassertr(first != last, false);

  clear_components();

  const BoundingVolume **p = first;
  while (p != last) {
    nassertr(!(*p)->is_infinite(), false);
    if (!(*p)->is_empty()) {
      const GeometricBoundingVolume *volume = (*p)->as_geometric_bounding_volume();
      if (volume != nullptr) {
        add_component(volume);
      } else {
        set_infinite();
        _components.clear();
        return false;
      }
    }
  }

  return true;
}

/**
 *
 */
int UnionBoundingVolume::
contains_point(const LPoint3 &point) const {
  nassertr(!point.is_nan(), IF_no_intersection);

  int result = 0;
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    result |= (*ci)->contains(point);
    if ((result & (IF_all | IF_dont_understand)) != 0) {
      // No point in looking further.
      break;
    }
  }

  return result;
}

/**
 *
 */
int UnionBoundingVolume::
contains_lineseg(const LPoint3 &a, const LPoint3 &b) const {
  nassertr(!a.is_nan() && !b.is_nan(), IF_no_intersection);

  int result = 0;
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    result |= (*ci)->contains(a, b);
    if ((result & (IF_all | IF_dont_understand)) != 0) {
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
int UnionBoundingVolume::
contains_sphere(const BoundingSphere *sphere) const {
  int result = 0;
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    result |= (*ci)->contains_sphere(sphere);
    if ((result & (IF_all | IF_dont_understand)) != 0) {
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
int UnionBoundingVolume::
contains_box(const BoundingBox *box) const {
  int result = 0;
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    result |= (*ci)->contains_box(box);
    if ((result & (IF_all | IF_dont_understand)) != 0) {
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
int UnionBoundingVolume::
contains_hexahedron(const BoundingHexahedron *hexahedron) const {
  int result = 0;
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    result |= (*ci)->contains_hexahedron(hexahedron);
    if ((result & (IF_all | IF_dont_understand)) != 0) {
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
int UnionBoundingVolume::
contains_line(const BoundingLine *line) const {
  int result = 0;
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    result |= (*ci)->contains_line(line);
    if ((result & (IF_all | IF_dont_understand)) != 0) {
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
int UnionBoundingVolume::
contains_plane(const BoundingPlane *plane) const {
  int result = 0;
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    result |= (*ci)->contains_plane(plane);
    if ((result & (IF_all | IF_dont_understand)) != 0) {
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
int UnionBoundingVolume::
contains_union(const UnionBoundingVolume *unionv) const {
  int result = 0;
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    result |= (*ci)->contains_union(unionv);
    if ((result & (IF_all | IF_dont_understand)) != 0) {
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
int UnionBoundingVolume::
contains_intersection(const IntersectionBoundingVolume *intersection) const {
  int result = 0;
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    result |= (*ci)->contains_intersection(intersection);
    if ((result & (IF_all | IF_dont_understand)) != 0) {
      // No point in looking further.
      break;
    }
  }

  return result;
}

/**
 * Generic handler for a FiniteBoundingVolume.
 */
int UnionBoundingVolume::
contains_finite(const FiniteBoundingVolume *volume) const {
  int result = 0;
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    result |= (*ci)->contains_finite(volume);
    if ((result & (IF_all | IF_dont_understand)) != 0) {
      // No point in looking further.
      break;
    }
  }

  return result;
}

/**
 * Generic handler for a GeometricBoundingVolume.
 */
int UnionBoundingVolume::
contains_geometric(const GeometricBoundingVolume *volume) const {
  int result = 0;
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    result |= (*ci)->contains_geometric(volume);
    if ((result & (IF_all | IF_dont_understand)) != 0) {
      // No point in looking further.
      break;
    }
  }

  return result;
}

/**
 * Generic reverse-direction comparison.  Called by BoundingVolumes that do
 * not implement contains_union() explicitly.  This returns the test of
 * whether the other volume contains this volume.
 */
int UnionBoundingVolume::
other_contains_union(const BoundingVolume *volume) const {
  int all_result = IF_possible | IF_some | IF_all;
  int some_result = 0;
  for (Components::const_iterator ci = _components.begin();
       ci != _components.end();
       ++ci) {
    int this_result = volume->contains(*ci);
    if ((this_result & IF_dont_understand) != 0) {
      some_result |= IF_dont_understand;
      break;
    }
    all_result &= this_result;
    some_result |= this_result;
  }

  some_result &= ~IF_all;
  return some_result | all_result;
}
