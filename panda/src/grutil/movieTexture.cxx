// Filename: movieTexture.cxx
// Created by: jyelon (01Aug2007)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"

#include "movieVideo.h"
#include "movieVideoCursor.h"
#include "movieTexture.h"
#include "clockObject.h"
#include "config_gobj.h"
#include "config_grutil.h"
#include "bamCacheRecord.h"
#include "bamReader.h"
#include "math.h"
#include "audioSound.h"

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
  do_load_one(video->open(), NULL, 0, LoaderOptions());
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::CData::Constructor
//       Access: public
//  Description: xxx
////////////////////////////////////////////////////////////////////
MovieTexture::CData::
CData() :
  _video_width(1),
  _video_height(1),
  _video_length(1.0),
  _playing(false),
  _clock(0.0),
  _play_rate(1.0),
  _loop_count(1),
  _loops_total(1)
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
  _video_height(copy._video_height),
  _video_length(copy._video_length),
  _playing(false),
  _clock(0.0),
  _play_rate(1.0),
  _loop_count(1),
  _loops_total(1)
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
  nassertv(false);
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
//     Function: MovieTexture::do_recalculate_image_properties
//       Access: Protected
//  Description: Resizes the texture, and adjusts the format,
//               based on the source movies.  The resulting texture
//               will be large enough to hold all the videos.
//
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
void MovieTexture::
do_recalculate_image_properties(CDWriter &cdata, const LoaderOptions &options) {
  int x_max = 1;
  int y_max = 1;
  bool alpha = false;
  double len = 0.0;
  
  for (int i=0; i<_z_size; i++) {
    MovieVideoCursor *t = cdata->_pages[i]._color;
    if (t) {
      if (t->size_x() > x_max) x_max = t->size_x();
      if (t->size_y() > y_max) y_max = t->size_y();
      if (t->length() > len) len = t->length();
      if (t->get_num_components() == 4) alpha=true;
    }
    t = cdata->_pages[i]._alpha;
    if (t) {
      if (t->size_x() > x_max) x_max = t->size_x();
      if (t->size_y() > y_max) y_max = t->size_y();
      if (t->length() > len) len = t->length();
      alpha = true;
    }
  }

  cdata->_video_width  = x_max;
  cdata->_video_height = y_max;
  cdata->_video_length = len;
  
  if (_texture_type == TT_cube_map) {
    // Texture must be square.
    if (x_max > y_max) y_max = x_max;
    if (y_max > x_max) x_max = y_max;
  }

  int x_size = x_max;
  int y_size = y_max;
  if (Texture::get_textures_power_2() != ATS_none) {
    x_max = up_to_power_2(x_max);
    y_max = up_to_power_2(y_max);
  }
  
  do_reconsider_image_properties(x_max, y_max, alpha?4:3, 
                                 T_unsigned_byte, cdata->_pages.size(),
                                 options);
  do_set_pad_size(x_max - x_size, y_max - y_size, 0);
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
            const LoaderOptions &options,
            bool header_only, BamCacheRecord *record) {

  nassertr(n == 0, false);
  nassertr(z >= 0 && z < _z_size, false);
  
  if (record != (BamCacheRecord *)NULL) {
    record->add_dependent_file(fullpath);
  }

  PT(MovieVideoCursor) color;
  PT(MovieVideoCursor) alpha;
  
  color = MovieVideo::get(fullpath)->open();
  if (color == 0) {
    return false;
  }
  if (!alpha_fullpath.empty()) {
    alpha = MovieVideo::get(alpha_fullpath)->open();
    if (alpha == 0) {
      return false;
    }
  }
  
  if (z == 0) {
    if (!has_name()) {
      set_name(fullpath.get_basename_wo_extension());
    }
    // Don't use has_filename() here, it will cause a deadlock
    if (_filename.empty()) {
      _filename = fullpath;
      _alpha_filename = alpha_fullpath;
    }
    
    _fullpath = fullpath;
    _alpha_fullpath = alpha_fullpath;
  }

  _primary_file_num_channels = primary_file_num_channels;
  _alpha_file_channel = alpha_file_channel;
  
  if (!do_load_one(color, alpha, z, options)) {
    return false;
  }
  
  set_loaded_from_image();
  set_loop(true);
  play();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::do_load_one
//       Access: Protected, Virtual
//  Description: Loads movie objects into the texture.
////////////////////////////////////////////////////////////////////
bool MovieTexture::
do_load_one(PT(MovieVideoCursor) color, PT(MovieVideoCursor) alpha, int z,
            const LoaderOptions &options) {
  
  {
    CDWriter cdata(_cycler);
    cdata->_pages.resize(z+1);
    cdata->_pages[z]._color = color;
    cdata->_pages[z]._alpha = alpha;
    do_recalculate_image_properties(cdata, options);
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
do_load_one(const PNMImage &pnmimage, const string &name, int z, int n,
            const LoaderOptions &options) {
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
  
  double offset;
  if (cdata->_synchronize != 0) {
    offset = cdata->_synchronize->get_time();
  } else {
    // Calculate the cursor position modulo the length of the movie.
    double now = ClockObject::get_global_clock()->get_frame_time();
    double clock = cdata->_clock;
    if (cdata->_playing) {
      clock += now * cdata->_play_rate;
    }
    int true_loop_count = cdata->_loops_total;
    if (true_loop_count <= 0) {
      true_loop_count = 1000000000;
    }
    if (clock >= cdata->_video_length * true_loop_count) {
      offset = cdata->_video_length;
    } else {
      offset = fmod(clock, cdata->_video_length);
    }
  }
  
  for (int i=0; i<((int)(cdata->_pages.size())); i++) {
    MovieVideoCursor *color = cdata->_pages[i]._color;
    MovieVideoCursor *alpha = cdata->_pages[i]._alpha;
    if (color && alpha) {
      if ((offset >= color->next_start())||
          ((offset < color->last_start()) && (color->can_seek()))) {
        color->fetch_into_texture_rgb(offset, (MovieTexture*)this, i);
      }
      if ((offset >= alpha->next_start())||
          ((offset < alpha->last_start()) && (alpha->can_seek()))) {
        alpha->fetch_into_texture_alpha(offset, (MovieTexture*)this, i, _alpha_file_channel);
      }
    } else if (color) {
      if ((offset >= color->next_start())||
          ((offset < color->last_start()) && (color->can_seek()))) {
        color->fetch_into_texture(offset, (MovieTexture*)this, i);
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
//     Function: MovieTexture::do_make_copy
//       Access: Protected, Virtual
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
do_make_copy() {
  PT(MovieTexture) tex = new MovieTexture(get_name());
  tex->do_assign(*this);

  return tex.p();
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::do_assign
//       Access: Protected
//  Description: Implements make_copy().
////////////////////////////////////////////////////////////////////
void MovieTexture::
do_assign(const MovieTexture &copy) {
  Texture::do_assign(copy);

  // Since 'make_copy' can be a slow operation, 
  // I release the read lock before calling make_copy.
  
  pvector<MovieVideoCursor *> color;
  pvector<MovieVideoCursor *> alpha;
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
        cdata->_pages[i]._color = color[i]->get_source()->open();
      }
      if (alpha[i]) {
        cdata->_pages[i]._alpha = alpha[i]->get_source()->open();
      }
    }
    do_recalculate_image_properties(cdata, LoaderOptions());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::reload_ram_image
//       Access: Protected, Virtual
//  Description: A MovieTexture must always keep its ram image, 
//               since there is no way to reload it from the 
//               source MovieVideo.
////////////////////////////////////////////////////////////////////
void MovieTexture::
do_reload_ram_image() {
  // A MovieTexture should never dump its RAM image.
  // Therefore, this is not needed.
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::restart
//       Access: Published
//  Description: Start playing the movie from where it was last
//               paused.  Has no effect if the movie is not paused,
//               or if the movie's cursor is already at the end.
////////////////////////////////////////////////////////////////////
void MovieTexture::
restart() {
  CDWriter cdata(_cycler);
  if (!cdata->_playing) {
    double now = ClockObject::get_global_clock()->get_frame_time();
    cdata->_clock = cdata->_clock - (now * cdata->_play_rate);
    cdata->_playing = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::stop
//       Access: Published
//  Description: Stops a currently playing or looping movie right
//               where it is.  The movie's cursor remains frozen at
//               the point where it was stopped.
////////////////////////////////////////////////////////////////////
void MovieTexture::
stop() {
  CDWriter cdata(_cycler);
  if (cdata->_playing) {
    double now = ClockObject::get_global_clock()->get_frame_time();
    cdata->_clock = cdata->_clock + (now * cdata->_play_rate);
    cdata->_playing = false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::play
//       Access: Published
//  Description: Plays the movie from the beginning.
////////////////////////////////////////////////////////////////////
void MovieTexture::
play() {
  CDWriter cdata(_cycler);
  double now = ClockObject::get_global_clock()->get_frame_time();
  cdata->_clock = 0.0 - (now * cdata->_play_rate);
  cdata->_playing = true;
  cdata->_loops_total = cdata->_loop_count;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::set_time
//       Access: Published
//  Description: Sets the movie's cursor.
////////////////////////////////////////////////////////////////////
void MovieTexture::
set_time(double t) {
  CDWriter cdata(_cycler);
  t = min(cdata->_video_length, max(0.0, t));
  if (cdata->_playing) {
    double now = ClockObject::get_global_clock()->get_frame_time();
    cdata->_clock = t - (now * cdata->_play_rate);
  } else {
    cdata->_clock = t;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::get_time
//       Access: Published
//  Description: Returns the current value of the movie's cursor.
//               If the movie's loop count is greater than one, then
//               its length is effectively multiplied for the
//               purposes of this function.  In other words, 
//               the return value will be in the range 0.0 
//               to (length * loopcount).
////////////////////////////////////////////////////////////////////
double MovieTexture::
get_time() const {
  CDReader cdata(_cycler);
  double clock = cdata->_clock;
  if (cdata->_playing) {
    double now = ClockObject::get_global_clock()->get_frame_time();
    clock += (now * cdata->_play_rate);
  }
  if (clock >= cdata->_video_length * cdata->_loops_total) {
    return cdata->_video_length;
  } else {
    return fmod(clock, cdata->_video_length);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::set_loop
//       Access: Published
//  Description: If true, sets the movie's loop count to 1 billion.
//               If false, sets the movie's loop count to one.
////////////////////////////////////////////////////////////////////
void MovieTexture::
set_loop(bool loop) {
  set_loop_count(loop ? 0:1);
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::get_loop
//       Access: Published
//  Description: Returns true if the movie's loop count is not equal
//               to one.
////////////////////////////////////////////////////////////////////
bool MovieTexture::
get_loop() const {
  CDReader cdata(_cycler);
  return (cdata->_loop_count == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::set_loop_count
//       Access: Published
//  Description: Sets the movie's loop count to the desired value.
////////////////////////////////////////////////////////////////////
void MovieTexture::
set_loop_count(int n) {
  CDWriter cdata(_cycler);
  cdata->_loop_count = n;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::get_loop_count
//       Access: Published
//  Description: Returns the movie's loop count.
////////////////////////////////////////////////////////////////////
int MovieTexture::
get_loop_count() const {
  CDReader cdata(_cycler);
  return cdata->_loop_count;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::set_play_rate
//       Access: Published
//  Description: Sets the movie's play-rate.  This is the speed at
//               which the movie's cursor advances.  The default is
//               to advance 1.0 movie-seconds per real-time second.
////////////////////////////////////////////////////////////////////
void MovieTexture::
set_play_rate(double rate) {
  CDWriter cdata(_cycler);
  if (cdata->_playing) {
    double now = ClockObject::get_global_clock()->get_frame_time();
    cdata->_clock += (now * cdata->_play_rate);
    cdata->_play_rate = rate;
    cdata->_clock -= (now * cdata->_play_rate);
  } else {
    cdata->_play_rate = rate;
  }    
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::get_play_rate
//       Access: Published
//  Description: Gets the movie's play-rate.
////////////////////////////////////////////////////////////////////
double MovieTexture::
get_play_rate() const {
  CDReader cdata(_cycler);
  return cdata->_play_rate;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::is_playing
//       Access: Published
//  Description: Returns true if the movie's cursor is advancing.
////////////////////////////////////////////////////////////////////
bool MovieTexture::
is_playing() const {
  CDReader cdata(_cycler);
  return cdata->_playing;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::synchronize_to
//       Access: Published
//  Description: Synchronize this texture to a sound.  Typically,
//               you would load the texture and the sound from the
//               same AVI file.
////////////////////////////////////////////////////////////////////
void MovieTexture::
synchronize_to(AudioSound *s) {
  CDWriter cdata(_cycler);
  cdata->_synchronize = s;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::unsynchronize
//       Access: Published
//  Description: Stop synchronizing with a sound.
////////////////////////////////////////////////////////////////////
void MovieTexture::
unsynchronize() {
  CDWriter cdata(_cycler);
  cdata->_synchronize = 0;
}
