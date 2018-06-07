/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vertexSlider.I
 * @author drose
 * @date 2005-03-28
 */

/**
 * Returns the name of this particular slider.  Every unique blend shape
 * within a particular Geom must be identified with a different name, which is
 * shared by the slider that controls it.
 */
INLINE const InternalName *VertexSlider::
get_name() const {
  return _name;
}

/**
 * Returns a sequence number that's guaranteed to change at least every time
 * the value reported by get_slider() changes.
 */
INLINE UpdateSeq VertexSlider::
get_modified(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_modified;
}

/**
 *
 */
INLINE VertexSlider::CData::
CData() {
}

/**
 *
 */
INLINE VertexSlider::CData::
CData(const VertexSlider::CData &copy) :
  _modified(copy._modified)
{
}

INLINE std::ostream &
operator << (std::ostream &out, const VertexSlider &obj) {
  obj.output(out);
  return out;
}
