// Filename: sourceEgg.cxx
// Created by:  drose (02Sep99)
// 
////////////////////////////////////////////////////////////////////

#include "sourceEgg.h"
#include "pTexture.h"
#include "eggPalettize.h"
#include "string_utils.h"
#include "palette.h"

#include <eggAttributes.h>
#include <eggVertex.h>
#include <eggNurbsSurface.h>
#include <eggPrimitive.h>
#include <eggTextureCollection.h>

TypeHandle SourceEgg::_type_handle;

SourceEgg::TextureRef::
TextureRef(PTexture *texture, bool repeats, bool alpha) :
  _texture(texture),
  _repeats(repeats),
  _alpha(alpha) 
{
  _eggtex = NULL;
}

SourceEgg::
SourceEgg() {
}

SourceEgg::TextureRef &SourceEgg::
add_texture(PTexture *texture, bool repeats, bool alpha) {
  _texrefs.push_back(TextureRef(texture, repeats, alpha));
  return _texrefs.back();
}

void SourceEgg::
get_textures(AttribFile &af, EggPalettize *prog) {
  _texrefs.clear();

  EggTextureCollection tc;
  tc.find_used_textures(this);

  EggTextureCollection::iterator ti;
  for (ti = tc.begin(); ti != tc.end(); ++ti) {
    EggTexture *eggtex = (*ti);
    string name = eggtex->get_basename();
    
    PTexture *texture = af.get_texture(name);
    texture->add_filename(*eggtex);

    if (prog->_dont_palettize) {
      // If the user specified -x, it means to omit all textures
      // processed in this run, forever.
      texture->set_omit(PTexture::OR_cmdline);
    } else {
      // Or until we next see it without -x.
      if (texture->get_omit() == PTexture::OR_cmdline) {
	texture->set_omit(PTexture::OR_none);
      }
    }

    bool repeats = 
      eggtex->get_wrap_mode() == EggTexture::WM_repeat ||
      eggtex->get_wrap_u() == EggTexture::WM_repeat ||
      eggtex->get_wrap_v() == EggTexture::WM_repeat;

    bool repeat_unspecified = 
      eggtex->get_wrap_mode() == EggTexture::WM_unspecified &&
      eggtex->get_wrap_u() == EggTexture::WM_unspecified &&
      eggtex->get_wrap_v() == EggTexture::WM_unspecified;

    bool alpha = true; //eggtex->uses_alpha();

    // Check the range of UV's actually used within the egg file.
    bool any_uvs = false;
    TexCoordd min_uv, max_uv;
    get_uv_range(this, eggtex, any_uvs, min_uv, max_uv);

    // Now we need to apply the texture matrix, if there is one, to
    // our bounding box UV's.
    
    if (eggtex->has_transform()) {
      // Transforming a bounding box by a matrix requires transforming
      // all four corners.

      TexCoordd a(min_uv[0], min_uv[1]);
      TexCoordd b(min_uv[0], max_uv[1]);
      TexCoordd c(max_uv[0], max_uv[1]);
      TexCoordd d(max_uv[0], min_uv[1]);

      LMatrix3d transform = eggtex->get_transform();

      a = a * transform;
      b = b * transform;
      c = c * transform;
      d = d * transform;

      // Now boil down these four corners into a new bounding box.

      min_uv.set(min(min(a[0], b[0]), min(c[0], d[0])),
		 min(min(a[1], b[1]), min(c[1], d[1])));
      max_uv.set(max(max(a[0], b[0]), max(c[0], d[0])),
		 max(max(a[1], b[1]), max(c[1], d[1])));
    }

    bool truly_repeats =
      (max_uv[0] > 1.0 + prog->_fuzz_factor || 
       min_uv[0] < 0.0 - prog->_fuzz_factor ||
       max_uv[1] > 1.0 + prog->_fuzz_factor || 
       min_uv[1] < 0.0 - prog->_fuzz_factor);
    
    if (repeat_unspecified) {
      // If the egg file didn't give us any advice regarding
      // repeating, we can go entirely based on what we saw in the
      // UV's.
      repeats = truly_repeats;

    } else {
      if (repeats && !truly_repeats) {
	// The egg file specified that the texture repeats, but we
	// discovered that it doesn't really.  Quietly mark it so.
	if (!prog->_dont_override_repeats) {
	  repeats = false;
	}

      } else if (!repeats && truly_repeats) {
	// The egg file specified that the texture doesn't repeat, but
	// its UV's were outside the normal range!  That's almost
	// certainly a modeling error.
	
	// We won't trouble the user with this kind of spammy message,
	// though.
	/*
	nout << "Warning: texture " << texture->get_name() << " (tref " 
	     << eggtex->name << ") marked clamped, but UV's range from ("
	     << min_uv << ") to (" << max_uv << ")\n";
	     */

      } else if (repeats && truly_repeats) {
	// Well, it really does repeat.  Or does it?
	if (fabs(max_uv[0] - min_uv[0]) <= 1.0 + prog->_fuzz_factor &&
	    fabs(max_uv[1] - min_uv[1]) <= 1.0 + prog->_fuzz_factor) {
	  // It really shouldn't!  The UV's fit totally within a unit
	  // square, just not the square (0,0) - (1,1).  This is a
	  // modeling problem; inform the user.
	  nout << "Warning: texture " << texture->get_name()
	       << " cannot be clamped because UV's range from ("
	       << min_uv << ") to (" << max_uv << ")\n";
	}
      }
    }
    
    TextureRef &texref = add_texture(texture, repeats, alpha);
    texref._eggtex = eggtex;
  }
}

// Updates each PTexture with the flags stored in the various egg
// files.  Also marks textures as used.
void SourceEgg::
mark_texture_flags() {
  TexRefs::iterator ti;
  for (ti = _texrefs.begin(); ti != _texrefs.end(); ++ti) {
    PTexture *texture = (*ti)._texture;
    texture->set_unused(false);
    if ((*ti)._alpha) {
      texture->set_uses_alpha(true);
    }
    if ((*ti)._repeats) {
      texture->set_omit(PTexture::OR_repeats);
    }
  }
}

// Updates the egg file to point to the new palettes.
void SourceEgg::
update_trefs() {
  TexRefs::iterator ti;
  for (ti = _texrefs.begin(); ti != _texrefs.end(); ++ti) {
    PTexture *texture = (*ti)._texture;
    EggTexture *eggtex = (*ti)._eggtex;

    if (eggtex != NULL) {
      // Make the alpha mode explicit if it isn't already.

      /*
      if (eggtex->get_alpha_mode == EggTexture::AM_unspecified) {
	eggtex->set_alpha = eggtex->UsesAlpha() ? 
	  EggTexture::AM_on : EggTexture::AM_off;
      }
      */

      if (!texture->is_packed() || 
	  texture->get_omit() != PTexture::OR_none) {
	// This texture wasn't palettized, so just rename the
	// reference to the new one.
	eggtex->set_fullpath(texture->get_filename());

      } else {
	// This texture was palettized, so redirect the tref to point
	// within the palette.
	Palette *palette = texture->get_palette();
	
	eggtex->set_fullpath(palette->get_filename());
	
	// Set the texture attributes to be uniform across all palettes.
	eggtex->set_minfilter(EggTexture::FT_mipmap_trilinear);
	eggtex->set_magfilter(EggTexture::FT_bilinear);
	eggtex->set_format(EggTexture::F_rgba8);
	eggtex->set_wrap_mode(EggTexture::WM_clamp);
	eggtex->set_wrap_u(EggTexture::WM_clamp);
	eggtex->set_wrap_v(EggTexture::WM_clamp);
	
	// Build a matrix that will scale the UV's to their new place
	// on the palette.
	int left, top, xsize, ysize, margin;
	texture->get_packed_location(left, top);
	texture->get_packed_size(xsize, ysize, margin);
	
	// Shrink the box to be within the margins.
	top += margin;
	left += margin;
	xsize -= margin*2;
	ysize -= margin*2;
	
	// Now determine the relative size and position within the
	// palette.
	int bottom = top + ysize;
	int palx, paly;
	palette->get_size(palx, paly);
	LVecBase2d t((double)left / (double)palx,
		     (double)(paly - bottom) / (double)paly);
	
	LVecBase2d s((double)xsize / (double)palx, 
		     (double)ysize / (double)paly);
	
	LMatrix3d texmat
	  (s[0],  0.0,  0.0,
	    0.0, s[1],  0.0,
	   t[0], t[1],  1.0);
	
	// Do we already have a texture matrix?  If so, compose them.
	if (eggtex->has_transform()) {
	  eggtex->set_transform(eggtex->get_transform() * texmat);
	} else {
	  // Otherwise, just store it.
	  eggtex->set_transform(texmat);
	}
      }
    }
  }
}

// Returns true if any of the textures referenced by the egg file have
// been adjusted this pass, implying that the egg file will have to be
// re-run through egg-palettize, and/or re-pfb'ed.
bool SourceEgg::
needs_rebuild(bool force_redo_all,
	      bool eggs_include_images) const {
  //  if (!_wrote_egg) {
    TexRefs::const_iterator ti;
    for (ti = _texrefs.begin(); ti != _texrefs.end(); ++ti) {
      bool dirty = 
	eggs_include_images ? 
	(*ti)._texture->needs_refresh() :
	(*ti)._texture->packing_changed();
      if (force_redo_all || dirty) {
	return true;
      }
    }
    //  }

  return false;
}

void SourceEgg::
write_pi(ostream &out) const {
  out << "egg " << get_egg_filename() << "\n";
  TexRefs::const_iterator ti;
  for (ti = _texrefs.begin(); ti != _texrefs.end(); ++ti) {
    out << "  " << (*ti)._texture->get_name();
    if ((*ti)._repeats) {
      out << " repeats";
    }
    if ((*ti)._alpha) {
      out << " alpha";
    }
    out << "\n";
  }
}

void SourceEgg::
get_uv_range(EggGroupNode *group, EggTexture *tref,
	     bool &any_uvs, TexCoordd &min_uv, TexCoordd &max_uv) {
  EggGroupNode::iterator ci;

  for (ci = group->begin(); ci != group->end(); ci++) {
    EggNode *child = (*ci);
    if (child->is_of_type(EggNurbsSurface::get_class_type())) {
      EggNurbsSurface *nurbs = DCAST(EggNurbsSurface, child);
      if (nurbs->has_texture() && nurbs->get_texture() == tref) {
	// Here's a NURBS surface that references the texture.  Unlike
	// other kinds of geometries, NURBS don't store UV's; they're
	// implicit in the surface.  NURBS UV's will always run in the
	// range (0,0) - (1,1).
	if (any_uvs) {
	  min_uv.set(min(min_uv[0], 0.0), min(min_uv[1], 0.0));
	  max_uv.set(max(max_uv[0], 1.0), max(max_uv[1], 1.0));
	} else {
	  min_uv.set(0.0, 0.0);
	  max_uv.set(1.0, 1.0);
	  any_uvs = true;
	}
      }

    } else if (child->is_of_type(EggPrimitive::get_class_type())) {
      EggPrimitive *geom = DCAST(EggPrimitive, child);
      if (geom->has_texture() && geom->get_texture() == tref) {
	// Here's a piece of geometry that references this texture.
	// Walk through its vertices at get its UV's.
	EggPrimitive::iterator pi;
	for (pi = geom->begin(); pi != geom->end(); ++pi) {
	  EggVertex *vtx = (*pi);
	  if (vtx->has_uv()) {
	    const TexCoordd &uv = vtx->get_uv();
	    if (any_uvs) {
	      min_uv.set(min(min_uv[0], uv[0]), min(min_uv[1], uv[1]));
	      max_uv.set(max(max_uv[0], uv[0]), max(max_uv[1], uv[1]));
	    } else {
	      // The first UV.
	      min_uv = uv;
	      max_uv = uv;
	      any_uvs = true;
	    }
	  }
	}
      }

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      EggGroupNode *cg = DCAST(EggGroupNode, child);
      get_uv_range(cg, tref, any_uvs, min_uv, max_uv);
    }
  }
}	
