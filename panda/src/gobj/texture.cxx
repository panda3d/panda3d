// Filename: texture.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "pandabase.h"
#include "texture.h"
#include "config_gobj.h"
#include "texturePool.h"
#include "textureContext.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "string_utils.h"

#include <stddef.h>


////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle Texture::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: up_to_power_2
//  Description: Returns the smallest power of 2 greater than or equal
//               to value.
////////////////////////////////////////////////////////////////////
static int
up_to_power_2(int value) {
  int x = 1;
  while (x < value) {
    x = (x << 1);
  }
  return x;
}

////////////////////////////////////////////////////////////////////
//     Function: down_to_power_2
//  Description: Returns the largest power of 2 less than or equal
//               to value.
////////////////////////////////////////////////////////////////////
static int
down_to_power_2(int value) {
  int x = 1;
  while ((x << 1) <= value) {
    x = (x << 1);
  }
  return x;
}

////////////////////////////////////////////////////////////////////
//     Function: consider_rescale
//  Description: Scales the PNMImage according to the whims of the
//               Configrc file.
////////////////////////////////////////////////////////////////////
static void
consider_rescale(PNMImage &pnmimage, const string &name) {
  int new_x_size = pnmimage.get_x_size();
  int new_y_size = pnmimage.get_y_size();

  if (textures_down_power_2) {
    new_x_size = down_to_power_2(new_x_size);
    new_y_size = down_to_power_2(new_y_size);
  } else if (textures_up_power_2) {
    new_x_size = up_to_power_2(new_x_size);
    new_y_size = up_to_power_2(new_y_size);
  }

  if (textures_down_square) {
    new_x_size = new_y_size = min(new_x_size, new_y_size);
  } else if (textures_up_square) {
    new_x_size = new_y_size = max(new_x_size, new_y_size);
  }

  if (max_texture_dimension > 0) {
    new_x_size = min(new_x_size, max_texture_dimension);
    new_y_size = min(new_y_size, max_texture_dimension);
  }

  if (pnmimage.get_x_size() != new_x_size ||
      pnmimage.get_y_size() != new_y_size) {
    gobj_cat.info()
      << "Automatically rescaling " << name << " from "
      << pnmimage.get_x_size() << " by " << pnmimage.get_y_size() << " to "
      << new_x_size << " by " << new_y_size << "\n";

    PNMImage scaled(new_x_size, new_y_size, pnmimage.get_num_channels(),
                    pnmimage.get_maxval(), pnmimage.get_type());
    scaled.quick_filter_from(pnmimage);
    pnmimage = scaled;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
Texture::
Texture() : ImageBuffer() {
  _magfilter = FT_nearest;
  _minfilter = FT_nearest;
  _wrapu = WM_repeat;
  _wrapv = WM_repeat;
  _anisotropic_degree = 1;
  _keep_ram_image = false;
  _pbuffer = new PixelBuffer;
  // _has_requested_size = false;
  _all_dirty_flags = 0;
  memset(&_border_color,0,sizeof(Colorf));
}


////////////////////////////////////////////////////////////////////
//     Function: Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
Texture::
Texture(int xsize, int ysize, int components, int component_width, PixelBuffer::Type type, 
        PixelBuffer::Format format, bool bAllocateRAM) : ImageBuffer() {
  _magfilter = FT_nearest;
  _minfilter = FT_nearest;
  _wrapu = WM_repeat;
  _wrapv = WM_repeat;
  _anisotropic_degree = 1;
  _keep_ram_image = bAllocateRAM;
  _pbuffer = new PixelBuffer(xsize,ysize,components,component_width,type,format,bAllocateRAM);
  // _has_requested_size = false;
  _all_dirty_flags = 0;
  memset(&_border_color,0,sizeof(Colorf));
}

////////////////////////////////////////////////////////////////////
//     Function: Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
Texture::
~Texture() {
  unprepare();
}

////////////////////////////////////////////////////////////////////
//     Function: read
//       Access: Published
//  Description: Reads the texture from the indicated filename.
////////////////////////////////////////////////////////////////////
bool Texture::
read(const Filename &fullpath) {
  PNMImage image;

  if (!image.read(fullpath)) {
    gobj_cat.error()
      << "Texture::read() - couldn't read: " << fullpath << endl;
    return false;
  }

  if (!has_name()) {
    set_name(fullpath.get_basename_wo_extension());
  }
  if (!has_filename()) {
    set_filename(fullpath);
    clear_alpha_filename();
  }

  set_fullpath(fullpath);
  clear_alpha_fullpath();

  // Check to see if we need to scale it.
  consider_rescale(image, get_name());

  return load(image);
}

////////////////////////////////////////////////////////////////////
//     Function: read
//       Access: Published
//  Description: Combine a 3-component image with a grayscale image
//               to get a 4-component image
////////////////////////////////////////////////////////////////////
bool Texture::
read(const Filename &fullpath, const Filename &alpha_fullpath) {
  PNMImage image;
  if (!image.read(fullpath)) {
    gobj_cat.error()
      << "Texture::read() - couldn't read: " << fullpath << endl;
    return false;
  }

  PNMImage alpha_image;
  if (!alpha_image.read(alpha_fullpath)) {
    gobj_cat.error()
      << "Texture::read() - couldn't read: " << alpha_fullpath << endl;
    return false;
  }

  if (!has_name()) {
    set_name(fullpath.get_basename_wo_extension());
  }
  if (!has_filename()) {
    set_filename(fullpath);
    set_alpha_filename(alpha_fullpath);
  }

  set_fullpath(fullpath);
  set_alpha_fullpath(alpha_fullpath);

  consider_rescale(image, get_name());

  // The grayscale (alpha channel) image must be the same size as the
  // main image.
  if (image.get_x_size() != alpha_image.get_x_size() ||
      image.get_y_size() != alpha_image.get_y_size()) {
    gobj_cat.info()
      << "Automatically rescaling " << alpha_fullpath.get_basename()
      << " from " << alpha_image.get_x_size() << " by " 
      << alpha_image.get_y_size() << " to " << image.get_x_size()
      << " by " << image.get_y_size() << "\n";

    PNMImage scaled(image.get_x_size(), image.get_y_size(),
                    alpha_image.get_num_channels(),
                    alpha_image.get_maxval(), alpha_image.get_type());
    scaled.quick_filter_from(alpha_image);
    alpha_image = scaled;
  }

  // Make the original image a 4-component image
  image.add_alpha();
  if (alpha_image.has_alpha()) {
    for (int x = 0; x < image.get_x_size(); x++) {
      for (int y = 0; y < image.get_y_size(); y++) {
        image.set_alpha(x, y, alpha_image.get_alpha(x, y));
      }
    }
  } else {
    for (int x = 0; x < image.get_x_size(); x++) {
      for (int y = 0; y < image.get_y_size(); y++) {
        image.set_alpha(x, y, alpha_image.get_gray(x, y));
      }
    }
  }

  return load(image);
}

////////////////////////////////////////////////////////////////////
//     Function: write
//       Access: Published
//  Description: Writes the texture to the indicated filename.
////////////////////////////////////////////////////////////////////
bool Texture::
write(const Filename &name) const {
  nassertr(has_ram_image(), false);
  PNMImage pnmimage;
  if (!_pbuffer->store(pnmimage)) {
    return false;
  }

  if (!pnmimage.write(name)) {
    gobj_cat.error()
      << "Texture::write() - couldn't write: " << name << endl;
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: set_wrapu
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
set_wrapu(Texture::WrapMode wrap) {
  if (_wrapu != wrap) {
    mark_dirty(DF_wrap);
    _wrapu = wrap;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: set_wrapv
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
set_wrapv(Texture::WrapMode wrap) {
  if (_wrapv != wrap) {
    mark_dirty(DF_wrap); 
    _wrapv = wrap;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: set_minfilter
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
set_minfilter(Texture::FilterType filter) {
  if (_minfilter != filter) {
    if (is_mipmap(_minfilter) != is_mipmap(filter)) {
      mark_dirty(DF_filter | DF_mipmap);
    } else {
      mark_dirty(DF_filter);
    }
    _minfilter = filter;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: set_magfilter
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
set_magfilter(Texture::FilterType filter) {
  if (_magfilter != filter) {
    mark_dirty(DF_filter);
    _magfilter = filter;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: set_anisotropic_degree
//       Access: Published
//  Description: Specifies the level of anisotropic filtering to apply
//               to the texture.  Normally, this is 1, to indicate
//               anisotropic filtering is disabled.  This may be set
//               to a number higher than one to enable anisotropic
//               filtering, if the rendering backend supports this.
////////////////////////////////////////////////////////////////////
void Texture::
set_anisotropic_degree(int anisotropic_degree) {
  if (_anisotropic_degree != anisotropic_degree) {
    mark_dirty(DF_filter);
    _anisotropic_degree = anisotropic_degree;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: set_border_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
set_border_color(const Colorf &color) {
   memcpy(&_border_color,&color,sizeof(Colorf));
}

////////////////////////////////////////////////////////////////////
//     Function: load
//       Access: Public
//  Description: Creates the texture from the already-read PNMImage.
////////////////////////////////////////////////////////////////////
bool Texture::
load(const PNMImage &pnmimage) {
  if (!_pbuffer->load(pnmimage))
    return false;

  mark_dirty(DF_image);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: store
//       Access: Public
//  Description: Saves the texture to the indicated PNMImage, but does
//               not write it to disk.
////////////////////////////////////////////////////////////////////
bool Texture::
store(PNMImage &pnmimage) const {
  return _pbuffer->store( pnmimage );
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::is_mipmap
//       Access: Public, Static
//  Description: Returns true if the indicated filter type requires
//               the use of mipmaps, or false if it does not.
////////////////////////////////////////////////////////////////////
bool Texture::
is_mipmap(FilterType type) {
  switch (type) {
  case FT_nearest_mipmap_nearest:
  case FT_linear_mipmap_nearest:
  case FT_nearest_mipmap_linear:
  case FT_linear_mipmap_linear:
    return true;
    
  default:
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: prepare
//       Access: Public
//  Description: Creates a context for the texture on the particular
//               GSG, if it does not already exist.  Returns the new
//               (or old) TextureContext.
////////////////////////////////////////////////////////////////////
TextureContext *Texture::
prepare(GraphicsStateGuardianBase *gsg) {
  Contexts::const_iterator ci;
  ci = _contexts.find(gsg);
  if (ci != _contexts.end()) {
    return (*ci).second;
  }

  TextureContext *tc = gsg->prepare_texture(this);
  _contexts[gsg] = tc;

  // Now that we have a new TextureContext with zero dirty flags, our
  // intersection of all dirty flags must be zero.  This doesn't mean
  // that some other contexts aren't still dirty, but at least one
  // context isn't.
  _all_dirty_flags = 0;

  if (!keep_texture_ram && !_keep_ram_image) {
    // Once we have prepared the texture, we can generally safely
    // remove the pixels from main RAM.  The GSG is now responsible
    // for remembering what it looks like.

    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "Dumping RAM for texture " << get_name() << "\n";
    }
    _pbuffer->_image.clear();
  }

  return tc;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::unprepare
//       Access: Public
//  Description: Frees the context allocated on all GSG's for which
//               the texture has been declared.
////////////////////////////////////////////////////////////////////
void Texture::
unprepare() {
  // We have to traverse a copy of the _contexts list, because the GSG
  // will call clear_gsg() in response to each release_texture(), and
  // we don't want to be modifying the _contexts list while we're
  // traversing it.
  Contexts temp = _contexts;

  Contexts::const_iterator ci;
  for (ci = temp.begin(); ci != temp.end(); ++ci) {
    GraphicsStateGuardianBase *gsg = (*ci).first;
    TextureContext *tc = (*ci).second;
    gsg->release_texture(tc);
  }

  // Now that we've called release_texture() on every known GSG, the
  // _contexts list should have completely emptied itself.
  nassertv(_contexts.empty());
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::unprepare
//       Access: Public
//  Description: Frees the texture context only on the indicated GSG,
//               if it exists there.
////////////////////////////////////////////////////////////////////
void Texture::
unprepare(GraphicsStateGuardianBase *gsg) {
  Contexts::iterator ci;
  ci = _contexts.find(gsg);
  if (ci != _contexts.end()) {
    TextureContext *tc = (*ci).second;
    gsg->release_texture(tc);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::clear_gsg
//       Access: Public
//  Description: Removes the indicated GSG from the Texture's known
//               GSG's, without actually releasing the texture on that
//               GSG.  This is intended to be called only from
//               GSG::release_texture(); it should never be called by
//               user code.
////////////////////////////////////////////////////////////////////
void Texture::
clear_gsg(GraphicsStateGuardianBase *gsg) {
  Contexts::iterator ci;
  ci = _contexts.find(gsg);
  if (ci != _contexts.end()) {
    _contexts.erase(ci);
  } else {
    // If this assertion fails, clear_gsg() was called on a GSG which
    // the texture didn't know about.
    nassertv(false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_ram_image
//       Access: Public
//  Description: Returns the PixelBuffer associated with the texture.
//               If the PixelBuffer does not currently have an
//               associated RAM image, and the texture was generated
//               by loading an image from a disk file (the most common
//               case), this forces the reload of the same texture.
//               This can happen if keep_texture_ram is configured to
//               false, and we have previously prepared this texture
//               with a GSG.
//
//               Note that it is not correct to call has_ram_image()
//               first to test whether this function will fail.  A
//               false return value from has_ram_image() indicates
//               only that get_ram_image() may need to reload the
//               texture from disk, which it will do automatically.
//
//               On the other hand, it is possible that the texture
//               cannot be found on disk or is otherwise unavailable.
//               If that happens, this function returns NULL.  There
//               is no way to predict whether get_ram_image() will
//               return NULL without calling it first.
////////////////////////////////////////////////////////////////////
PixelBuffer *Texture::
get_ram_image() {
  if (!has_ram_image() && has_filename()) {
    // Now we have to reload the texture image.
    gobj_cat.info()
      << "Reloading texture " << get_name() << "\n";
    
    if (has_alpha_fullpath()) {
      read(get_fullpath(), get_alpha_fullpath());
    } else {
      read(get_fullpath());
    }
  }

  if (has_ram_image()) {
    return _pbuffer;
  } else {
    return (PixelBuffer *)NULL;
  }
}

void Texture::copy(GraphicsStateGuardianBase *gsg, const DisplayRegion *dr) {
  gsg->copy_texture(prepare(gsg), dr);
}

void Texture::copy(GraphicsStateGuardianBase *gsg, const DisplayRegion *dr,
                   const RenderBuffer &rb) {
  gsg->copy_texture(prepare(gsg), dr, rb);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::mark_dirty
//       Access: Public
//  Description: Sets the indicated dirty bits on for all texture
//               contexts that share this Texture.  Does not change
//               the bits that are not on.  This presumably will
//               inform the GSG that the texture properties have
//               changed.  See also TextureContext::mark_dirty().
//
//               Normally, this does not need to be called directly;
//               changing the properties on the texture will
//               automatically call this.  However, if you fiddle with
//               the texture image directly, for instance by meddling
//               with the _pbuffer member, you may need to explicitly
//               call mark_dirty(Texture::DF_image).
////////////////////////////////////////////////////////////////////
void Texture::
mark_dirty(int flags_to_set) {
  if ((_all_dirty_flags & flags_to_set) == flags_to_set) {
    // If all the texture contexts already share these bits, no need
    // to do anything else.
    return;
  }

  // Otherwise, iterate through the contexts and mark them all dirty.
  Contexts::iterator ci;
  for (ci = _contexts.begin(); ci != _contexts.end(); ++ci) {
    (*ci).second->mark_dirty(flags_to_set);
  }

  _all_dirty_flags |= flags_to_set;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::string_wrap_mode
//       Access: Public
//  Description: Returns the WrapMode value associated with the given
//               string representation, or WM_invalid if the string
//               does not match any known WrapMode value.
////////////////////////////////////////////////////////////////////
Texture::WrapMode Texture::
string_wrap_mode(const string &string) {
  if (cmp_nocase_uh(string, "repeat") == 0) {
    return WM_repeat;
  } else if (cmp_nocase_uh(string, "clamp") == 0) {
    return WM_clamp;
  } else if (cmp_nocase_uh(string, "mirror") == 0) {
    return WM_clamp;
  } else if (cmp_nocase_uh(string, "mirror_once") == 0) {
    return WM_clamp;
  } else if (cmp_nocase_uh(string, "border_color") == 0) {
    return WM_border_color;
  } else {
    return WM_invalid;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::string_filter_type
//       Access: Public
//  Description: Returns the FilterType value associated with the given
//               string representation, or FT_invalid if the string
//               does not match any known FilterType value.
////////////////////////////////////////////////////////////////////
Texture::FilterType Texture::
string_filter_type(const string &string) {
  if (cmp_nocase_uh(string, "nearest") == 0) {
    return FT_nearest;
  } else if (cmp_nocase_uh(string, "linear") == 0) {
    return FT_linear;
  } else if (cmp_nocase_uh(string, "nearest_mipmap_nearest") == 0) {
    return FT_nearest_mipmap_nearest;
  } else if (cmp_nocase_uh(string, "linear_mipmap_nearest") == 0) {
    return FT_linear_mipmap_nearest;
  } else if (cmp_nocase_uh(string, "nearest_mipmap_linear") == 0) {
    return FT_nearest_mipmap_linear;
  } else if (cmp_nocase_uh(string, "linear_mipmap_linear") == 0) {
    return FT_linear_mipmap_linear;
  } else if (cmp_nocase_uh(string, "mipmap") == 0) {
    return FT_linear_mipmap_linear;

  } else {
    return FT_invalid;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::register_with_read_factory
//       Access: Public, Static
//  Description: Factory method to generate a Texture object
////////////////////////////////////////////////////////////////////
void Texture::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_Texture);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::make_Texture
//       Access: Protected
//  Description: Factory method to generate a Texture object
////////////////////////////////////////////////////////////////////
TypedWritable* Texture::
make_Texture(const FactoryParams &params) {
  //The process of making a texture is slightly
  //different than making other Writable objects.
  //That is because all creation of Textures should
  //be done through calls to TexturePool, which ensures
  //that any loads of the same Texture, refer to the
  //same memory
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);

  string name;
  Filename filename, alpha_filename;

  name = scan.get_string();
  filename = scan.get_string();
  alpha_filename = scan.get_string();

  PT(Texture) me;

  if (filename.empty()) {
    // This texture has no filename; since we don't have an image to
    // load, we can't actually create the texture.
    gobj_cat.info()
      << "Cannot create texture '" << name << "' with no filename.\n";

  } else {
    // This texture does have a filename, so try to load it from disk.
    if (alpha_filename.empty()) {
      me = TexturePool::load_texture(filename);
    } else {
      me = TexturePool::load_texture(filename, alpha_filename);
    }
  }

  if (me == (Texture *)NULL) {
    // Oops, we couldn't load the texture; we'll just return NULL.
    // But we do need a dummy texture to read in and ignore all of the
    // attributes.
    PT(Texture) dummy = new Texture;
    dummy->fillin(scan, manager);

  } else {
    me->set_name(name);
    me->fillin(scan, manager);
  }
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void Texture::
fillin(DatagramIterator &scan, BamReader *manager) {
  //We don't want to call ImageBuffer::fillin, like we
  //would normally, since due to needing to know the name
  //of the Texture before creating it, we have already read
  //that name in.  This is something of a problem as it forces
  //Texture to know how the parent write_datagram works.  And
  //makes the assumption that the only data being written is
  //the name

  _wrapu = (enum WrapMode) scan.get_uint8();
  _wrapv = (enum WrapMode) scan.get_uint8();
  _minfilter = (enum FilterType) scan.get_uint8();
  _magfilter = (enum FilterType) scan.get_uint8();
  _anisotropic_degree = scan.get_int16();

  if (scan.get_remaining_size() > 0) {
    bool has_pbuffer = scan.get_bool();
    if (has_pbuffer) {
      PixelBuffer::Format format = (PixelBuffer::Format)scan.get_uint8();
      int num_components = -1;
      if (scan.get_remaining_size() > 0) {
        num_components = scan.get_uint8();
      }

      if (_pbuffer != (PixelBuffer *)NULL) {
        if (num_components == _pbuffer->get_num_components()) {
          // Only reset the format if the number of components hasn't
          // changed.
          _pbuffer->set_format(format);
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void Texture::
write_datagram(BamWriter *manager, Datagram &me) {
  ImageBuffer::write_datagram(manager, me);
  me.add_uint8(_wrapu);
  me.add_uint8(_wrapv);
  me.add_uint8(_minfilter);
  me.add_uint8(_magfilter);
  me.add_int16(_anisotropic_degree);

  // We also need to write out the pixel buffer's format, even though
  // that's not stored as part of the texture structure.
  bool has_pbuffer = (_pbuffer != (PixelBuffer *)NULL);
  me.add_bool(has_pbuffer);
  if (has_pbuffer) {
    me.add_uint8(_pbuffer->get_format());
    me.add_uint8(_pbuffer->get_num_components());
  }
}

