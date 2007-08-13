// Filename: movieTexture.cxx
// Created by: jyelon (01Aug2007)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http:// etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"

#include "movieVideo.h"
#include "movieTexture.h"
#include "clockObject.h"
#include "config_gobj.h"
#include "config_grutil.h"
#include "bamCacheRecord.h"

TypeHandle MovieTexture::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::Constructor
//       Access: Published
//  Description: Creates a blank movie texture.  Movies must be 
//               added using do_read_one or do_load_one.
////////////////////////////////////////////////////////////////////
MovieTexture::
MovieTexture(const string &name) : 
  Texture(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::Constructor
//       Access: Published
//  Description: Creates a texture playing the specified movie.
////////////////////////////////////////////////////////////////////
MovieTexture::
MovieTexture(PT(MovieVideo) video) : 
  Texture(video->get_name())
{
  do_load_one(video, NULL, 0);
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::CData::Constructor
//       Access: public
//  Description: xxx
////////////////////////////////////////////////////////////////////
MovieTexture::CData::
CData()
{
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::CData::Copy Constructor
//       Access: public
//  Description: xxx
////////////////////////////////////////////////////////////////////
MovieTexture::CData::
CData(const CData &copy) :
  _pages(copy._pages),
  _video_width(copy._video_width),
  _video_height(copy._video_height)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::CData::make_copy
//       Access: public
//  Description: xxx
////////////////////////////////////////////////////////////////////
CycleData *MovieTexture::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::Copy Constructor
//       Access: Protected
//  Description: Use MovieTexture::make_copy() to make a duplicate copy of
//               an existing MovieTexture.
////////////////////////////////////////////////////////////////////
MovieTexture::
MovieTexture(const MovieTexture &copy) : 
  Texture(copy)
{
  // Since 'make_copy' can be a slow operation, 
  // I release the read lock before calling make_copy.
  
  pvector<MovieVideo *> color;
  pvector<MovieVideo *> alpha;
  {
    CDReader copy_cdata(copy._cycler);
    color.resize(copy_cdata->_pages.size());
    alpha.resize(copy_cdata->_pages.size());
    for (int i=0; i<(int)(color.size()); i++) {
      color[i] = copy_cdata->_pages[i]._color;
      alpha[i] = copy_cdata->_pages[i]._alpha;
    }
  }
  
  {
    CDWriter cdata(_cycler);
    cdata->_pages.resize(color.size());
    for (int i=0; i<(int)(color.size()); i++) {
      if (color[i]) {
        cdata->_pages[i]._color = color[i]->make_copy();
      }
      if (alpha[i]) {
        cdata->_pages[i]._alpha = color[i]->make_copy();
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::Destructor
//       Access: Published, Virtual
//  Description: xxx
////////////////////////////////////////////////////////////////////
MovieTexture::
~MovieTexture() {
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::make_copy
//       Access: Published, Virtual
//  Description: Returns a new copy of the same Texture.  This copy,
//               if applied to geometry, will be copied into texture
//               as a separate texture from the original, so it will
//               be duplicated in texture memory (and may be
//               independently modified if desired).
//               
//               If the Texture is an MovieTexture, the resulting
//               duplicate may be animated independently of the
//               original.
////////////////////////////////////////////////////////////////////
PT(Texture) MovieTexture::
make_copy() {
  return new MovieTexture(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::make_texture
//       Access: Public, Static
//  Description: A factory function to make a new MovieTexture, used
//               to pass to the TexturePool.
////////////////////////////////////////////////////////////////////
PT(Texture) MovieTexture::
make_texture() {
  return new MovieTexture("");
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::VideoPage::Constructor
//       Access: Private
//  Description: Creates a completely blank video page.
////////////////////////////////////////////////////////////////////
MovieTexture::VideoPage::
VideoPage() :
  _color(0),
  _alpha(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::recalculate_image_properties
//       Access: Protected
//  Description: Resizes the texture, and adjusts the format,
//               based on the source movies.  The resulting texture
//               will be large enough to hold all the videos.
////////////////////////////////////////////////////////////////////
void MovieTexture::
recalculate_image_properties(CDWriter &cdata) {
  int x_max = 0;
  int y_max = 0;
  bool alpha = false;
  
  for (int i=0; i<_z_size; i++) {
    MovieVideo *t = cdata->_pages[i]._color;
    if (t) {
      if (t->size_x() > x_max) x_max = t->size_x();
      if (t->size_y() > y_max) y_max = t->size_y();
      if (t->get_num_components() == 4) alpha=true;
    }
    t = cdata->_pages[i]._alpha;
    if (t) {
      if (t->size_x() > x_max) x_max = t->size_x();
      if (t->size_y() > y_max) y_max = t->size_y();
      alpha = true;
    }
  }

  cdata->_video_width  = x_max;
  cdata->_video_height = y_max;
  
  if (get_texture_type() == TT_cube_map) {
    // Texture must be square.
    if (x_max > y_max) y_max = x_max;
    if (y_max > x_max) x_max = y_max;
  }
  
  if (textures_power_2 != ATS_none) {
    x_max = up_to_power_2(x_max);
    y_max = up_to_power_2(y_max);
  }
  
  reconsider_image_properties(x_max, y_max, alpha?4:3, 
                              T_unsigned_byte, cdata->_pages.size());
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::do_read_one
//       Access: Protected, Virtual
//  Description: Combines a color and alpha video image from the two
//               indicated filenames.  Both must be the same kind of
//               video with similar properties.
////////////////////////////////////////////////////////////////////
bool MovieTexture::
do_read_one(const Filename &fullpath, const Filename &alpha_fullpath,
            int z, int n, int primary_file_num_channels, int alpha_file_channel,
            bool header_only, BamCacheRecord *record) {

  nassertr(n == 0, false);
  nassertr(z >= 0 && z < get_z_size(), false);
  
  if (record != (BamCacheRecord *)NULL) {
    record->add_dependent_file(fullpath);
  }

  PT(MovieVideo) color;
  PT(MovieVideo) alpha;
  
  color = MovieVideo::load(fullpath);
  if (color == 0) {
    return false;
  }
  if (!alpha_fullpath.empty()) {
    alpha = MovieVideo::load(alpha_fullpath);
    if (alpha == 0) {
      return false;
    }
  }
  
  if (z == 0) {
    if (!has_name()) {
      set_name(fullpath.get_basename_wo_extension());
    }
    if (!has_filename()) {
      set_filename(fullpath);
      set_alpha_filename(alpha_fullpath);
    }
    
    set_fullpath(fullpath);
    set_alpha_fullpath(alpha_fullpath);
  }

  _primary_file_num_channels = primary_file_num_channels;
  _alpha_file_channel = alpha_file_channel;
  
  return do_load_one(color, alpha, z);

  set_loaded_from_image();
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::do_load_one
//       Access: Protected, Virtual
//  Description: Loads movie objects into the texture.
////////////////////////////////////////////////////////////////////
bool MovieTexture::
do_load_one(PT(MovieVideo) color, PT(MovieVideo) alpha, int z) {
  
  {
    CDWriter cdata(_cycler);
    cdata->_pages.resize(z+1);
    cdata->_pages[z]._color = color;
    cdata->_pages[z]._alpha = alpha;
    cdata->_pages[z]._base_clock = ClockObject::get_global_clock()->get_frame_time();
    recalculate_image_properties(cdata);
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::do_load_one
//       Access: Protected, Virtual
//  Description: Loading a static image into a MovieTexture is
//               an error.
////////////////////////////////////////////////////////////////////
bool MovieTexture::
do_load_one(const PNMImage &pnmimage, const string &name, int z, int n) {
  grutil_cat.error() << "You cannot load a static image into a MovieTexture\n";
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::register_with_read_factory
//       Access: Public, Static
//  Description: Factory method to generate a Texture object
////////////////////////////////////////////////////////////////////
void MovieTexture::
register_with_read_factory() {
  // Since Texture is such a funny object that is reloaded from the
  // TexturePool each time, instead of actually being read fully from
  // the bam file, and since the VideoTexture and MovieTexture
  // classes don't really add any useful data to the bam record, we
  // don't need to define make_from_bam(), fillin(), or
  // write_datagram() in this class--we just inherit the same
  // functions from Texture.

  // We do, however, have to register this class with the BamReader,
  // to avoid warnings about creating the wrong kind of object from
  // the bam file.
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::has_cull_callback
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this node during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool MovieTexture::
has_cull_callback() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::cull_callback
//       Access: Public, Virtual
//  Description: This function will be called during the cull 
//               traversal to update the MovieTexture.  This update
//               consists of fetching the next video frame from the
//               underlying MovieVideo sources.  The MovieVideo
//               object belongs to the cull thread.
////////////////////////////////////////////////////////////////////
bool MovieTexture::
cull_callback(CullTraverser *, const CullTraverserData &) const {
  CDReader cdata(_cycler);
  double now = ClockObject::get_global_clock()->get_frame_time();
  for (int i=0; i<((int)(cdata->_pages.size())); i++) {
    double delta = now - cdata->_pages[i]._base_clock;
    MovieVideo *color = cdata->_pages[i]._color;
    MovieVideo *alpha = cdata->_pages[i]._alpha;
    if (color) {
      double offset = delta;
      if ((offset < color->last_start()) || (offset >= color->next_start())) {
        if (alpha) {
          color->fetch_into_texture_rgb(offset, (MovieTexture*)this, i);
        } else {
          color->fetch_into_texture(offset, (MovieTexture*)this, i);
        }
      }
    }
    if (alpha) {
      double offset = delta;
      if ((offset < alpha->last_start()) || (offset >= alpha->next_start())) {
	alpha->fetch_into_texture_alpha(offset, (MovieTexture*)this, i, _alpha_file_channel);
      }
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::get_keep_ram_image
//       Access: Published, Virtual
//  Description: A MovieTexture must always keep its ram image, 
//               since there is no way to reload it from the 
//               source MovieVideo.
////////////////////////////////////////////////////////////////////
bool MovieTexture::
get_keep_ram_image() const {
  // A MovieTexture should never dump its RAM image.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::reload_ram_image
//       Access: Protected, Virtual
//  Description: A MovieTexture must always keep its ram image, 
//               since there is no way to reload it from the 
//               source MovieVideo.
////////////////////////////////////////////////////////////////////
void MovieTexture::
reload_ram_image() {
  // A MovieTexture should never dump its RAM image.
  // Therefore, this is not needed.
}

