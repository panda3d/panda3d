// Filename: cLwoSurface.cxx
// Created by:  drose (25Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "cLwoSurface.h"
#include "lwoToEggConverter.h"

#include <lwoSurfaceColor.h>
#include <lwoSurfaceParameter.h>
#include <lwoSurfaceSmoothingAngle.h>
#include <lwoSurfaceSidedness.h>
#include <eggPrimitive.h>


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
    }      
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
apply_properties(EggPrimitive *egg_prim, float &smooth_angle) {
  if (!_surface->_source.empty()) {
    // This surface is derived from another surface; apply that one
    // first.
    CLwoSurface *parent = _converter->get_surface(_surface->_source);
    if (parent != (CLwoSurface *)NULL && parent != this) {
      parent->apply_properties(egg_prim, smooth_angle);
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
