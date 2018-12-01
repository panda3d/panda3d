/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMesherFanMaker.I
 * @author drose
 * @date 2005-03-22
 */

/**
 * Provides a unique ordering between different fan makers based on the
 * leading edge.
 */
INLINE bool EggMesherFanMaker::
operator < (const EggMesherFanMaker &other) const {
  nassertr(!_edges.empty() && !other._edges.empty(), false);
  return _edges.front() < other._edges.front();
}

/**
 * Provides a unique ordering between different fan makers based on the
 * leading edge.
 */
INLINE bool EggMesherFanMaker::
operator != (const EggMesherFanMaker &other) const {
  return !operator == (other);
}

/**
 * Provides a unique ordering between different fan makers based on the
 * leading edge.
 */
INLINE bool EggMesherFanMaker::
operator == (const EggMesherFanMaker &other) const {
  return _edges.front() == other._edges.front();
}

/**
 * Returns true if the fan maker has no edges, false otherwise.
 */
INLINE bool EggMesherFanMaker::
is_empty() const {
  return (_edges.empty());
}

/**
 * Returns true if the fan maker has enough edges to define at least one fan,
 * false otherwise.
 */
INLINE bool EggMesherFanMaker::
is_valid() const {
  return (_edges.size() > 2);
}

/**
 * Returns true if the strip and the other strip are coplanar.
 */
INLINE bool EggMesherFanMaker::
is_coplanar_with(const EggMesherFanMaker &other) const {
  return _planar && other._planar &&
    _strips.front()->is_coplanar_with(*other._strips.front(),
                                      egg_coplanar_threshold);
}

INLINE std::ostream &
operator << (std::ostream &out, const EggMesherFanMaker &fm) {
  fm.output(out);
  return out;
}
