/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomVertexAnimationSpec.I
 * @author drose
 * @date 2005-03-29
 */

/**
 *
 */
INLINE GeomVertexAnimationSpec::
GeomVertexAnimationSpec() :
  _animation_type(AT_none),
  _num_transforms(0),
  _indexed_transforms(0)
{
}

/**
 *
 */
INLINE GeomVertexAnimationSpec::
GeomVertexAnimationSpec(const GeomVertexAnimationSpec &other) :
  _animation_type(other._animation_type),
  _num_transforms(other._num_transforms),
  _indexed_transforms(other._indexed_transforms)
{
}

/**
 *
 */
INLINE void GeomVertexAnimationSpec::
operator = (const GeomVertexAnimationSpec &other) {
  _animation_type = other._animation_type;
  _num_transforms = other._num_transforms;
  _indexed_transforms = other._indexed_transforms;
}

/**
 * Returns the type of animation represented by this spec.
 */
INLINE GeomVertexAnimationSpec::AnimationType GeomVertexAnimationSpec::
get_animation_type() const {
  return _animation_type;
}

/**
 * This is only meaningful for animation_type AT_hardware.  It specifies the
 * maximum number of transforms that might be simultaneously applied to any
 * one vertex by the data in this format.
 */
INLINE int GeomVertexAnimationSpec::
get_num_transforms() const {
  return _num_transforms;
}

/**
 * This is only meaningful for animation_type AT_hardware.  If true, it
 * indicates that the format uses indexed animation tables.  It is false if
 * each vertex will reference the first _num_transforms table entries only.
 */
INLINE bool GeomVertexAnimationSpec::
get_indexed_transforms() const {
  return _indexed_transforms;
}

/**
 * Specifies that no vertex animation is represented by this spec.
 */
INLINE void GeomVertexAnimationSpec::
set_none() {
  _animation_type = AT_none;
}

/**
 * Specifies that vertex animation is to be performed by Panda.  This is the
 * most general setting and can handle any kind of vertex animation
 * represented.
 */
INLINE void GeomVertexAnimationSpec::
set_panda() {
  _animation_type = AT_panda;
}

/**
 * Specifies that vertex animation is to be performed by the graphics hardware
 * (or at least by the graphics backend API, which is actually still free to
 * animate the vertices on the CPU).
 *
 * This is only legal if the graphics hardware can support the specified
 * limits on number of transforms and/or indexed transforms.  Also, no current
 * graphics API's support morphing.
 */
INLINE void GeomVertexAnimationSpec::
set_hardware(int num_transforms, bool indexed_transforms) {
  _animation_type = AT_hardware;
  _num_transforms = num_transforms;
  _indexed_transforms = indexed_transforms;
}

/**
 * Provides an arbitrary ordering between different animation specs.
 */
INLINE bool GeomVertexAnimationSpec::
operator < (const GeomVertexAnimationSpec &other) const {
  return (compare_to(other) < 0);
}

/**
 *
 */
INLINE bool GeomVertexAnimationSpec::
operator == (const GeomVertexAnimationSpec &other) const {
  return (compare_to(other) == 0);
}

/**
 *
 */
INLINE bool GeomVertexAnimationSpec::
operator != (const GeomVertexAnimationSpec &other) const {
  return (compare_to(other) != 0);
}

/**
 * Provides an arbitrary ordering between different animation specs.
 */
INLINE int GeomVertexAnimationSpec::
compare_to(const GeomVertexAnimationSpec &other) const {
  if (_animation_type != other._animation_type) {
    return (int)_animation_type - (int)other._animation_type;
  }

  if (_animation_type == AT_hardware) {
    if (_num_transforms != other._num_transforms) {
      return _num_transforms - other._num_transforms;
    }
    if (_indexed_transforms != other._indexed_transforms) {
      return (int)_indexed_transforms - (int)other._indexed_transforms;
    }
  }

  return 0;
}

INLINE std::ostream &
operator << (std::ostream &out, const GeomVertexAnimationSpec &animation) {
  animation.output(out);
  return out;
}
