/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomVertexAnimationSpec.h
 * @author drose
 * @date 2005-03-29
 */

#ifndef GEOMVERTEXANIMATIONSPEC_H
#define GEOMVERTEXANIMATIONSPEC_H

#include "pandabase.h"
#include "geomEnums.h"

class BamWriter;
class BamReader;
class Datagram;
class DatagramIterator;

/**
 * This object describes how the vertex animation, if any, represented in a
 * GeomVertexData is encoded.
 *
 * Vertex animation includes soft-skinned skeleton animation and morphs (blend
 * shapes), and might be performed on the CPU by Panda, or passed down to the
 * graphics backed to be performed on the hardware (depending on the
 * hardware's advertised capabilities).
 *
 * Changing this setting doesn't by itself change the way the animation is
 * actually performed; this just specifies how the vertices are set up to be
 * animated.
 */
class EXPCL_PANDA_GOBJ GeomVertexAnimationSpec : public GeomEnums {
PUBLISHED:
  INLINE GeomVertexAnimationSpec();
  INLINE GeomVertexAnimationSpec(const GeomVertexAnimationSpec &other);
  INLINE void operator = (const GeomVertexAnimationSpec &other);

  INLINE AnimationType get_animation_type() const;
  MAKE_PROPERTY(animation_type, get_animation_type);

  INLINE int get_num_transforms() const;
  INLINE bool get_indexed_transforms() const;
  MAKE_PROPERTY(num_transforms, get_num_transforms);
  MAKE_PROPERTY(indexed_transforms, get_indexed_transforms);

  INLINE void set_none();
  INLINE void set_panda();
  INLINE void set_hardware(int num_transforms, bool indexed_transforms);

  void output(std::ostream &out) const;

public:
  INLINE bool operator < (const GeomVertexAnimationSpec &other) const;
  INLINE bool operator == (const GeomVertexAnimationSpec &other) const;
  INLINE bool operator != (const GeomVertexAnimationSpec &other) const;
  INLINE int compare_to(const GeomVertexAnimationSpec &other) const;

public:
  void write_datagram(BamWriter *manager, Datagram &dg);
  void fillin(DatagramIterator &scan, BamReader *manager);

private:
  AnimationType _animation_type;

  int _num_transforms;
  bool _indexed_transforms;
};

INLINE std::ostream &
operator << (std::ostream &out, const GeomVertexAnimationSpec &animation);

#include "geomVertexAnimationSpec.I"

#endif
