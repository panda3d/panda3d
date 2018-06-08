/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cLwoSurfaceBlock.cxx
 * @author drose
 * @date 2001-04-26
 */

#include "cLwoSurfaceBlock.h"
#include "cLwoSurfaceBlockTMap.h"
#include "lwoToEggConverter.h"

#include "lwoSurfaceBlockChannel.h"
#include "lwoSurfaceBlockEnabled.h"
#include "lwoSurfaceBlockImage.h"
#include "lwoSurfaceBlockRepeat.h"
#include "lwoSurfaceBlockVMapName.h"
#include "dcast.h"


/**
 *
 */
CLwoSurfaceBlock::
CLwoSurfaceBlock(LwoToEggConverter *converter, const LwoSurfaceBlock *block) :
  _converter(converter),
  _block(block)
{
  _block_type = _block->_header->get_id();
  _ordinal = _block->_header->_ordinal;
  _enabled = true;
  _opacity_type = LwoSurfaceBlockOpacity::T_additive;
  _opacity = 1.0;
  _transform = LMatrix4d::ident_mat();
  _inv_transform = LMatrix4d::ident_mat();
  _projection_mode = LwoSurfaceBlockProjection::M_uv;
  _axis = LwoSurfaceBlockAxis::A_y;
  _clip_index = -1;
  _w_wrap = LwoSurfaceBlockWrap::M_repeat;
  _h_wrap = LwoSurfaceBlockWrap::M_repeat;
  _w_repeat = 1.0;
  _h_repeat = 1.0;
  _tmap = nullptr;

  // Scan the chunks in the header.
  int num_hchunks = _block->_header->get_num_chunks();
  for (int hi = 0; hi < num_hchunks; hi++) {
    const IffChunk *hchunk = _block->_header->get_chunk(hi);

    if (hchunk->is_of_type(LwoSurfaceBlockChannel::get_class_type())) {
      const LwoSurfaceBlockChannel *bc =
        DCAST(LwoSurfaceBlockChannel, hchunk);
      _channel_id = bc->_channel_id;

    } else if (hchunk->is_of_type(LwoSurfaceBlockEnabled::get_class_type())) {
      const LwoSurfaceBlockEnabled *ec =
        DCAST(LwoSurfaceBlockEnabled, hchunk);
      _enabled = ec->_enabled;
    }
  }

  // Scan the chunks in the body.
  int num_chunks = _block->get_num_chunks();
  for (int i = 0; i < num_chunks; i++) {
    const IffChunk *chunk = _block->get_chunk(i);

    if (chunk->is_of_type(LwoSurfaceBlockTMap::get_class_type())) {
      const LwoSurfaceBlockTMap *lwo_tmap = DCAST(LwoSurfaceBlockTMap, chunk);
      if (_tmap != nullptr) {
        nout << "Two TMAP chunks encountered within surface block.\n";
        delete _tmap;
      }
      _tmap = new CLwoSurfaceBlockTMap(_converter, lwo_tmap);

    } else if (chunk->is_of_type(LwoSurfaceBlockProjection::get_class_type())) {
      const LwoSurfaceBlockProjection *proj = DCAST(LwoSurfaceBlockProjection, chunk);
      _projection_mode = proj->_mode;

    } else if (chunk->is_of_type(LwoSurfaceBlockAxis::get_class_type())) {
      const LwoSurfaceBlockAxis *axis = DCAST(LwoSurfaceBlockAxis, chunk);
      _axis = axis->_axis;

    } else if (chunk->is_of_type(LwoSurfaceBlockImage::get_class_type())) {
      const LwoSurfaceBlockImage *image = DCAST(LwoSurfaceBlockImage, chunk);
      _clip_index = image->_index;

    } else if (chunk->is_of_type(LwoSurfaceBlockWrap::get_class_type())) {
      const LwoSurfaceBlockWrap *wrap = DCAST(LwoSurfaceBlockWrap, chunk);
      _w_wrap = wrap->_width;
      _h_wrap = wrap->_height;

    } else if (chunk->is_of_type(LwoSurfaceBlockWrap::get_class_type())) {
      const LwoSurfaceBlockWrap *wrap = DCAST(LwoSurfaceBlockWrap, chunk);
      _w_wrap = wrap->_width;
      _h_wrap = wrap->_height;

    } else if (chunk->is_of_type(LwoSurfaceBlockVMapName::get_class_type())) {
      const LwoSurfaceBlockVMapName *vmap = DCAST(LwoSurfaceBlockVMapName, chunk);
      _uv_name = vmap->_name;

    } else if (chunk->is_of_type(LwoSurfaceBlockRepeat::get_class_type())) {
      const LwoSurfaceBlockRepeat *repeat = DCAST(LwoSurfaceBlockRepeat, chunk);
      if (repeat->get_id() == IffId("WRPW")) {
        _w_repeat = repeat->_cycles;
      } else if (repeat->get_id() == IffId("WRPH")) {
        _h_repeat = repeat->_cycles;
      }
    }
  }

  if (_tmap != nullptr) {
    _tmap->get_transform(_transform);
  }

  // Also rotate the transform if we specify some axis other than Y. (All the
  // map_* uv mapping functions are written to assume Y is the dominant axis.)
  switch (_axis) {
  case LwoSurfaceBlockAxis::A_x:
    _transform = LMatrix4d::rotate_mat(90.0,
                                       LVecBase3d::unit_z(),
                                       CS_yup_left) * _transform;
    break;

  case LwoSurfaceBlockAxis::A_y:
    break;

  case LwoSurfaceBlockAxis::A_z:
    _transform = LMatrix4d::rotate_mat(-90.0,
                                       LVecBase3d::unit_x(),
                                       CS_yup_left) * _transform;
    break;
  }

  _inv_transform.invert_from(_transform);
}

/**
 *
 */
CLwoSurfaceBlock::
~CLwoSurfaceBlock() {
  if (_tmap != nullptr) {
    delete _tmap;
  }
}
