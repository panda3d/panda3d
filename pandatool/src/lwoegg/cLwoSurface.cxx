// Filename: cLwoSurface.cxx
// Created by:  drose (25Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "cLwoSurface.h"
#include "cLwoSurfaceBlock.h"
#include "cLwoClip.h"
#include "lwoToEggConverter.h"

#include <lwoSurfaceColor.h>
#include <lwoSurfaceParameter.h>
#include <lwoSurfaceSmoothingAngle.h>
#include <lwoSurfaceSidedness.h>
#include <lwoSurfaceBlock.h>
#include <eggPrimitive.h>
#include <string_utils.h>
#include <mathNumbers.h>


////////////////////////////////////////////////////////////////////
//     Function: CLwoSurface::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CLwoSurface::
CLwoSurface(LwoToEggConverter *converter, const LwoSurface *surface) :
  _converter(converter),
  _surface(surface)
{
  _flags = 0;
  _checked_texture = false;
  _has_uvs = false;
  _block = (CLwoSurfaceBlock *)NULL;

  // Walk through the chunk list, looking for some basic properties.
  int num_chunks = _surface->get_num_chunks();
  for (int i = 0; i < num_chunks; i++) {
    const IffChunk *chunk = _surface->get_chunk(i);

    if (chunk->is_of_type(LwoSurfaceColor::get_class_type())) {
      const LwoSurfaceColor *color = DCAST(LwoSurfaceColor, chunk);
      _flags |= F_color;
      _color = color->_color;

    } else if (chunk->is_of_type(LwoSurfaceParameter::get_class_type())) {
      const LwoSurfaceParameter *param = DCAST(LwoSurfaceParameter, chunk);
      IffId type = param->get_id();

      if (type == IffId("DIFF")) {
	_flags |= F_diffuse;
	_diffuse = param->_value;

      } else if (type == IffId("LUMI")) {
	_flags |= F_luminosity;
	_luminosity = param->_value;

      } else if (type == IffId("SPEC")) {
	_flags |= F_specular;
	_specular = param->_value;

      } else if (type == IffId("REFL")) {
	_flags |= F_reflection;
	_reflection = param->_value;

      } else if (type == IffId("TRAN")) {
	_flags |= F_transparency;
	_transparency = param->_value;

      } else if (type == IffId("TRNL")) {
	_flags |= F_translucency;
	_translucency = param->_value;
      }

    } else if (chunk->is_of_type(LwoSurfaceSmoothingAngle::get_class_type())) {
      const LwoSurfaceSmoothingAngle *sa = DCAST(LwoSurfaceSmoothingAngle, chunk);
      _flags |= F_smooth_angle;
      _smooth_angle = sa->_angle;

    } else if (chunk->is_of_type(LwoSurfaceSidedness::get_class_type())) {
      const LwoSurfaceSidedness *sn = DCAST(LwoSurfaceSidedness, chunk);
      _flags |= F_backface;
      _backface = (sn->_sidedness == LwoSurfaceSidedness::S_front_and_back);

    } else if (chunk->is_of_type(LwoSurfaceBlock::get_class_type())) {
      const LwoSurfaceBlock *lwo_block = DCAST(LwoSurfaceBlock, chunk);
      // One of possibly several blocks in the texture that define
      // additional fancy rendering properties.

      CLwoSurfaceBlock *block = new CLwoSurfaceBlock(_converter, lwo_block);

      // We only consider enabled "IMAP" type blocks that affect "COLR".
      if (block->_block_type == IffId("IMAP") &&
	  block->_channel_id == IffId("COLR") &&
	  block->_enabled) {
	// Now save the block with the lowest ordinal.
	if (_block == (CLwoSurfaceBlock *)NULL) {
	  _block = block;

	} else if (block->_ordinal < _block->_ordinal) {
	  delete _block;
	  _block = block;

	} else {
	  delete block;
	}

      } else {
	delete block;
      }
    }      
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLwoSurface::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CLwoSurface::
~CLwoSurface() {
  if (_block != (CLwoSurfaceBlock *)NULL) {
    delete _block;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLwoSurface::apply_properties
//       Access: Public
//  Description: Applies the color, texture, etc. described by the
//               surface to the indicated egg primitive.
//
//               If the surface defines a smoothing angle,
//               smooth_angle may be updated to reflect it if the
//               angle is greater than that specified.
////////////////////////////////////////////////////////////////////
void CLwoSurface::
apply_properties(EggPrimitive *egg_prim, vector_PT_EggVertex &egg_vertices,
		 float &smooth_angle) {
  if (!_surface->_source.empty()) {
    // This surface is derived from another surface; apply that one
    // first.
    CLwoSurface *parent = _converter->get_surface(_surface->_source);
    if (parent != (CLwoSurface *)NULL && parent != this) {
      parent->apply_properties(egg_prim, egg_vertices, smooth_angle);
    }
  }

  // We treat color and transparency separately, because Lightwave
  // does, and this will allow us to treat inherited surfaces a little
  // more robustly.
  if ((_flags & F_color) != 0) {
    Colorf color(1.0, 1.0, 1.0, 1.0);
    if (egg_prim->has_color()) {
      color = egg_prim->get_color();
    }

    color[0] = _color[0];
    color[1] = _color[1];
    color[2] = _color[2];

    egg_prim->set_color(color);
  }

  if (check_texture()) {
    // Texture overrides the primitive's natural color.
    egg_prim->set_texture(_egg_texture);
    egg_prim->clear_color();

    // Assign UV's to the vertices.
    vector_PT_EggVertex::const_iterator vi;
    for (vi = egg_vertices.begin(); vi != egg_vertices.end(); ++vi) {
      EggVertex *egg_vertex = (*vi);
      egg_vertex->set_uv(get_uv(egg_vertex->get_pos3()));
    }
  }

  if ((_flags & F_transparency) != 0) {
    Colorf color(1.0, 1.0, 1.0, 1.0);
    if (egg_prim->has_color()) {
      color = egg_prim->get_color();
    }

    color[3] = 1.0 - _transparency;
    egg_prim->set_color(color);
  }

  if ((_flags & F_backface) != 0) {
    egg_prim->set_bface_flag(_backface);
  }

  if ((_flags & F_smooth_angle) != 0) {
    smooth_angle = max(smooth_angle, _smooth_angle);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLwoSurface::check_texture
//       Access: Public
//  Description: Checks whether the surface demands a texture or not.
//               Returns true if so, false otherwise.
//
//               If the surface demands a texture, this also sets up
//               _egg_texture and _compute_uvs as appropriate for the
//               texture.
////////////////////////////////////////////////////////////////////
bool CLwoSurface::
check_texture() {
  if (_checked_texture) {
    return (_egg_texture != (EggTexture *)NULL);
  }
  _checked_texture = true;
  _egg_texture = (EggTexture *)NULL;

  if (_block == (CLwoSurfaceBlock *)NULL) {
    // No texture.  Not even a shader block.
    return false;
  }

  int clip_index = _block->_clip_index;
  if (clip_index < 0) {
    // No image file associated with the texture.
    return false;
  }

  CLwoClip *clip = _converter->get_clip(clip_index);
  if (clip == (CLwoClip *)NULL) {
    nout << "No clip image with index " << clip_index << "\n";
    return false;
  }

  if (!clip->is_still_image()) {
    // Can't do anything with an animated image right now.
    return false;
  }

  Filename pathname = _converter->convert_texture_path(clip->_filename);

  _egg_texture = new EggTexture("clip" + format_string(clip_index), pathname);
  _has_uvs = true;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CLwoSurface::get_uv
//       Access: Public
//  Description: Computes or looks up the appropriate UV for the given
//               vertex.
////////////////////////////////////////////////////////////////////
LPoint2d CLwoSurface::
get_uv(const LPoint3d &pos) const {
  // For now, we always compute spherical UV's.

  // To compute the x position on the frame, we only need to consider
  // the angle of the vector about the Y axis.  Project the vector
  // into the XZ plane to do this.

  LVector2d xz(pos[0], pos[2]);

  // The x position is longitude: the angle about the Y axis.
  double x = 
    (atan2(xz[0], -xz[1]) / (2.0 * MathNumbers::pi) + 0.5) * _block->_w_repeat;

  // Now rotate the vector into the YZ plane, and the y position is
  // latitude: the angle about the X axis.
  LVector2d yz(pos[1], xz.length());
  double y = 
    (atan2(yz[0], yz[1]) / MathNumbers::pi + 0.5) * _block->_h_repeat;

  return LPoint2d(x, y);
}
