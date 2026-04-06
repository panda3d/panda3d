/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file texMatrixAttrib.I
 * @author drose
 * @date 2002-03-14
 */

/**
 * Use TexMatrixAttrib::make() to construct a new TexMatrixAttrib object.
 */
INLINE TexMatrixAttrib::
TexMatrixAttrib() {
}

/**
 * Use TexMatrixAttrib::make() to construct a new TexMatrixAttrib object.
 */
INLINE TexMatrixAttrib::
TexMatrixAttrib(const TexMatrixAttrib &copy) :
  _stages(copy._stages)
{
}

/**
 * Returns the override value associated with the indicated stage.
 */
INLINE int TexMatrixAttrib::
get_override(TextureStage *stage) const {
  Stages::const_iterator si;
  si = _stages.find(StageNode(stage));
  if (si != _stages.end()) {
    return (*si)._override;
  }
  nassert_raise("Specified TextureStage not included in attrib");
  return 0;
}

/**
 * Returns the union of the Geom::GeomRendering bits that will be required
 * once this TexMatrixAttrib is applied to a geom which includes the indicated
 * geom_rendering bits.
 */
INLINE int TexMatrixAttrib::
get_geom_rendering(int geom_rendering) const {
  if ((geom_rendering & Geom::GR_point_sprite) != 0) {
    if (!is_empty()) {
      geom_rendering |= Geom::GR_point_sprite_tex_matrix;
    }
  }

  return geom_rendering;
}

/**
 *
 */
INLINE TexMatrixAttrib::StageNode::
StageNode(const TextureStage *stage) :
  // Yeah, we cast away the constness here.  Just too much trouble to deal
  // with it properly.
  _stage((TextureStage *)stage),
  _override(0)
{
}

/**
 * Compares the full attributes of StageNodes (as opposed to just the pointer
 * compared by CompareTextureStagePointer, below).
 */
INLINE bool TexMatrixAttrib::StageNode::
operator < (const TexMatrixAttrib::StageNode &other) const {
  if (_stage != other._stage) {
    return _stage < other._stage;
  }
  if (_transform != other._transform) {
    return _transform < other._transform;
  }
  return _override < other._override;
}

/**
 * This STL function object is used to sort a list of texture stages in order
 * by pointer.
 */
INLINE bool TexMatrixAttrib::CompareTextureStagePointer::
operator () (const TexMatrixAttrib::StageNode &a,
             const TexMatrixAttrib::StageNode &b) const {
  return a._stage < b._stage;
}
