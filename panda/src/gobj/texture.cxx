// Filename: texture.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
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
#include "preparedGraphicsObjects.h"

#include <stddef.h>


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
//     Function: consider_downgrade
//  Description: Reduces the number of channels in the texture, if
//               necessary, according to num_channels.
////////////////////////////////////////////////////////////////////
static void
consider_downgrade(PNMImage &pnmimage, int num_channels, 
                   const string &name) {
  if (num_channels != 0 && num_channels < pnmimage.get_num_channels()) {
    // One special case: we can't reduce from 3 to 2 components, since
    // that would require adding an alpha channel.
    if (pnmimage.get_num_channels() == 3 && num_channels == 2) {
      return;
    }

    gobj_cat.info()
      << "Downgrading " << name << " from " << pnmimage.get_num_channels()
      << " components to " << num_channels << ".\n";
    pnmimage.set_num_channels(num_channels);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
Texture::
Texture() : ImageBuffer() {
  _magfilter = FT_linear;
  _minfilter = FT_linear;
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
//     Function: Texture::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
Texture::
Texture(int xsize, int ysize, int components, int component_width, PixelBuffer::Type type, 
        PixelBuffer::Format format, bool bAllocateRAM) : ImageBuffer() {
  _magfilter = FT_linear;
  _minfilter = FT_linear;
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
//     Function: Texture::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
Texture::
~Texture() {
  release_all();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::read
//       Access: Published
//  Description: Reads the texture from the indicated filename.  If
//               num_channels is not 0, it specifies the number of
//               components to downgrade the image to if it is greater
//               than this number.
////////////////////////////////////////////////////////////////////
bool Texture::
read(const Filename &fullpath, int primary_file_num_channels) {
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
  consider_downgrade(image, primary_file_num_channels, get_name());

  _primary_file_num_channels = image.get_num_channels();
  _alpha_file_channel = 0;

  return load(image);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::read
//       Access: Published
//  Description: Combine a 3-component image with a grayscale image
//               to get a 4-component image
////////////////////////////////////////////////////////////////////
bool Texture::
read(const Filename &fullpath, const Filename &alpha_fullpath,
     int primary_file_num_channels, int alpha_file_channel) {
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

  consider_downgrade(image, primary_file_num_channels, get_name());

  _primary_file_num_channels = image.get_num_channels();

  // Make the original image a 4-component image by taking the
  // grayscale value from the second image.
  image.add_alpha();

  if (alpha_file_channel == 4 || 
      (alpha_file_channel == 2 && alpha_image.get_num_channels() == 2)) {
    // Use the alpha channel.
    for (int x = 0; x < image.get_x_size(); x++) {
      for (int y = 0; y < image.get_y_size(); y++) {
        image.set_alpha(x, y, alpha_image.get_alpha(x, y));
      }
    }
    _alpha_file_channel = alpha_image.get_num_channels();

  } else if (alpha_file_channel >= 1 && alpha_file_channel <= 3 &&
             alpha_image.get_num_channels() >= 3) {
    // Use the appropriate red, green, or blue channel.
    for (int x = 0; x < image.get_x_size(); x++) {
      for (int y = 0; y < image.get_y_size(); y++) {
        image.set_alpha(x, y, alpha_image.get_channel_val(x, y, alpha_file_channel - 1));
      }
    }
    _alpha_file_channel = alpha_file_channel;

  } else {
    // Use the grayscale channel.
    for (int x = 0; x < image.get_x_size(); x++) {
      for (int y = 0; y < image.get_y_size(); y++) {
        image.set_alpha(x, y, alpha_image.get_gray(x, y));
      }
    }
    _alpha_file_channel = 0;
  }

  return load(image);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::write
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
//     Function: Texture::set_wrapu
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
//     Function: Texture::set_wrapv
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
//     Function: Texture::set_minfilter
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
//     Function: Texture::set_magfilter
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
//     Function: Texture::set_anisotropic_degree
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
//     Function: Texture::set_border_color
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::
set_border_color(const Colorf &color) {
   memcpy(&_border_color,&color,sizeof(Colorf));
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::prepare
//       Access: Published
//  Description: Indicates that the texture should be enqueued to be
//               prepared in the indicated prepared_objects at the
//               beginning of the next frame.  This will ensure the
//               texture is already loaded into texture memory if it
//               is expected to be rendered soon.
//
//               Use this function instead of prepare_now() to preload
//               textures from a user interface standpoint.
////////////////////////////////////////////////////////////////////
void Texture::
prepare(PreparedGraphicsObjects *prepared_objects) {
  prepared_objects->enqueue_texture(this);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::load
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
//     Function: Texture::store
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
//     Function: Texture::prepare_now
//       Access: Public
//  Description: Creates a context for the texture on the particular
//               GSG, if it does not already exist.  Returns the new
//               (or old) TextureContext.  This assumes that the
//               GraphicsStateGuardian is the currently active
//               rendering context and that it is ready to accept new
//               textures.  If this is not necessarily the case, you
//               should use prepare_later() instead.
//
//               Normally, this is not called directly except by the
//               GraphicsStateGuardian; a texture does not need to be
//               explicitly prepared by the user before it may be
//               rendered.
////////////////////////////////////////////////////////////////////
TextureContext *Texture::
prepare_now(PreparedGraphicsObjects *prepared_objects, 
            GraphicsStateGuardianBase *gsg) {
  Contexts::const_iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    return (*ci).second;
  }

  TextureContext *tc = prepared_objects->prepare_texture_now(this, gsg);
  if (tc != (TextureContext *)NULL) {
    _contexts[prepared_objects] = tc;

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
  }
  return tc;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::release
//       Access: Public
//  Description: Frees the texture context only on the indicated object,
//               if it exists there.  Returns true if it was released,
//               false if it had not been prepared.
////////////////////////////////////////////////////////////////////
bool Texture::
release(PreparedGraphicsObjects *prepared_objects) {
  Contexts::iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    TextureContext *tc = (*ci).second;
    prepared_objects->release_texture(tc);
    return true;
  }

  // Maybe it wasn't prepared yet, but it's about to be.
  return prepared_objects->dequeue_texture(this);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::release_all
//       Access: Public
//  Description: Frees the context allocated on all objects for which
//               the texture has been declared.  Returns the number of
//               contexts which have been freed.
////////////////////////////////////////////////////////////////////
int Texture::
release_all() {
  // We have to traverse a copy of the _contexts list, because the
  // PreparedGraphicsObjects object will call clear_prepared() in response
  // to each release_texture(), and we don't want to be modifying the
  // _contexts list while we're traversing it.
  Contexts temp = _contexts;
  int num_freed = (int)_contexts.size();

  Contexts::const_iterator ci;
  for (ci = temp.begin(); ci != temp.end(); ++ci) {
    PreparedGraphicsObjects *prepared_objects = (*ci).first;
    TextureContext *tc = (*ci).second;
    prepared_objects->release_texture(tc);
  }

  // Now that we've called release_texture() on every known context,
  // the _contexts list should have completely emptied itself.
  nassertr(_contexts.empty(), num_freed);

  return num_freed;
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
//     Function: Texture::clear_prepared
//       Access: Private
//  Description: Removes the indicated PreparedGraphicsObjects table
//               from the Texture's table, without actually releasing
//               the texture.  This is intended to be called only from
//               PreparedGraphicsObjects::release_texture(); it should
//               never be called by user code.
////////////////////////////////////////////////////////////////////
void Texture::
clear_prepared(PreparedGraphicsObjects *prepared_objects) {
  Contexts::iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    _contexts.erase(ci);
  } else {
    // If this assertion fails, clear_prepared() was given a
    // prepared_objects which the texture didn't know about.
    nassertv(false);
  }
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
  bool has_rawdata = false;

  parse_params(params, scan, manager);

  // Get the properties written by ImageBuffer::write_datagram().
  string name = scan.get_string();
  Filename filename = scan.get_string();
  Filename alpha_filename = scan.get_string();

  int primary_file_num_channels = 0;  
  int alpha_file_channel = 0;  

  if (manager->get_file_minor_ver() == 2) {
    // We temporarily had a version that stored the number of channels
    // here.
    primary_file_num_channels = scan.get_uint8();

  } else if (manager->get_file_minor_ver() >= 3) {
    primary_file_num_channels = scan.get_uint8();
    alpha_file_channel = scan.get_uint8();
  }

  // from minor version 5, read the rawdata mode, else carry on
  if (manager->get_file_minor_ver() >= 5)
    has_rawdata = scan.get_bool();

  Texture *me = NULL;
  if (has_rawdata) {
    // then create a Texture and don't load from the file
    me = new Texture;

  } else {
    if (filename.empty()) {
      // This texture has no filename; since we don't have an image to
      // load, we can't actually create the texture.
      gobj_cat.info()
        << "Cannot create texture '" << name << "' with no filename.\n";

    } else {
      // This texture does have a filename, so try to load it from disk.
      if (alpha_filename.empty()) {
        me = TexturePool::load_texture(filename, primary_file_num_channels);
      } else {
        me = TexturePool::load_texture(filename, alpha_filename, 
                                       primary_file_num_channels, alpha_file_channel);
      }
    }
  }

  if (me == (Texture *)NULL) {
    // Oops, we couldn't load the texture; we'll just return NULL.
    // But we do need a dummy texture to read in and ignore all of the
    // attributes.
    PT(Texture) dummy = new Texture;
    dummy->fillin(scan, manager, has_rawdata);

  } else {
    me->set_name(name);
    me->fillin(scan, manager, has_rawdata);

    /*
    cerr << "_xsize = " << me->_pbuffer->get_xsize() << "\n";
    cerr << "_ysize = " << me->_pbuffer->get_ysize() << "\n";
    cerr << "_xorg = " << me->_pbuffer->get_xorg() << "\n";
    cerr << "_yorg = " << me->_pbuffer->get_xorg() << "\n";
    cerr << "_components = " << me->_pbuffer->get_num_components() << "\n";
    */
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
fillin(DatagramIterator &scan, BamReader *manager, bool has_rawdata) {
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

  bool has_pbuffer = scan.get_bool();
  if (has_pbuffer) {
    PixelBuffer::Format format = (PixelBuffer::Format)scan.get_uint8();
    int num_channels = -1;
    num_channels = scan.get_uint8();

    if (_pbuffer != (PixelBuffer *)NULL) {
      if (num_channels == _pbuffer->get_num_components()) {
        // Only reset the format if the number of components hasn't
        // changed, since if the number of components has changed our
        // texture no longer matches what it was when the bam was
        // written.
        _pbuffer->set_format(format);
      }

      if (has_rawdata) {
        // In the rawdata case, we must always set the format.
        _pbuffer->set_format(format);
        _pbuffer->set_xsize(scan.get_int32());
        _pbuffer->set_ysize(scan.get_int32());
        _pbuffer->set_xorg(scan.get_int32());
        _pbuffer->set_yorg(scan.get_int32());
        _pbuffer->set_border(scan.get_uint8());
        _pbuffer->set_image_type((PixelBuffer::Type)scan.get_uint8());
        _pbuffer->set_num_components(scan.get_uint8());
        _pbuffer->set_component_width(scan.get_uint8());

        _pbuffer->set_loaded();
        PN_uint32 u_size = scan.get_uint32();

        // fill the _image buffer with image data
        string temp_buff = scan.extract_bytes(u_size);
        _pbuffer->_image = PTA_uchar::empty_array((int) u_size);
        for (PN_uint32 u_idx=0; u_idx < u_size; ++u_idx) {
          _pbuffer->_image[(int)u_idx] = (uchar) temp_buff[u_idx];
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
  // We also need to write out the pixel buffer's format, even though
  // that's not stored as part of the texture structure.
  bool has_pbuffer = (_pbuffer != (PixelBuffer *)NULL);
  bool has_rawdata = (bam_texture_mode == BTM_rawdata);

  // These properties are read in again by make_Texture(), above.
  ImageBuffer::write_datagram(manager, me);

  // from minor version 5, you add this byte to support rawdata mode
  me.add_bool(has_rawdata);

  // These properties are read in again by fillin(), above.
  me.add_uint8(_wrapu);
  me.add_uint8(_wrapv);
  me.add_uint8(_minfilter);
  me.add_uint8(_magfilter);
  me.add_int16(_anisotropic_degree);

  me.add_bool(has_pbuffer);
  if (has_pbuffer) {
    me.add_uint8(_pbuffer->get_format());
    me.add_uint8(_pbuffer->get_num_components());
  }

  // if it has rawdata, then stuff them here along with the header information
  if (has_rawdata) {
    me.add_int32(_pbuffer->get_xsize());
    me.add_int32(_pbuffer->get_ysize());
    me.add_int32(_pbuffer->get_xorg());
    me.add_int32(_pbuffer->get_yorg());
    me.add_uint8(_pbuffer->get_border());
    me.add_uint8(_pbuffer->get_image_type());
    me.add_uint8(_pbuffer->get_num_components());
    me.add_uint8(_pbuffer->get_component_width());

    me.add_uint32(_pbuffer->_image.size());
    me.append_data(_pbuffer->_image, _pbuffer->_image.size());
  }
}

